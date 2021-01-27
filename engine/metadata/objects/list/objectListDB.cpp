////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : list db 
////////////////////////////////////////////////////////////////////////////

#include "objectList.h"
#include "appData.h"
#include "database/databaseLayer.h"
#include "utils/stringutils.h"

void CObjectListValue::UpdateModel()
{
	m_aColumnInfo.clear();

	wxString sTableName = m_metaObject->GetSpecialTableNameDB();
	wxString sQuery = "SELECT * FROM " + sTableName + " ORDER BY CAST(UUID AS VARCHAR(36)); ";

	for (auto attribute : m_metaObject->GetObjectAttributes())
	{
		m_aColumnInfo[attribute->GetMetaID()] = attribute->GetTypeObject();
	}

	m_aTableValues.clear(); int position = 0;

	PreparedStatement *statement = objectDatabase->PrepareStatement(sQuery);
	DatabaseResultSet *resultSet = statement->RunQueryWithResults();

	while (resultSet->Next())
	{
		std::map<unsigned int, CValue> m_valueRow;

		m_aTableGuidValues.push_back(resultSet->GetResultString(GUID_NAME));

		for (auto attribute : m_metaObject->GetObjectAttributes())
		{
			wxString sFieldName = attribute->GetName();

			switch (attribute->GetTypeObject())
			{
			case eValueTypes::TYPE_BOOLEAN:
				m_valueRow.insert_or_assign(attribute->GetMetaID(), resultSet->GetResultBool(m_metaObject->GetSpecialFiledNameDB(attribute)));
				break;
			case eValueTypes::TYPE_NUMBER:
				m_valueRow.insert_or_assign(attribute->GetMetaID(), resultSet->GetResultDouble(m_metaObject->GetSpecialFiledNameDB(attribute)));
				break;
			case eValueTypes::TYPE_DATE:
				m_valueRow.insert_or_assign(attribute->GetMetaID(), resultSet->GetResultDate(m_metaObject->GetSpecialFiledNameDB(attribute)));
				break;
			case eValueTypes::TYPE_STRING:
				m_valueRow.insert_or_assign(attribute->GetMetaID(), resultSet->GetResultString(m_metaObject->GetSpecialFiledNameDB(attribute)));
				break;
			default:
				wxMemoryBuffer m_buffer;
				resultSet->GetResultBlob(m_metaObject->GetSpecialFiledNameDB(attribute), m_buffer);
				if (!m_buffer.IsEmpty()) { 
					m_valueRow.insert_or_assign(attribute->GetMetaID(), CValueReference::CreateFromPtr(m_buffer.GetData()));
				}
				break;
			}
		}
		
		m_aTableValues.push_back(m_valueRow);
		position++;
	};

	resultSet->Close();

	if (!CTranslateError::IsSimpleMode())
		IValueTable::Reset(m_aTableValues.size());

	objectDatabase->CloseStatement(statement);
}