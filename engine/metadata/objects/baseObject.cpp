////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : base classes for catalogs, docs etc..  
////////////////////////////////////////////////////////////////////////////

#include "baseObject.h"
#include "metadata/moduleManager/moduleManager.h"
#include "compiler/systemObjects.h"
#include "database/databaseLayer.h"
#include "metadata/metadata.h"
#include "metadata/objects/tablePart/tablePart.h"
#include "utils/stringutils.h"
#include "appData.h"

//***********************************************************************
//*                         metadata                                    * 
//***********************************************************************

wxIMPLEMENT_ABSTRACT_CLASS(IMetaObjectValue, IMetaObject)

//*******************************************************************
//*                           MetaObjectValue                       *
//*******************************************************************

#include "compiler/methods.h"
#include "utils/fs/fs.h"


IMetaObjectValue::IMetaObjectValue() :
	IMetaObject()
{
}

IMetaObjectValue::~IMetaObjectValue()
{
}

IMetaObject *IMetaObjectValue::FindMetaObjectByID(meta_identifier_t meta_id)
{
	return m_metaData->GetMetaObject(meta_id);
}

IMetaObjectValue *IMetaObjectValue::FindMetaObjectValue(meta_identifier_t meta_id)
{
	return dynamic_cast<IMetaObjectValue *>(metadata->GetMetaObject(meta_id));
}

bool IMetaObjectValue::OnLoadMetaObject()
{
	return true;
}

bool IMetaObjectValue::OnSaveMetaObject()
{
	return true;
}

bool IMetaObjectValue::DeleteData()
{
	return RemoveMetaObjectInDB();
}

std::vector<CValue> IMetaObjectRefValue::GetListData(int minValue)
{
	wxString sTableName = GetSpecialTableNameDB();
	wxString sQuery = "SELECT * FROM '" + sTableName + "' ";

	if (minValue) sQuery << " LIMIT " << minValue << ";";
	else sQuery << ";";

	PreparedStatement *statement = objectDatabase->PrepareStatement(sQuery);
	DatabaseResultSet *resultSet = statement->RunQueryWithResults();

	std::vector<CValue> aListData;

	while (resultSet->Next())
	{
		wxString guid = resultSet->GetResultString(GUID_NAME);
		aListData.push_back(new CValueReference(this, guid));
	}

	resultSet->Close();
	objectDatabase->CloseStatement(statement);

	return aListData;
}

//*******************************************************************
//*                            ARRAY                                *
//*******************************************************************

std::vector<CAttributeObject *> IMetaObjectValue::GetObjectAttributes()
{
	std::vector<CAttributeObject *> m_aAttributes;

	for (auto metaObj : m_metaObjects)
	{
		if (metaObj->GetClsid() == g_metaAttributeCLSID)
			m_aAttributes.push_back(dynamic_cast<CAttributeObject *>(metaObj));
	}

	return m_aAttributes;
}

std::vector<CEnumerationObject *> IMetaObjectValue::GetObjectEnums()
{
	std::vector<CEnumerationObject *> m_aEnumerations;

	for (auto metaObj : m_metaObjects)
	{
		if (metaObj->GetClsid() == g_metaEnumCLSID)
			m_aEnumerations.push_back(dynamic_cast<CEnumerationObject *>(metaObj));
	}

	return m_aEnumerations;
}

std::vector<CMetaFormObject *> IMetaObjectValue::GetObjectForms()
{
	std::vector<CMetaFormObject *> m_aAttributes;

	for (auto metaObj : m_metaObjects)
	{
		if (metaObj->GetClsid() == g_metaFormCLSID)
			m_aAttributes.push_back(dynamic_cast<CMetaFormObject *>(metaObj));
	}

	return m_aAttributes;
}

std::vector<CMetaModuleObject *> IMetaObjectValue::GetObjectModules()
{
	std::vector<CMetaModuleObject *> m_aAttributes;

	for (auto metaObj : m_metaObjects)
	{
		if (metaObj->GetClsid() == g_metaModuleCLSID)
			m_aAttributes.push_back(dynamic_cast<CMetaModuleObject *>(metaObj));
	}

	return m_aAttributes;
}

std::vector<CMetaGridObject *> IMetaObjectValue::GetObjectTemplates()
{
	std::vector<CMetaGridObject *> m_aAttributes;

	for (auto metaObj : m_metaObjects)
	{
		if (metaObj->GetClsid() == g_metaTemplateCLSID)
			m_aAttributes.push_back(dynamic_cast<CMetaGridObject *>(metaObj));
	}

	return m_aAttributes;
}

std::vector<CTableObject *> IMetaObjectValue::GetObjectTables()
{
	std::vector<CTableObject *> m_aAttributes;

	for (auto metaObj : m_metaObjects)
	{
		if (metaObj->GetClsid() == g_metaTableCLSID)
			m_aAttributes.push_back(dynamic_cast<CTableObject *>(metaObj));
	}

	return m_aAttributes;
}

//*******************************************************************
//*                           IMetaObjectRefValue                   *
//*******************************************************************

wxIMPLEMENT_ABSTRACT_CLASS(IMetaObjectRefValue, IMetaObjectValue)

IMetaObject *IMetaObjectRefValue::FindMetaObjectByID(meta_identifier_t metaid)
{
	for (auto metaObj : GetObjects())
	{
		if (metaid == metaObj->GetMetaID())
			return metaObj;
	}

	return NULL;
}

CValueReference *IMetaObjectRefValue::FindObjectValue(const Guid &guid)
{
	return new CValueReference(this, guid);
}

IMetaObjectRefValue *IMetaObjectRefValue::FindMetaObjectValue(meta_identifier_t meta_id)
{
	return dynamic_cast<IMetaObjectRefValue *>(metadata->GetMetaObject(meta_id));
}

//*******************************************************************
//*                           IDataObjectSource                     *
//*******************************************************************

#include "forms/elements/frame.h"

IDataObjectSource::IDataObjectSource() : m_valueFrameObject(NULL)
{
}

IDataObjectSource::~IDataObjectSource()
{
	wxDELETE(m_valueFrameObject);
}

//*******************************************************************
//*                           IDataObjectValue                      *
//*******************************************************************

wxIMPLEMENT_ABSTRACT_CLASS(IDataObjectValue, CValue);

IDataObjectValue::IDataObjectValue() : CValue(eValueTypes::TYPE_VALUE),
IDataObjectSource(), IModuleInfo(),
m_methods(new CMethods())
{
}

IDataObjectValue::IDataObjectValue(const IDataObjectValue &source) : CValue(eValueTypes::TYPE_VALUE),
IDataObjectSource(source), IModuleInfo(),
m_methods(new CMethods())
{
}

wxString IDataObjectValue::GetTypeString() const
{
	IMetaObjectValue *metaObjectValue = GetMetaObject();
	wxASSERT(metaObjectValue);
	wxString sType = metaObjectValue->GetClassName();
	sType << wxT("Object.") << metaObjectValue->GetName();
	return sType;
}

wxString IDataObjectValue::GetString() const
{
	IMetaObjectValue *metaObjectValue = GetMetaObject();
	wxASSERT(metaObjectValue);
	return metaObjectValue->GetClassName();
}

IDataObjectValue::~IDataObjectValue()
{
	wxDELETE(m_methods);
}

//**************************************************************************************************
//*                                  IDataObjectRefValue                                           *           
//**************************************************************************************************

wxIMPLEMENT_ABSTRACT_CLASS(IDataObjectRefValue, IDataObjectValue);

IDataObjectRefValue::IDataObjectRefValue(IMetaObjectRefValue *metaObjValue) : IDataObjectValue(),
m_metaObject(metaObjValue), m_bNewObject(true)
{
	m_objGuid = Guid::newGuid();
	m_reference_impl = new reference_t(metaObjValue->GetMetaID(), m_objGuid);
}

IDataObjectRefValue::IDataObjectRefValue(IMetaObjectRefValue *metaObjValue, const Guid &objGuid) : IDataObjectValue(),
m_metaObject(metaObjValue), m_bNewObject(false)
{
	m_objGuid = objGuid;
	m_reference_impl = new reference_t(metaObjValue->GetMetaID(), m_objGuid);
}

IDataObjectRefValue::~IDataObjectRefValue()
{
	wxDELETE(m_reference_impl);
}

void IDataObjectRefValue::PrepareEmptyObject()
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
		m_aObjectValues[StringUtils::MakeUpper(table->GetName())] = new CValueTabularRefSection(m_metaObject, table, m_objGuid, m_bNewObject);
		m_aObjectTypes[StringUtils::MakeUpper(table->GetName())] = wxNOT_FOUND;
	}
}

CValueReference *IDataObjectRefValue::GetReference()
{
	if (m_bNewObject) {
		return new CValueReference(m_metaObject);
	}
	else {
		return new CValueReference(m_metaObject, m_objGuid);
	}
}

wxString IDataObjectRefValue::GetTypeString() const
{
	wxString sType = m_metaObject->GetClassName();
	sType << wxT("Object.") << m_metaObject->GetName();
	return sType;
}

wxString IDataObjectRefValue::GetString() const
{
	if (m_bNewObject) {
		return m_metaObject->GetDescription(m_aObjectValues);
	}
	else {
		return m_metaObject->GetDescription(m_aObjectValues, m_objGuid);
	}
}

//*******************************************************************
//*                           IDataObjectList                       *
//*******************************************************************

IDataObjectList::IDataObjectList(IMetaObjectRefValue *metaObjValue, IMetaFormObject *formObject, IOwnerInfo *ownerControl) : IDataObjectSource(),
m_metaObject(metaObjValue), m_ownerInfo(ownerControl),
m_nFormType(formObject->GetTypeForm()), m_objGuid(Guid::newGuid()), 
m_currentLine(wxNOT_FOUND)
{
	m_valueFrameObject = formObject->GenerateFormAndRun(metaObjValue, this);
}

IDataObjectList::~IDataObjectList()
{
}

//events 
void IDataObjectList::AddValue()
{
	IDataObjectValue *dataValue = m_metaObject->CreateObjectValue();
	if (dataValue) {
		dataValue->ShowValue();
	}
}

void IDataObjectList::CopyValue()
{
	if (m_currentLine == wxNOT_FOUND)
		return;

	auto itFounded = m_aTableGuidValues.begin();
	std::advance(itFounded, m_currentLine - 1);

	IDataObjectValue *objData = m_metaObject->CreateObjectValue(*itFounded);
	
	if (!objData->IsEmpty())
	{
		//CValue vNewObject = m_object->CopyObject();
		//vNewObject.ShowValue();
	}
}

void IDataObjectList::EditValue()
{
	if (m_currentLine == wxNOT_FOUND)
		return;

	auto itFounded = m_aTableGuidValues.begin();
	std::advance(itFounded, m_currentLine - 1);

	CValueReference *reference = m_metaObject->FindObjectValue(*itFounded);
	reference->ShowValue();
}

void IDataObjectList::DeleteValue()
{
	if (m_currentLine == wxNOT_FOUND)
		return;

	auto itFounded = m_aTableGuidValues.begin();
	std::advance(itFounded, m_currentLine - 1);

	IDataObjectValue *objData = m_metaObject->CreateObjectValue(*itFounded);
	
	if (!objData->IsEmpty()) {
		//objData->DeleteObject();
	}

	UpdateModel();
}

void IDataObjectList::SelectValue()
{
	if (m_currentLine == wxNOT_FOUND)
		return;

	auto itFounded = m_aTableGuidValues.begin();
	std::advance(itFounded, m_currentLine - 1);

	if (m_ownerInfo) {
		CValue reference = m_metaObject->FindObjectValue(*itFounded);
		m_ownerInfo->SendOwnerValue(reference);
		m_valueFrameObject->CloseForm();
	}
}
