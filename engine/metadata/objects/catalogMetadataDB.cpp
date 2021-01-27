////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : catalog metadata - db
////////////////////////////////////////////////////////////////////////////

#include "catalog.h"
#include "metadata/metadata.h"
#include "database/databaseLayer.h"
#include "database/databaseErrorCodes.h"
#include "metadata/objects/tablePart/tablePart.h"
#include "window/mainFrame.h"
#include "utils/stringutils.h"

#define TABLE_NAME wxT("catalog_")

//**********************************************************************************************************
//*                                          Common functions                                              *
//**********************************************************************************************************

wxString CMetaObjectCatalogValue::GetSpecialTableNameSpaceDB()
{
	return TABLE_NAME;
}

//**********************************************************************************************************
//*                                          Metaobject in DB                                              *
//**********************************************************************************************************

#include "reference/reference.h"

bool CMetaObjectCatalogValue::CreateMetaObjectInDB()
{
	wxString sTableName = GetSpecialTableNameDB();

	if (objectDatabase->TableExists(sTableName))
	{
		wxString sql = "SELECT FIRST 0 * FROM " + sTableName;

		DatabaseResultSet *rs = objectDatabase->RunQueryWithResults(sql);
		ResultSetMetaData *mrs = rs->GetMetaData();

		for (auto attribute : GetObjectAttributes())
		{
			wxString sFieldName = GetSpecialFiledNameDB(attribute);

			if (mrs->FindColumnByName(sFieldName) == wxNOT_FOUND) {
				
				int retCode = objectDatabase->RunQuery("ALTER TABLE " + sTableName + " ADD " + sFieldName + " " + attribute->GetSQLTypeObject() + ";");
				
				if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
					return false;
			}
		}

		rs->Close();
	}
	else
	{
		int retCode = 0;

		retCode = objectDatabase->RunQuery("CREATE TABLE " + sTableName + " ("
			"UUID             VARCHAR(36) NOT NULL PRIMARY KEY,"
			"UUIDREF          BLOB,"
			"CODE             CHAR(11),"
			"NAME             VARCHAR(150),"
			"IS_GROUP         SMALLINT);");

		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;

		for (auto attribute : GetObjectAttributes())
		{
			wxString sFieldName = GetSpecialFiledNameDB(attribute);
			retCode = objectDatabase->RunQuery("ALTER TABLE " + sTableName + " ADD " + sFieldName + " " + attribute->GetSQLTypeObject() + ";");

			if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
				return false;
		}

		retCode = objectDatabase->RunQuery("CREATE INDEX " + sTableName + "_INDEX ON " + sTableName + " ("
			"UUID,"
			"CODE,"
			"NAME,"
			"IS_GROUP);");

		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;
	}

	for (auto table : GetObjectTables())
	{
		wxString sTableName = GetSpecialTableNameSpaceDB() + TABLE_PART_NAME + StringUtils::IntToStr(table->GetMetaID());
		if (objectDatabase->TableExists(sTableName))
		{
			wxString sql = "SELECT FIRST 0 * FROM " + sTableName;

			DatabaseResultSet *rs = objectDatabase->RunQueryWithResults(sql);
			ResultSetMetaData *mrs = rs->GetMetaData();

			for (auto attribute : table->GetObjectAttributes())
			{
				wxString sFieldName = GetSpecialFiledNameDB(attribute);

				if (mrs->FindColumnByName(sFieldName) == wxNOT_FOUND) {
					
					int retCode = objectDatabase->RunQuery("ALTER TABLE " + sTableName + " ADD " + sFieldName + " " + attribute->GetSQLTypeObject() + ";");
					
					if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
						return false;
				}
			}

			rs->Close();
		}
		else
		{
			int retCode = objectDatabase->RunQuery("CREATE TABLE " + sTableName + " ("
				"UUID             CHAR(36)          NOT NULL PRIMARY KEY,"
				"UUIDREF          BLOB);");

			for (auto attribute : table->GetObjectAttributes())
			{
				wxString sFieldName = GetSpecialFiledNameDB(attribute);

				retCode = objectDatabase->RunQuery("ALTER TABLE " + sTableName + " ADD " + sFieldName + " " + attribute->GetSQLTypeObject() + ";");

				if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
					return false;
			}

			retCode = objectDatabase->RunQuery("CREATE INDEX " + sTableName + "_INDEX ON " + sTableName + " (UUID);");

			if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
				return false;
		}
	}

	return true;
}

bool CMetaObjectCatalogValue::RemoveMetaObjectInDB()
{
	wxString sTableName = GetSpecialTableNameDB();

	if (objectDatabase->TableExists(sTableName)) {
		
		int retCode = objectDatabase->RunQuery(wxT("DROP TABLE ") + sTableName);

		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;
	}

	for (auto table : GetObjectTables())
	{
		wxString sTableName = GetSpecialTableNameSpaceDB() + TABLE_PART_NAME + StringUtils::IntToStr(table->GetMetaID());

		if (objectDatabase->TableExists(sTableName)) {
			
			int retCode = objectDatabase->RunQuery(wxT("DROP TABLE ") + sTableName);

			if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
				return false;
		}
	}

	return true;
}

wxString CMetaObjectCatalogValue::GetDescription(const Guid &guid)
{
	wxString sTableName = GetSpecialTableNameDB(), sDesctiption;

	if (objectDatabase->TableExists(sTableName))
	{
		wxString sql = "SELECT NAME FROM " + sTableName + " WHERE UUID = '" + guid.str() + "'";

		DatabaseResultSet *resultSet = objectDatabase->RunQueryWithResults(sql);
		ResultSetMetaData *resultSetMetaData = resultSet->GetMetaData();

		while (resultSet->Next())
		{
			wxString sName = resultSet->GetResultString(m_attributeName->GetSQLFieldName());
			sDesctiption << sName;
		}

		resultSet->Close();
	}

	return sDesctiption;
}

wxString CMetaObjectCatalogValue::GetDescription(const std::map<wxString, CValue> &valData)
{
	CValue vName = valData.find(m_attributeName->GetSQLFieldName())->second;

	wxString sDesctiption;
	sDesctiption << vName.GetString();
	return sDesctiption;
}

wxString CMetaObjectCatalogValue::GetDescription(const std::map<wxString, CValue> &valData, const Guid &objGuid)
{
	CValue vName = valData.find(m_attributeName->GetSQLFieldName())->second;

	wxString sDesctiption;
	sDesctiption << vName.GetString();
	return sDesctiption;
}