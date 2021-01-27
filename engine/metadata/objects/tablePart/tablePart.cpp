////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : tabular sections
////////////////////////////////////////////////////////////////////////////

#include "tablePart.h"
#include "appData.h"
#include "database/databaseLayer.h"
#include "database/databaseErrorCodes.h"
#include "metadata/objects/baseObject.h"
#include "utils/stringutils.h"

wxIMPLEMENT_ABSTRACT_CLASS(IValueTabularSection, IValueTable);
wxIMPLEMENT_DYNAMIC_CLASS(IValueTabularSection::CValueTabularSectionReturnLine, IValueTable::IValueTableReturnLine);
wxIMPLEMENT_DYNAMIC_CLASS(CValueTabularSection, IValueTabularSection);
wxIMPLEMENT_DYNAMIC_CLASS(CValueTabularRefSection, IValueTabularSection);

//////////////////////////////////////////////////////////////////////
//               CValueTabularSection                               //
//////////////////////////////////////////////////////////////////////

CValueTabularSection::CValueTabularSection() {}

CValueTabularSection::CValueTabularSection(IMetaObjectValue *metaObject, CTableObject *tableObject, IDataObjectValue *dataObject) : IValueTabularSection(metaObject, tableObject),
m_metaObject(metaObject), m_dataObject(dataObject) {}

CValueTabularSection::~CValueTabularSection() {}

#include "metadata/objects/baseObject.h"

wxString CValueTabularSection::GetTypeString() const
{
	return wxT("tablePart.") + m_metaObject->GetName();
}

wxString CValueTabularSection::GetString() const
{
	return wxT("tablePart.") + m_metaObject->GetName();
}

//////////////////////////////////////////////////////////////////////
//               CValueTabularRefSection                            //
//////////////////////////////////////////////////////////////////////

CValueTabularRefSection::CValueTabularRefSection() {}

CValueTabularRefSection::CValueTabularRefSection(IMetaObjectRefValue *metaObject, CTableObject *tableObject, const Guid &objGuid, bool newObject) : IValueTabularSection(metaObject, tableObject),
m_metaObject(m_metaObject), m_objGuid(objGuid), m_bNewObject(newObject) {}

CValueTabularRefSection::~CValueTabularRefSection() {}

bool CValueTabularRefSection::LoadDataFromDB()
{
	if (!m_bNewObject)
		return false;

	wxString sTableName = m_metaObject->GetSpecialTableNameSpaceDB() + TABLE_PART_NAME + StringUtils::IntToStr(m_metaTableObj->GetMetaID());
	wxString sql = "SELECT * FROM " + sTableName + " WHERE UUID = '" + m_objGuid.str() + "'";

	DatabaseResultSet *resultSet = objectDatabase->RunQueryWithResults(sql);
	if (!resultSet) return false;

	while (resultSet->Next())
	{
		CDataTable aRowTable;

		for (auto attribute : m_metaTableObj->GetObjectAttributes())
		{
			wxString sNameAttribute = m_metaObject->GetSpecialFiledNameDB(attribute);

			switch (attribute->GetTypeObject())
			{
			case eValueTypes::TYPE_BOOLEAN: aRowTable.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = resultSet->GetResultBool(sNameAttribute); break;
			case eValueTypes::TYPE_NUMBER: aRowTable.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = resultSet->GetResultDouble(sNameAttribute); break;
			case eValueTypes::TYPE_DATE: aRowTable.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = resultSet->GetResultDate(sNameAttribute); break;
			case eValueTypes::TYPE_STRING: aRowTable.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = resultSet->GetResultString(sNameAttribute); break;
			default:
			{
				wxMemoryBuffer m_buffer;
				resultSet->GetResultBlob(sNameAttribute, m_buffer);
				if (!m_buffer.IsEmpty()) aRowTable.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = CValueReference::CreateFromPtr(m_buffer.GetData()); break;
			}
			aRowTable.aDataTypes[StringUtils::MakeUpper(attribute->GetName())] = attribute->GetTypeObject();
			}
		}
		m_aObjectValues.push_back(aRowTable);
	}
	resultSet->Close();
	return true;
}

bool CValueTabularRefSection::SaveDataInDB()
{
	bool m_bError = false;
	reference_t *m_reference_impl = new reference_t(m_metaObject->GetMetaID(), m_objGuid);

	wxString sTableName = m_metaObject->GetSpecialTableNameSpaceDB() + TABLE_PART_NAME + StringUtils::IntToStr(m_metaTableObj->GetMetaID());
	wxString sQuery = "UPDATE OR INSERT INTO " + sTableName + " (";

	sQuery += "UUID, UUIDREF";

	for (auto attribute : m_metaObject->GetObjectAttributes())
	{
		sQuery = sQuery + ", " + m_metaObject->GetSpecialFiledNameDB(attribute);
	}

	sQuery += ") VALUES (?, ?";

	for (auto attribute : m_metaObject->GetObjectAttributes())
	{
		sQuery += ", ?";
	}

	sQuery += ");";

	PreparedStatement *statement = objectDatabase->PrepareStatement(sQuery);

	statement->SetParamString(1, m_objGuid.str());
	statement->SetParamBlob(2, m_reference_impl, sizeof(reference_t));

	for (auto objectValue : m_aObjectValues)
	{
		if (m_bError) break;

		int nPosition = 2;

		for (auto attribute : m_metaTableObj->GetObjectAttributes())
		{
			objectValue.aDataValues[StringUtils::MakeUpper(m_metaTableObj->GetName())].SetBinaryData(nPosition, statement); nPosition++;
		}

		m_bError = statement->RunQuery() == DATABASE_LAYER_QUERY_RESULT_ERROR;
	}

	objectDatabase->CloseStatement(statement);
	
	delete m_reference_impl;

	if (!m_bError) {
		m_bNewObject = false;
	}

	return !m_bError;
}

bool CValueTabularRefSection::DeleteDataInDB()
{
	if (!m_bNewObject)
		return false;

	wxString sTableName = m_metaObject->GetSpecialTableNameSpaceDB() + TABLE_PART_NAME + StringUtils::IntToStr(m_metaObject->GetMetaID());
	objectDatabase->RunQuery("DELETE FROM " + sTableName + " WHERE UUID = '" + m_objGuid.str() + "';");
	return true;
}

wxString CValueTabularRefSection::GetTypeString() const
{
	return wxT("tablePart.") + m_metaObject->GetName();
}

wxString CValueTabularRefSection::GetString() const
{
	return wxT("tablePart.") + m_metaObject->GetName();
}

//////////////////////////////////////////////////////////////////////
//               CValueTabularSectionReturnLine                     //
//////////////////////////////////////////////////////////////////////

#include "compiler/methods.h"

void IValueTabularSection::CValueTabularSectionReturnLine::PrepareNames() const
{
	std::vector<SEng> aAttributes;

	for (auto attribute : m_ownerTable->m_metaTableObj->GetObjectAttributes())
	{
		SEng aAttribute;
		aAttribute.sName = attribute->GetName();
		aAttribute.sAlias = wxT("default");
		aAttributes.push_back(aAttribute);
	}

	m_methods.PrepareAttributes(aAttributes.data(), aAttributes.size());
}

CMethods IValueTabularSection::CValueTabularSectionReturnLine::m_methods;
