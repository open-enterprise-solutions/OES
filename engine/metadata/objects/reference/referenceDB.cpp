////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : reference - db
////////////////////////////////////////////////////////////////////////////

#include "reference.h"
#include "appData.h"
#include "metadata/objects/baseObject.h"
#include "metadata/objects/tablePart/tablePart.h"
#include "database/databaseLayer.h"
#include "utils/stringutils.h"

bool CValueReference::ReadReferenceInDB()
{
	if (!m_metaObject || !m_objGuid.isValid())
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
			//load attributes 
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
			CValueTabularRefSection *tabularSection = new CValueTabularRefSection(m_metaObject, currTable, m_objGuid);
			
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