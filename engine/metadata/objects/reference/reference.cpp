////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : reference object
////////////////////////////////////////////////////////////////////////////

#include "reference.h"
#include "metadata/metadata.h"
#include "metadata/objects/baseObject.h"
#include "metadata/objects/tablePart/tablePart.h"
#include "database/databaseLayer.h"
#include "compiler/methods.h"
#include "utils/stringutils.h"

//**********************************************************************************************
//*                                     reference                                              *        
//**********************************************************************************************

void CValueReference::PrepareReference()
{
	wxASSERT(m_metaObject);

	if (IsEmpty())
	{
		m_reference_impl = NULL;

		//attrbutes can refValue 
		for (auto attribute : m_metaObject->GetObjectAttributes())
		{
			if (eValueTypes::TYPE_BOOLEAN == attribute->GetTypeObject() ||
				eValueTypes::TYPE_NUMBER == attribute->GetTypeObject() ||
				eValueTypes::TYPE_DATE == attribute->GetTypeObject() ||
				eValueTypes::TYPE_STRING == attribute->GetTypeObject()) m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes(attribute->GetTypeObject());

			m_aObjectTypes[StringUtils::MakeUpper(attribute->GetName())] = attribute->GetTypeObject();
		}

		// table is collection values 
		for (auto table : m_metaObject->GetObjectTables())
		{
			m_aObjectValues[StringUtils::MakeUpper(table->GetName())] = new CValueTabularRefSection(m_metaObject, table, m_objGuid);
			m_aObjectValues[table->GetName()] = wxNOT_FOUND;
		}
	}
	else
	{
		m_reference_impl = new reference_t(m_metaObject->GetMetaID(), m_objGuid);

		if (ReadReferenceInDB()) {
			m_existInDB = true;
		}
	}
}

CValueReference::CValueReference(meta_identifier_t metaID, const Guid &objGuid) : CValue(eValueTypes::TYPE_VALUE, true), m_methods(new CMethods()),
m_metaObject(wxStaticCast(metadata->GetMetaObject(metaID), IMetaObjectRefValue)),
m_objGuid(objGuid), m_existInDB(false)
{
	PrepareReference();
}

CValueReference::CValueReference(IMetaObjectRefValue *metaObject, const Guid &objGuid) : CValue(eValueTypes::TYPE_VALUE, true), m_methods(new CMethods()),
m_metaObject(metaObject),
m_objGuid(objGuid), m_existInDB(false)
{
	PrepareReference();
}

CValueReference::~CValueReference()
{
	wxDELETE(m_methods);
}

CValue CValueReference::CreateFromPtr(void *ptr)
{
	reference_t *reference = static_cast<reference_t *>(ptr);
	if (reference) return new CValueReference(reference->m_id, reference->m_guid);
	return CValue();
}

void CValueReference::SetBinaryData(int nPosition, PreparedStatement *preparedStatment)
{
	if (m_reference_impl) {
		preparedStatment->SetParamBlob(nPosition, m_reference_impl, sizeof(reference_t));
	}
}

void CValueReference::GetBinaryData(int nPosition, DatabaseResultSet *databaseResultSet)
{
	wxMemoryBuffer memoryBuffer;
	databaseResultSet->GetResultBlob(nPosition, memoryBuffer);

	if (!memoryBuffer.IsEmpty())
	{
		reference_t *m_reference_temp = static_cast<reference_t *>(memoryBuffer.GetData());

		m_metaObject = dynamic_cast<IMetaObjectRefValue *>(metadata->GetMetaObject(m_reference_temp->m_id));
		m_objGuid = m_reference_temp->m_guid;

		if (m_metaObject) {
			m_reference_impl = new reference_t(m_reference_temp->m_id, m_reference_temp->m_guid);
		}
	}
}

void CValueReference::ShowValue()
{
	IDataObjectValue *m_objValue = NULL;

	if (m_metaObject && m_objGuid.isValid()) {
		m_objValue = m_metaObject->CreateObjectValue(m_objGuid);
	}
	else {
		m_objValue = m_metaObject->CreateObjectValue();
	}

	if (m_objValue) {
		m_objValue->ShowFrameValue();
	}
}

CValue CValueReference::IsEmptyRef()
{
	return IsEmpty();
}

CValue CValueReference::GetMetadata()
{
	wxASSERT(m_metaObject);
	return m_metaObject;
}

CValue CValueReference::GetObject()
{
	if (!m_objGuid.isValid()) {
		return m_metaObject->CreateObjectValue();
	}

	return m_metaObject->CreateObjectValue(m_objGuid);
}

unsigned int CValueReference::GetTypeRef() 
{ 
	return m_metaObject->GetMetaID(); 
}

wxString CValueReference::GetString() const
{
	if (!m_objGuid.isValid() || !m_metaObject) 
		return wxEmptyString;

	if (!m_existInDB)
	{
		wxString sNullRef;
		sNullRef << wxT("not found <") << m_metaObject->GetMetaID() << wxT(":") + m_objGuid.str() << wxT(">");
		return sNullRef;
	}

	return m_metaObject->GetDescription(m_aObjectValues, m_objGuid);
}

wxString CValueReference::GetTypeString() const
{
	wxString sType = m_metaObject->GetClassName();
	sType << wxT("Ref.") << m_metaObject->GetName();
	return sType;
}