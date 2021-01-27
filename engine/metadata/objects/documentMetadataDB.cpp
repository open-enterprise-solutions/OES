////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : document metadata - db
////////////////////////////////////////////////////////////////////////////

#include "document.h"
#include "metadata/metadata.h"
#include "metadata/objects/tablePart/tablePart.h"
#include "database/databaseLayer.h"
#include "database/databaseErrorCodes.h"
#include "window/mainFrame.h"
#include "utils/stringutils.h"

#define TABLE_NAME wxT("document_")

//**********************************************************************************************************
//*                                          Common functions                                              *
//**********************************************************************************************************

wxString CMetaObjectDocumentValue::GetSpecialTableNameSpaceDB()
{
	return TABLE_NAME;
}

//**********************************************************************************************************
//*                                          Metaobject in DB                                              *
//**********************************************************************************************************

#include "reference/reference.h"

bool CMetaObjectDocumentValue::CreateMetaObjectInDB()
{
	wxString sTableName = GetSpecialTableNameDB();

	if (objectDatabase->TableExists(sTableName))
	{
		wxString sql = "SELECT FIRST 0 * FROM " + sTableName + "";

		DatabaseResultSet *rs = objectDatabase->RunQueryWithResults(sql);
		ResultSetMetaData *mrs = rs->GetMetaData();

		for (auto attribute : GetObjectAttributes())
		{
			wxString sFieldName = GetSpecialFiledNameDB(attribute);

			if (mrs->FindColumnByName(sFieldName) == wxNOT_FOUND) {
				objectDatabase->RunQuery("ALTER TABLE " + sTableName + " ADD " + sFieldName + " " + attribute->GetSQLTypeObject() + ";");
			}

		}

		rs->Close();
	}
	else
	{
		int retCode = 0;

		retCode = objectDatabase->RunQuery("CREATE TABLE " + sTableName + " ("
			"UUID              VARCHAR(36) NOT NULL PRIMARY KEY,"
			"UUIDREF           BLOB,"
			"NUMBER            CHAR(20),"
			"DATE_DOC          TIMESTAMP,"
			"POSTED            SMALLINT);");

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
			"NUMBER,"
			"DATE_DOC,"
			"POSTED);");

		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;
	}

	for (auto table : GetObjectTables())
	{
		wxString sTableName = GetSpecialTableNameSpaceDB() + TABLE_PART_NAME + StringUtils::IntToStr(table->GetMetaID());

		if (objectDatabase->TableExists(sTableName)) {
			wxString sql = "SELECT FIRST 0 * FROM " + sTableName;

			DatabaseResultSet *rs = objectDatabase->RunQueryWithResults(sql);
			ResultSetMetaData *mrs = rs->GetMetaData();

			for (auto attribute : table->GetObjectAttributes())
			{
				wxString sFieldName = GetSpecialFiledNameDB(attribute);

				if (mrs->FindColumnByName(sFieldName) == wxNOT_FOUND)
				{
					objectDatabase->RunQuery("ALTER TABLE " + sTableName + " ADD " + sFieldName + " " + attribute->GetSQLTypeObject() + ";");
				}
			}

			rs->Close();
		}
		else
		{
			int retCode = objectDatabase->RunQuery("CREATE TABLE " + sTableName + " ("
				"UUID             CHAR(36)          NOT NULL PRIMARY KEY,"
				"UUIDREF          BLOB);");

			if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
				return false;

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

bool CMetaObjectDocumentValue::RemoveMetaObjectInDB()
{
	wxString sTableName = GetSpecialTableNameDB();

	if (objectDatabase->TableExists(sTableName))
	{
		int retCode = objectDatabase->RunQuery("DROP TABLE " + sTableName);

		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;
	}

	for (auto table : GetObjectTables())
	{
		wxString sTableName = GetSpecialTableNameSpaceDB() + TABLE_PART_NAME + StringUtils::IntToStr(table->GetMetaID());
		if (objectDatabase->TableExists(sTableName))
		{
			int retCode = objectDatabase->RunQuery("DROP TABLE " + sTableName);

			if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
				return false;
		}
	}

	return true;
}

wxString CMetaObjectDocumentValue::GetDescription(const Guid &guid)
{
	wxString sTableName = GetSpecialTableNameDB(), sDescription;

	if (objectDatabase->TableExists(sTableName))
	{
		wxString sql = "SELECT * FROM " + sTableName + " WHERE UUID = '" + guid.str() + "'";

		DatabaseResultSet *resultSet = objectDatabase->RunQueryWithResults(sql);
		ResultSetMetaData *resultSetMetaData = resultSet->GetMetaData();

		while (resultSet->Next())
		{
			wxString sNumber = resultSet->GetResultString(m_attributeNumber->GetSQLFieldName());
			wxDateTime sDate = resultSet->GetResultDate(m_attributeDate->GetSQLFieldName());

			sDescription << m_sName << " " << sNumber << " (" << sDate.Format("%d.%m.%Y %H:%M:%S") << ")";
		}

		resultSet->Close();
	}

	return sDescription; 
}

wxString CMetaObjectDocumentValue::GetDescription(const std::map<wxString, CValue> &valData)
{
	CValue vDate = valData.find(m_attributeDate->GetSQLFieldName())->second;
	CValue vNumber = valData.find(m_attributeNumber->GetSQLFieldName())->second;

	wxString sDesctiption;
	sDesctiption << m_sName << " " << vNumber.GetString() << " (" << vDate.GetString() << ")";
	return sDesctiption;
}

wxString CMetaObjectDocumentValue::GetDescription(const std::map<wxString, CValue> &valData, const Guid &objGuid)
{
	CValue vDate = valData.find(m_attributeDate->GetSQLFieldName())->second;
	CValue vNumber = valData.find(m_attributeNumber->GetSQLFieldName())->second;

	wxString sDesctiption;
	sDesctiption << m_sName << " " << vNumber.GetString() << " (" << vDate.GetString() << ")";
	return sDesctiption;
}