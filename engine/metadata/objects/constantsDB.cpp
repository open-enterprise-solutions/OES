////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : constants - db
////////////////////////////////////////////////////////////////////////////

#include "constants.h"
#include "database/databaseLayer.h"
#include "database/databaseErrorCodes.h"
#include "utils/stringutils.h"
#include "appData.h"

#define TABLE_NAME wxT("const_")

wxString CConstantObject::GetSpecialTableNameDB()
{
	return TABLE_NAME + StringUtils::IntToStr(GetMetaID());
}

#define FIELD_NAME wxT("fld_")

wxString CConstantObject::GetSpecialFiledNameDB()
{
	return FIELD_NAME + StringUtils::IntToStr(m_metaID) + GetSuffixObject();
}

bool CConstantObject::ReadConstInDB()
{
	wxString sTableName = GetSpecialTableNameDB();

	if (objectDatabase->TableExists(sTableName))
	{
		/*wxString sFieldName = GetSpecialFiledNameDB();
		DatabaseResultSet *resultSet = objectDatabase->RunQueryWithResults("SELECT FIRST 1 " + sFieldName + " FROM " + sTableName);
		if (resultSet->Next()) {}
		resultSet->Close();*/
	}

	return true; 
}

bool CConstantObject::SaveConstInDB()
{
	wxString sTableName = GetSpecialTableNameDB();
	wxString sFieldName = GetSpecialFiledNameDB();

	if (!objectDatabase->TableExists(sTableName)) {
		
		int retCode = objectDatabase->RunQuery("CREATE TABLE " + sTableName + " (" + sFieldName + " " + GetSQLTypeObject() + ", RECORD_KEY CHAR DEFAULT '0x31' PRIMARY KEY);");
		
		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;
	}
	else {
		DatabaseResultSet *resultSet = objectDatabase->RunQueryWithResults("SELECT FIRST 0 * FROM " + sTableName);

		if (!resultSet)
			return false;

		ResultSetMetaData *mrs = resultSet->GetMetaData();

		if (!mrs)
			return false;

		for (int col = 1; col < mrs->GetColumnCount(); col++) {

			wxString colName = mrs->GetColumnName(col);

			if (colName == wxT("RECORD_KEY"))
				continue;

			if (colName != sFieldName) {

				int retCode = objectDatabase->RunQuery("ALTER TABLE " + sTableName + " DROP " + colName + ";");
				
				if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
					return false;
			}
		}

		if (mrs->FindColumnByName(sFieldName) == wxNOT_FOUND) {
			
			int retCode = objectDatabase->RunQuery("ALTER TABLE " + sTableName + " ADD " + sFieldName + " " + GetSQLTypeObject() + ";");
			
			if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
				return false;
		}

		resultSet->Close();
	}

	return true;
}

bool CConstantObject::DeleteConstInDB()
{
	wxString sTableName = GetSpecialTableNameDB();

	if (objectDatabase->TableExists(sTableName)) {
		
		int retCode = objectDatabase->RunQuery("DROP TABLE " + sTableName);
		
		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;
	}

	return true;
}