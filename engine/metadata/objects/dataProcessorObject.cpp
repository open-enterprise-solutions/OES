////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : dataProcessor - object
////////////////////////////////////////////////////////////////////////////

#include "dataProcessor.h"
#include "metadata/metadata.h"

#include "appData.h"
#include "reference/reference.h"
#include "forms/elements/frame.h"
#include "database/databaseLayer.h"
#include "compiler/systemObjects.h"

#include "metadata/objects/tablePart/tablePart.h"

#include "utils/fs/fs.h"
#include "utils/stringutils.h"

//*********************************************************************************************
//*                                  ObjectCatalogValue                                       *
//*********************************************************************************************

CObjectDataProcessorValue::CObjectDataProcessorValue(const CObjectDataProcessorValue &source) : IDataObjectValue(source),
m_metaObject(source.m_metaObject), m_objGuid(Guid::newGuid())
{
	if (m_metaObject->m_objMode == METAOBJECT_NORMAL) {
		InitializeModule();
	}
}

CObjectDataProcessorValue::CObjectDataProcessorValue(CMetaObjectDataProcessorValue *metaObj) : IDataObjectValue(),
m_metaObject(metaObj), m_objGuid(Guid::newGuid())
{
	if (m_metaObject->m_objMode == METAOBJECT_NORMAL) {
		InitializeModule();
	}
}

CObjectDataProcessorValue::~CObjectDataProcessorValue()
{
	if (appData->IsEnterpriseMode()
		&& m_metaObject->m_objMode == METAOBJECT_EXTERNAL) {
		delete m_metaObject->GetMetadata();
	}
}

void CObjectDataProcessorValue::PrepareEmptyObject()
{
	//attrbutes can refValue 
	for (auto attribute : m_metaObject->GetObjectAttributes())
	{
		switch (attribute->GetTypeObject())
		{
		case eValueTypes::TYPE_BOOLEAN: m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_BOOLEAN; break;
		case eValueTypes::TYPE_NUMBER: m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_NUMBER; break;
		case eValueTypes::TYPE_DATE: m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_DATE; break;
		case eValueTypes::TYPE_STRING: m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_STRING; break;
		default: m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = new CValueReference(attribute->GetTypeObject()); break;
		}
		m_aObjectTypes[StringUtils::MakeUpper(attribute->GetName())] = attribute->GetTypeObject();
	}

	// table is collection values 
	for (auto table : m_metaObject->GetObjectTables())
	{
		m_aObjectValues[StringUtils::MakeUpper(table->GetName())] = new CValueTabularSection(m_metaObject, table, this);
		m_aObjectTypes[StringUtils::MakeUpper(table->GetName())] = wxNOT_FOUND;
	}
}

bool CObjectDataProcessorValue::InitializeModule()
{
	IMetadata *m_metaData = m_metaObject->GetMetadata();
	wxASSERT(m_metaData);
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!m_compileModule)
	{
		m_compileModule = new CCompileModule(m_metaObject->GetModuleObject());
		m_compileModule->SetParent(moduleManager->GetCompileModule());
		m_compileModule->AddContextVariable(thisObject, this);
	}

	try
	{
		m_compileModule->Compile();
	}
	catch (const CTranslateError *err)
	{
		if (appData->IsEnterpriseMode()) CSystemObjects::Raise(err->what());
		else CSystemObjects::Message(err->what());

		return false;
	};

	if (appData->IsEnterpriseMode())
	{
		m_procUnit = new CProcUnit();
		m_procUnit->SetParent(moduleManager->GetProcUnit());
	}

	PrepareEmptyObject();

	if (appData->IsEnterpriseMode())
		m_procUnit->Execute(m_compileModule->m_cByteCode, true);

	//is Ok
	return true;
}

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

void CObjectDataProcessorValue::SetValueByMetaID(meta_identifier_t metaid, CValue & value)
{
	IMetadata *metaData = m_metaObject->GetMetadata();
	wxASSERT(metaData);

	IMetaObject *metaObject = metaData->GetMetaObject(metaid);
	if (metaObject) {
		auto foundedIt = std::find_if(m_aObjectValues.begin(), m_aObjectValues.end(), [metaObject](std::pair<wxString, CValue>pair) { return CompareString(pair.first, metaObject->GetName()); });
		if (foundedIt != m_aObjectValues.end()) {
			foundedIt->second = value;
		}
	}
}

CValue CObjectDataProcessorValue::GetValueByMetaID(meta_identifier_t metaid)
{
	IMetadata *metaData = m_metaObject->GetMetadata();
	wxASSERT(metaData);

	IMetaObject *metaObject = metaData->GetMetaObject(metaid);
	if (metaObject) {
		auto foundedIt = std::find_if(m_aObjectValues.begin(), m_aObjectValues.end(), [metaObject](std::pair<wxString, CValue>pair) { return CompareString(pair.first, metaObject->GetName()); });
		if (foundedIt != m_aObjectValues.end()) {
			return foundedIt->second;
		}
	}

	return CValue();
}

void CObjectDataProcessorValue::ShowFrameValue(const wxString &formName, IOwnerInfo *owner)
{
	if (m_valueFrameObject && m_valueFrameObject->IsShown()) {
		m_valueFrameObject->ActivateForm();
		return;
	}

	//if form is not initialised then generate  
	if (!m_valueFrameObject) {
		m_valueFrameObject = GetFrameValue(formName, owner);
	}

	m_valueFrameObject->Modify(false);
	m_valueFrameObject->ShowForm();
}

CValueFrame *CObjectDataProcessorValue::GetFrameValue(const wxString &formName, IOwnerInfo *owner)
{
	if (m_valueFrameObject)
		return m_valueFrameObject;

	IMetaFormObject* defList = NULL;

	if (!formName.IsEmpty()) {
		for (auto metaForm : m_metaObject->GetObjectForms()) {
			if (CompareString(formName, metaForm->GetName())) {
				defList = metaForm; break;
			}
		}
		wxASSERT(defList);
	}
	else {
		defList = m_metaObject->GetDefaultFormByID(CMetaObjectDataProcessorValue::eFormDataProcessor);
	}

	if (defList) {
		m_valueFrameObject = defList->GenerateFormAndRun(m_metaObject, this);
	}
	else {
		m_valueFrameObject = new CValueFrame();
	}

	m_valueFrameObject->IncrRef();

	return m_valueFrameObject;
}