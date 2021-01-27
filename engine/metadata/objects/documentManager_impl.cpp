////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : document manager
////////////////////////////////////////////////////////////////////////////

#include "documentManager.h"
#include "appData.h"
#include "database/databaseLayer.h"

CValue CManagerDocument::FindByNumber(CValue vCode)
{
	wxString sCode = vCode.GetString();
	sCode.Trim(true);
	sCode.Trim(false);

	if (appData->IsEnterpriseMode())
	{
		wxString sTableName = m_metaObject->GetSpecialTableNameDB();

		if (objectDatabase->TableExists(sTableName))
		{
			wxString sql = "SELECT UUID FROM " + sTableName + " WHERE number_t = '" + sCode + "' LIMIT 1";

			wxString csUUID = wxEmptyString;

			DatabaseResultSet *databaseResultSet = objectDatabase->RunQueryWithResults(sql);
			while (databaseResultSet->Next())
			{
				csUUID = databaseResultSet->GetResultString(GUID_NAME);
			}
			databaseResultSet->Close();

			if (!csUUID.IsEmpty())
			{
				return new CValueReference(m_metaObject, csUUID);
			}
		}
	}

	return new CValueReference(m_metaObject);
}
