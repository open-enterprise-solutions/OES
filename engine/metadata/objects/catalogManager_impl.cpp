////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : catalog manager
////////////////////////////////////////////////////////////////////////////

#include "catalogManager.h"
#include "appData.h"
#include "database/databaseLayer.h"

CValue CManagerCatalog::FindByCode(CValue vCode)
{
	if (appData->IsEnterpriseMode())
	{
		wxString sTableName = m_metaObject->GetSpecialTableNameDB();

		if (objectDatabase->TableExists(sTableName))
		{
			wxString sUUID = wxEmptyString;

			wxString sql = "SELECT FIRST 1 UUID FROM " + sTableName + " WHERE CODE = '" + vCode.GetString() + "'";
			DatabaseResultSet *databaseResultSet = objectDatabase->RunQueryWithResults(sql);

			if (!databaseResultSet) return new CValueReference(m_metaObject);
			if (databaseResultSet->Next()) sUUID = databaseResultSet->GetResultString(GUID_NAME);
			databaseResultSet->Close();

			if (!sUUID.IsEmpty()) return new CValueReference(m_metaObject, sUUID);
		}
	}

	return new CValueReference(m_metaObject);
}

CValue CManagerCatalog::FindByName(CValue vName)
{
	if (appData->IsEnterpriseMode())
	{
		wxString sTableName = m_metaObject->GetSpecialTableNameDB();

		if (objectDatabase->TableExists(sTableName))
		{
			wxString sUUID = wxEmptyString;

			wxString sql = "SELECT FIRST 1 UUID FROM " + sTableName + " WHERE NAME = '" + vName.GetString() + "'";
			DatabaseResultSet *databaseResultSet = objectDatabase->RunQueryWithResults(sql);

			if (!databaseResultSet) return new CValueReference(m_metaObject);
			if (databaseResultSet->Next()) sUUID = databaseResultSet->GetResultString(GUID_NAME);
			databaseResultSet->Close();

			if (!sUUID.IsEmpty()) return new CValueReference(m_metaObject, sUUID);
		}
	}

	return new CValueReference(m_metaObject);
}