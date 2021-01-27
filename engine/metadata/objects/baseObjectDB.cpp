////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : base db reader/saver
////////////////////////////////////////////////////////////////////////////

#include "baseObject.h"
#include "appData.h"
#include "database/databaseLayer.h"
#include "database/databaseErrorCodes.h"
#include "metadata/objects/tablePart/tablePart.h"
#include "utils/stringutils.h"

#define FIELD_NAME wxT("fld_")

//**********************************************************************************************************
//*                                          Common functions                                              *
//**********************************************************************************************************

wxString IMetaObjectValue::GetSpecialTableNameDB()
{
	wxString sTableName = GetSpecialTableNameSpaceDB();
	return sTableName << GetMetaID();
}

wxString IMetaObjectValue::GetSpecialFiledNameSpaceDB()
{
	return FIELD_NAME;
}

wxString IMetaObjectValue::GetSpecialFiledNameDB(CAttributeObject *pAttribute)
{
	wxString sFieldName = GetSpecialFiledNameSpaceDB();
	return sFieldName << pAttribute->GetMetaID() << pAttribute->GetSuffixObject();
}

int IMetaObjectValue::GetTypeIDFromTableDB(const wxString &table)
{
	wxString sTable = table;
	sTable.Replace(GetSpecialTableNameSpaceDB(), wxEmptyString);
	return wxAtoi(sTable);
}

int IMetaObjectValue::GetTypeIDFromFieldDB(const wxString &field)
{
	long nType = wxNOT_FOUND;

	wxString sField = field;
	sField.Replace(GetSpecialFiledNameSpaceDB(), wxEmptyString);
	sField = sField.Left(sField.Length() - 1);

	if (sField.ToLong(&nType)) return nType;
	return nType;
}

eValueTypes IMetaObjectValue::GetTypeFromFieldDB(const wxString &field)
{
	wxString sField = field;
	sField.Replace(GetSpecialFiledNameSpaceDB(), wxEmptyString);

	char c = sField[sField.Length() - 1];
	switch (c)
	{
	case 'B': return eValueTypes::TYPE_BOOLEAN;
	case 'N': return eValueTypes::TYPE_NUMBER;
	case 'D': return eValueTypes::TYPE_DATE;
	case 'S': return eValueTypes::TYPE_STRING;
	case 'R': return eValueTypes::TYPE_REFFER;
	};

	return eValueTypes::TYPE_EMPTY;
}

//**********************************************************************************************************
//*                                          Query functions                                               *
//**********************************************************************************************************

bool IDataObjectRefValue::ReadInDB()
{
	if (!m_metaObject || m_bNewObject)
		return false;

	wxString sTableName = m_metaObject->GetSpecialTableNameDB();

	if (objectDatabase->TableExists(sTableName))
	{
		bool m_isExist = false;

		wxString sql = "SELECT FIRST 1 * FROM " + sTableName + " WHERE UUID = '" + m_objGuid.str() + "';";
		DatabaseResultSet *m_resultSet = objectDatabase->RunQueryWithResults(sql);

		if (!m_resultSet) return false;

		if (m_resultSet->Next())
		{
			//load other attributes 
			for (auto attribute : m_metaObject->GetObjectAttributes())
			{
				wxString sNameAttribute = m_metaObject->GetSpecialFiledNameDB(attribute);

				switch (attribute->GetTypeObject())
				{
				case eValueTypes::TYPE_BOOLEAN:
					m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = m_resultSet->GetResultBool(sNameAttribute); break;
				case eValueTypes::TYPE_NUMBER:
					m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = m_resultSet->GetResultNumber(sNameAttribute); break;
				case eValueTypes::TYPE_DATE:
					m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = m_resultSet->GetResultDate(sNameAttribute); break;
				case eValueTypes::TYPE_STRING:
					m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = m_resultSet->GetResultString(sNameAttribute); break;
				default:
				{
					wxMemoryBuffer m_buffer;
					m_resultSet->GetResultBlob(sNameAttribute, m_buffer);
					if (!m_buffer.IsEmpty()) m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())] = CValueReference::CreateFromPtr(m_buffer.GetData()); break;
				}
				}
				m_aObjectTypes[StringUtils::MakeUpper(attribute->GetName())] = attribute->GetTypeObject();
			}

			m_isExist = true;
		}

		m_resultSet->Close();

		for (auto currTable : m_metaObject->GetObjectTables())
		{
			CValueTabularRefSection *tabularSection = new CValueTabularRefSection(m_metaObject, currTable, m_objGuid, m_bNewObject);
			
			if (m_isExist && tabularSection->LoadDataFromDB()) {
				m_aTableParts.push_back(tabularSection);
			}

			m_aObjectValues[StringUtils::MakeUpper(currTable->GetName())] = tabularSection;
			m_aObjectTypes[StringUtils::MakeUpper(currTable->GetName())] = wxNOT_FOUND;
		}

		return m_isExist;
	}

	return false;
}

bool IDataObjectRefValue::SaveInDB()
{
	wxASSERT(m_metaObject);

	wxString sTableName = m_metaObject->GetSpecialTableNameDB();
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

	sQuery += ") MATCHING (UUID);";

	PreparedStatement *m_statement = objectDatabase->PrepareStatement(sQuery);
	if (!m_statement) return false;

	m_objGuid = m_reference_impl->m_guid;

	m_statement->SetParamString(1, m_objGuid.str());
	m_statement->SetParamBlob(2, m_reference_impl, sizeof(reference_t));

	int nPosition = 3;

	for (auto attribute : m_metaObject->GetObjectAttributes())
	{
		m_aObjectValues[StringUtils::MakeUpper(attribute->GetName())].SetBinaryData(nPosition, m_statement); nPosition++;
	}

	bool m_bError = m_statement->RunQuery() == DATABASE_LAYER_QUERY_RESULT_ERROR;

	objectDatabase->CloseStatement(m_statement);

	//table parts
	if (!m_bError)
	{
		for (auto table : m_aTableParts)
		{
			if (!table->SaveDataInDB()) {
				m_bError = true; break;
			}
		}
	}

	if (!m_bError) {
		m_bNewObject = false;
	}

	return !m_bError;
}

bool IDataObjectRefValue::DeleteInDB()
{
	if (m_bNewObject) 
		return false;

	wxString sTableName = m_metaObject->GetSpecialTableNameDB();
	bool m_bError = objectDatabase->RunQuery("DELETE FROM " + sTableName + " WHERE UUID = '" + m_objGuid.str() + "';") == DATABASE_LAYER_QUERY_RESULT_ERROR;

	//table parts
	for (auto table : m_aTableParts)
	{
		if (!table->DeleteDataInDB()) 
		{ m_bError = true; break; }
	}

	return !m_bError;
}