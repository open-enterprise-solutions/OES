////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : catalog object
////////////////////////////////////////////////////////////////////////////

#include "catalog.h"
#include "metadata/metadata.h"

#include "appData.h"
#include "reference/reference.h"
#include "forms/elements/frame.h"
#include "database/databaseLayer.h"
#include "compiler/systemObjects.h"

#include "utils/fs/fs.h"
#include "utils/stringutils.h"

//*********************************************************************************************
//*                                  ObjectCatalogValue                                       *
//*********************************************************************************************

CObjectCatalogValue::CObjectCatalogValue(const CObjectCatalogValue &source) :
	IDataObjectRefValue(source), m_catOwner(NULL), m_catParent(NULL)
{
	InitializeModule();
}

CObjectCatalogValue::CObjectCatalogValue(CMetaObjectCatalogValue *metaObj) :
	IDataObjectRefValue(metaObj), m_catOwner(NULL), m_catParent(NULL)
{
	InitializeModule();
}


CObjectCatalogValue::CObjectCatalogValue(CMetaObjectCatalogValue *metaObj, const Guid &guid) :
	IDataObjectRefValue(metaObj, guid), m_catOwner(NULL), m_catParent(NULL)
{
	InitializeModule();
}

bool CObjectCatalogValue::InitializeModule()
{
	IMetadata *metaData = m_metaObject->GetMetadata();
	wxASSERT(metaData);
	IModuleManager *moduleManager = metaData->GetModuleManager();
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

	if (appData->IsDesignerMode() || (appData->IsEnterpriseMode() && !ReadInDB())) {
		PrepareEmptyObject();
	}

	if (appData->IsEnterpriseMode())
		m_procUnit->Execute(m_compileModule->m_cByteCode, true);

	//is Ok
	return true;
}

inline bool CompareString(const wxString &lhs, const wxString &rhs)
{
	return lhs.CompareTo(rhs, wxString::ignoreCase) == 0;
}

void CObjectCatalogValue::SetValueByMetaID(meta_identifier_t metaid, CValue &value)
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

CValue CObjectCatalogValue::GetValueByMetaID(meta_identifier_t metaid)
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

void CObjectCatalogValue::ShowFrameValue(const wxString &formName, IOwnerInfo *owner)
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

CValueFrame *CObjectCatalogValue::GetFrameValue(const wxString &formName, IOwnerInfo *ownerControl)
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
		defList = m_metaObject->GetDefaultFormByID(CMetaObjectCatalogValue::eFormObject);
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

#include "compiler/systemObjects.h"

//***********************************************************************************************
//*                                   Catalog events                                            *
//***********************************************************************************************

void CObjectCatalogValue::FillObject(CValue vFillObject)
{
	if (appData->IsEnterpriseMode())
	{
		m_procUnit->CallFunction("Filling", vFillObject);
	}
}

CValue CObjectCatalogValue::CopyObject()
{
	if (appData->IsEnterpriseMode())
	{
		IDataObjectValue *vCopyValue = CopyObjectValue();
		vCopyValue->GetProcUnit()->CallFunction("OnCopy");
		return vCopyValue;
	}

	return CValue();
}

bool CObjectCatalogValue::WriteObject()
{
	if (appData->IsEnterpriseMode())
	{
		if (!CTranslateError::IsSimpleMode())
		{
			objectDatabase->BeginTransaction();

			{
				CValue cancel = false;
				m_procUnit->CallFunction("BeforeWrite", cancel);

				if (cancel.GetBoolean()) { objectDatabase->RollBack(); CSystemObjects::Raise("failed to write object in db!"); return false; }
			}

			if (!SaveInDB()) { objectDatabase->RollBack(); CSystemObjects::Raise("failed to write object in db!"); return false; }

			{
				CValue cancel = false;
				m_procUnit->CallFunction("OnWrite", cancel);
				if (cancel.GetBoolean()) { objectDatabase->RollBack(); CSystemObjects::Raise("failed to write object in db!"); return false; }
			}

			objectDatabase->Commit();

			if (m_valueFrameObject) {
				m_valueFrameObject->Modify(false);
			}
		}
	}

	return true;
}

bool CObjectCatalogValue::DeleteObject()
{
	if (appData->IsEnterpriseMode())
	{
		if (!CTranslateError::IsSimpleMode())
		{
			objectDatabase->BeginTransaction();

			{
				CValue cancel = false;
				m_procUnit->CallFunction("BeforeDelete", cancel);
				if (cancel.GetBoolean()) { objectDatabase->RollBack(); CSystemObjects::Raise("failed to write object in db!"); return false; }
			}

			if (!DeleteInDB()) { objectDatabase->RollBack(); CSystemObjects::Raise("failed to delete object in db!"); return false; }

			{
				CValue cancel = false;
				m_procUnit->CallFunction("OnDelete", cancel);
				if (cancel.GetBoolean()) { objectDatabase->RollBack(); CSystemObjects::Raise("failed to write object in db!"); return false; }
			}

			objectDatabase->Commit();
		}
	}

	return true;
}