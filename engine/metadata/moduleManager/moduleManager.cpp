////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : module manager for common modules and compile value (in designer mode)
////////////////////////////////////////////////////////////////////////////

#include "moduleManager.h"
#include "metadata/objects/baseManager.h"
#include "compiler/enumFactory.h"
#include "compiler/methods.h"
#include "compiler/systemObjects.h"
#include "utils/stringutils.h"
#include "appData.h"

#define objectManager wxT("manager")
#define objectSysManager wxT("sysManager")
#define objectEnumManager wxT("enumManager")

#define objectMetadataManager wxT("metadata")

//*********************************************************************************************************
//*                                   Singleton class "moduleManager"                                     *
//*********************************************************************************************************

#include "metadata/metadata.h"

IModuleManager::IModuleManager(IMetadata *metaData, CMetaModuleObject *obj) :
	CValue(eValueTypes::TYPE_MODULE), IModuleInfo(new CCompileModule(obj, false)),
	m_methods(new CMethods()),
	m_initialized(false)
{
	//increment reference
	m_objectManager = new CManagerBase(metaData);
	m_objectManager->IncrRef();

	m_objectSysManager = new CSystemObjects();
	m_objectSysManager->IncrRef();

	m_metaManager = new CMetadataValue(metaData);
	m_metaManager->IncrRef();

	//add global variables 
	m_aValueGlVariables.insert_or_assign(objectMetadataManager, m_metaManager);
}

void IModuleManager::Clear()
{
	for (auto compileModule : m_aCompileModules)
	{
		CValue *dataRef = compileModule.second;
		wxASSERT(dataRef);
		dataRef->DecrRef();
	}

	for (auto moduleValue : m_aCommonModules)
	{
		moduleValue->DecrRef();
	}

	//clear compile table 
	m_aCompileModules.clear();
	m_aCommonModules.clear();
}

IModuleManager::~IModuleManager()
{
	Clear();

	wxDELETE(m_methods);

	wxDELETE(m_objectManager);
	wxDELETE(m_objectSysManager);
	wxDELETE(m_metaManager);
}

//*************************************************************************************************************************
//************************************************  support compile module ************************************************
//*************************************************************************************************************************

bool IModuleManager::AddCompileModule(IMetaObject *mobj, CValue *object)
{
	if (!appData->IsDesignerMode() || !object)
		return true;

	std::map<IMetaObject *, CValue *>::iterator founded = m_aCompileModules.find(mobj);

	if (founded == m_aCompileModules.end()) {
		m_aCompileModules.insert_or_assign(mobj, object);
		object->IncrRef();
		return true;
	}

	return false;
}

bool IModuleManager::RemoveCompileModule(IMetaObject *obj)
{
	if (!appData->IsDesignerMode())
		return true;

	std::map<IMetaObject *, CValue *>::iterator founded = m_aCompileModules.find(obj);

	if (founded != m_aCompileModules.end()) {
		CValue *dataRef = founded->second;
		m_aCompileModules.erase(founded);
		dataRef->DecrRef();
		return true;
	}

	return false;
}

bool IModuleManager::AddCommonModule(CMetaCommonModuleObject *commonModule, bool managerModule)
{
	CModuleValue *moduleValue = new CModuleValue(this, commonModule);
	moduleValue->IncrRef();

	if (!IModuleManager::AddCompileModule(commonModule, moduleValue))
		return false;

	m_aCommonModules.push_back(moduleValue);

	if (!managerModule)
		m_aValueGlVariables.insert_or_assign(commonModule->GetName(), moduleValue);

	return true;
}

bool IModuleManager::AddCommonModuleAndRun(CMetaCommonModuleObject *commonModule)
{
	CModuleValue *moduleValue = new CModuleValue(this, commonModule);
	moduleValue->IncrRef();

	if (!IModuleManager::AddCompileModule(commonModule, moduleValue))
		return false;

	m_aCommonModules.push_back(moduleValue);
	m_aValueGlVariables.insert_or_assign(commonModule->GetName(), moduleValue);

	return moduleValue->CreateCommonModule();
}

CValue *IModuleManager::FindCommonModule(CMetaCommonModuleObject *commonModule)
{
	auto m_moduleObjectIt = std::find_if(m_aCommonModules.begin(), m_aCommonModules.end(), [commonModule](CModuleValue *valueModule) { return commonModule == valueModule->GetModuleObject(); });

	if (m_moduleObjectIt != m_aCommonModules.end())
		return *m_moduleObjectIt;

	return NULL;
}

bool IModuleManager::RenameCommonModule(CMetaCommonModuleObject *commonModule, const wxString &newName)
{
	CValue *moduleValue = FindCommonModule(commonModule);
	wxASSERT(moduleValue);

	m_aValueGlVariables.insert_or_assign(newName, moduleValue);
	m_aValueGlVariables.erase(commonModule->GetName());

	return true;
}

bool IModuleManager::RemoveCommonModule(CMetaCommonModuleObject *commonModule)
{
	CValue *moduleValue = FindCommonModule(commonModule);
	wxASSERT(moduleValue);

	if (!IModuleManager::RemoveCompileModule(commonModule))
		return false;

	auto m_moduleObjectIt = std::find(m_aCommonModules.begin(), m_aCommonModules.end(), moduleValue);

	if (m_moduleObjectIt == m_aCommonModules.end())
		return false;

	m_aValueGlVariables.erase(commonModule->GetName());
	m_aCommonModules.erase(m_moduleObjectIt);

	moduleValue->DecrRef();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////
//  CModuleManager
//////////////////////////////////////////////////////////////////////////////////

CModuleManager::CModuleManager(IMetadata *metaData, CMetaObject *metaObject)
	: IModuleManager(metaData, metaObject->GetModuleObject())
{
}

//main module - initialize
bool CModuleManager::CreateMainModule()
{
	if (m_initialized)
		return true;

	//Добавление глобальных констант
	for (auto variable : m_aValueGlVariables)
	{
		m_compileModule->AddVariable(variable.first, variable.second);
	}

	//create singleton "manager"
	m_compileModule->AddContextVariable(objectManager, m_objectManager);
	//create singleton "sys.manager"
	m_compileModule->AddContextVariable(objectSysManager, m_objectSysManager);
	//create singleton "enum.manager"
	m_compileModule->AddContextVariable(objectEnumManager, enumFactory);

	//initialize procUnit
	wxDELETE(m_procUnit);

	try
	{
		m_compileModule->Compile();
	}
	catch (const CTranslateError *err)
	{
		if (appData->IsEnterpriseMode()) {
			wxMessageBox(err->what(), m_compileModule->GetModuleName());
			return false;
		}
		else {
			CSystemObjects::Message(err->what());
		}
	};

	if (appData->IsEnterpriseMode())
	{
		m_procUnit = new CProcUnit();
		m_procUnit->Execute(m_compileModule->m_cByteCode, true);
	}

	//Setup common modules
	for (auto moduleValue : m_aCommonModules)
	{
		if (!moduleValue->CreateCommonModule())
		{
			if (appData->IsEnterpriseMode())
				return false;
		}
	}

	m_initialized = true;
	return true;
}

bool CModuleManager::DestroyMainModule()
{
	if (!m_initialized)
		return true;

	//Добавление глобальных констант
	for (auto variable : m_aValueGlVariables)
	{
		m_compileModule->RemoveVariable(variable.first);
	}

	//create singleton "manager"
	m_compileModule->RemoveVariable(objectManager);
	//create singleton "sys.manager"
	m_compileModule->RemoveVariable(objectSysManager);
	//create singleton "enum.manager"
	m_compileModule->RemoveVariable(objectEnumManager);

	m_compileModule->Reset();
	wxDELETE(m_procUnit);

	//Setup common modules
	for (auto moduleValue : m_aCommonModules)
	{
		if (!moduleValue->DestroyCommonModule())
		{
			if (appData->IsEnterpriseMode())
				return false;
		}
	}

	m_initialized = false;
	return true;
}

//main module - initialize
bool CModuleManager::StartMainModule()
{
	if (!m_initialized)
		return false;

	if (BeforeStart()) {
		OnStart(); return true;
	}

	return false;
}

//main module - destroy
bool CModuleManager::ExitMainModule(bool force)
{
	if (force)
		return true;

	if (!m_initialized)
		return false;

	if (BeforeExit()) {
		OnExit(); m_initialized = false; return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////
//  CExternalModuleManager
//////////////////////////////////////////////////////////////////////////////////

CCompileModule *CExternalModuleManager::GetCompileModule() const
{
	IModuleManager *moduleManager = metadata->GetModuleManager();
	wxASSERT(moduleManager);
	return moduleManager->GetCompileModule();
}

CProcUnit *CExternalModuleManager::GetProcUnit() const
{
	IModuleManager *moduleManager = metadata->GetModuleManager();
	wxASSERT(moduleManager);
	return moduleManager->GetProcUnit();
}

std::map<wxString, CValue*>& CExternalModuleManager::GetContextVariables()
{
	IModuleManager *moduleManager = metadata->GetModuleManager();
	wxASSERT(moduleManager);
	return moduleManager->GetContextVariables();
}

CExternalModuleManager::CExternalModuleManager(IMetadata *metaData, CMetaObjectDataProcessorValue *metaObject)
	: IModuleManager(CMetadata::Get(), metaObject->GetModuleObject())
{
	m_objectData = new CObjectDataProcessorValue(metaObject);
}

CExternalModuleManager::~CExternalModuleManager()
{
}

bool CExternalModuleManager::CreateMainModule()
{
	if (m_initialized)
		return true;

	IModuleManager *moduleManager = metadata->GetModuleManager();
	wxASSERT(moduleManager);

	m_compileModule->SetParent(moduleManager->GetCompileModule());

	//Добавление глобальных констант
	for (auto variable : m_aValueGlVariables)
	{
		m_compileModule->AddVariable(variable.first, variable.second);
	}

	//initialize procUnit
	wxDELETE(m_procUnit);

	try
	{
		m_compileModule->Compile();
	}
	catch (const CTranslateError *err)
	{
		if (appData->IsEnterpriseMode())
		{
			wxMessageBox(err->what(), m_compileModule->GetModuleName());
			return false;
		}
		else
		{
			CSystemObjects::Message(err->what());
		}
	};

	//set complile module 
	m_objectData->m_compileModule = m_compileModule;

	if (appData->IsEnterpriseMode())
	{
		m_procUnit = new CProcUnit();
		m_procUnit->SetParent(moduleManager->GetProcUnit());
		m_procUnit->Execute(m_compileModule->m_cByteCode, true);
	}

	//set proc unit 
	m_objectData->m_procUnit = m_procUnit;

	//Setup common modules
	for (auto moduleValue : m_aCommonModules)
	{
		if (!moduleValue->CreateCommonModule())
		{
			if (appData->IsEnterpriseMode())
				return false;
		}
	}

	//set initialized true
	m_initialized = true;
	return true;
}

bool CExternalModuleManager::DestroyMainModule()
{
	if (!m_initialized)
		return true;

	//Добавление глобальных констант
	for (auto variable : m_aValueGlVariables)
	{
		m_compileModule->RemoveVariable(variable.first);
	}

	m_compileModule->Reset();

	//set complile module 
	m_objectData->m_compileModule = NULL;

	wxDELETE(m_procUnit);

	//set proc unit 
	m_objectData->m_procUnit = NULL;

	//Setup common modules
	for (auto moduleValue : m_aCommonModules)
	{
		if (!moduleValue->DestroyCommonModule())
		{
			if (appData->IsEnterpriseMode())
				return false;
		}
	}

	m_initialized = false;
	return true;
}

#include "forms/elements/frame.h"

//main module - initialize
bool CExternalModuleManager::StartMainModule()
{
	if (!m_initialized)
		return false;

	//incrRef - for control delete 
	m_objectData->IncrRef();

	IMetaObjectValue *commonObject = m_objectData->GetMetaObject();
	wxASSERT(commonObject);
	IMetaFormObject *defFormObject = commonObject->GetDefaultFormByID(CMetaObjectDataProcessorValue::eFormDataProcessor);

	if (defFormObject) {

		CValueFrame *valueFrame = NULL;

		if (!IModuleManager::FindCompileModule(defFormObject, valueFrame)) {

			valueFrame = defFormObject->GenerateForm(commonObject, m_objectData);

			try
			{
				valueFrame->InitializeModule();
				valueFrame->ShowForm();
			}
			catch (const CTranslateError *err) {

				wxMessageBox(err->what());
				wxDELETE(valueFrame);

				if (appData->IsEnterpriseMode()) {
					//decrRef - for control delete 
					m_objectData->DecrRef();
					return false;
				}
			}
			catch (...) {

				wxDELETE(valueFrame);

				if (appData->IsEnterpriseMode()) {
					//decrRef - for control delete 
					m_objectData->DecrRef();
					return false;
				}
			}
		}
	}

	//decrRef - for control delete 
	m_objectData->DecrRef();

	return true;
}

//main module - destroy
bool CExternalModuleManager::ExitMainModule(bool force)
{
	if (force)
		return true;

	if (!m_initialized)
		return false;

	return true;
}

//**********************************************************************
//*                       Runtime register                             *
//**********************************************************************

SO_VALUE_REGISTER(IModuleManager::CModuleValue, "module", CModuleValue, TEXT2CLSID("SO_MODL"));
SO_VALUE_REGISTER(IModuleManager::CMetadataValue, "metadata", CMetadataValue, TEXT2CLSID("SO_METD"));