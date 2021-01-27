////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : enumeration metadata - db 
////////////////////////////////////////////////////////////////////////////

#include "enumeration.h"
#include "appData.h"
#include "database/databaseLayer.h"
#include "database/databaseErrorCodes.h"
#include "utils/stringutils.h"

#define TABLE_NAME wxT("enum_")

//**********************************************************************************************************
//*                                          Common functions                                              *
//**********************************************************************************************************

wxString CMetaObjectEnumerationValue::GetSpecialTableNameSpaceDB()
{
	return TABLE_NAME;
}

//**********************************************************************************************************
//*                                          Metaobject in DB                                              *
//**********************************************************************************************************

#include "metadata/objects/reference/reference.h"

bool CMetaObjectEnumerationValue::CreateMetaObjectInDB()
{
	wxString sTableName = GetSpecialTableNameDB(); std::vector<Guid> aCreatedEnumeration;

	if (!objectDatabase->TableExists(sTableName))
	{
		int retCode = 0;

		retCode = objectDatabase->RunQuery("CREATE TABLE " + sTableName + " ("
			"UUID             VARCHAR(36)          NOT NULL PRIMARY KEY,"
			"UUIDREF          BLOB,"
			"METAID           INTEGER);");

		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;

		retCode = objectDatabase->RunQuery("CREATE INDEX " + sTableName + "_INDEX ON " + sTableName + " ("
			"UUID,"
			"METAID);");

		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;
	}
	else
	{
		DatabaseResultSet *databaseResultSet = objectDatabase->RunQueryWithResults("SELECT * FROM " + sTableName);

		if (!databaseResultSet)
			return false;

		while (databaseResultSet->Next())
		{
			Guid metaGuid = databaseResultSet->GetResultString("UUID");
			aCreatedEnumeration.push_back(metaGuid);
		}

		databaseResultSet->Close();
	}

	PreparedStatement *prepareStatement = objectDatabase->PrepareStatement("UPDATE OR INSERT INTO " + sTableName + " (UUID, METAID, UUIDREF) VALUES (?, ?, ?) MATCHING(METAID);");

	if (!prepareStatement)
		return false;

	for (auto enumObj : GetObjectEnums())
	{
		auto itFounded = std::find(aCreatedEnumeration.begin(), aCreatedEnumeration.end(), enumObj->GetGuid());

		if (itFounded != aCreatedEnumeration.end()) {
			aCreatedEnumeration.erase(itFounded);
			continue;
		}

		CValueReference *reference = new CValueReference(this, enumObj->GetGuid());
		if (!reference)
			return false;

		prepareStatement->SetParamString(1, enumObj->GetGuid().str());
		prepareStatement->SetParamInt(2, enumObj->GetMetaID());

		reference->SetBinaryData(3, prepareStatement);

		bool hasError = prepareStatement->RunQuery() == DATABASE_LAYER_QUERY_RESULT_ERROR;
		
		delete reference;
		
		if (hasError)
			return false;
	}

	prepareStatement->Close();

	for (auto metaGuid : aCreatedEnumeration) {
		int retCode = objectDatabase->RunQuery("DELETE FROM " + sTableName + " WHERE UUID = '" + metaGuid.str() + "' ;");
		if (retCode == DATABASE_LAYER_QUERY_RESULT_ERROR)
			return false;
	}

	return true;
}

bool CMetaObjectEnumerationValue::RemoveMetaObjectInDB()
{
	wxString sTableName = GetSpecialTableNameDB();
	int retCode = objectDatabase->RunQuery("DROP TABLE " + sTableName + ";");
	return retCode != DATABASE_LAYER_QUERY_RESULT_ERROR;
}

wxString CMetaObjectEnumerationValue::GetDescription(const Guid &guid)
{
	if (!guid.isValid()) return wxEmptyString;

	for (auto eEnum : GetObjectEnums())
	{
		if (guid == eEnum->GetGuid())
			return eEnum->GetName();
	}

	return wxT("<error>");
}

wxString CMetaObjectEnumerationValue::GetDescription(const std::map<wxString, CValue> &valData)
{
	return wxT("<error>");
}

wxString CMetaObjectEnumerationValue::GetDescription(const std::map<wxString, CValue> &valData, const Guid &objGuid)
{
	return GetDescription(objGuid);
}