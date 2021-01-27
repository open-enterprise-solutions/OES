////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : meta-enumerations
////////////////////////////////////////////////////////////////////////////

#include "enumeration.h"
#include "appData.h"
#include "metadata/objects/baseObject.h"
#include "metadata/metadata.h"
#include "database/databaseLayer.h"

wxIMPLEMENT_DYNAMIC_CLASS(CEnumerationObject, IMetaObject)

bool CEnumerationObject::LoadData(CMemoryReader &reader)
{
	/*IMetaObjectValue *metaObject = wxStaticCast(GetParent(), IMetaObjectValue);

	if (metaObject)
	{
		if (objectDatabase->TableExists(metaObject->GetSpecialTableNameDB()))
		{
			auto result = objectDatabase->RunQueryWithResults("SELECT UUID FROM " + metaObject->GetSpecialTableNameDB() + " WHERE METAID = '" << GetMetaID() << "'");
			if (result->Next()) m_guid = result->GetResultString(GUID_NAME);
		}

		return false;
	}*/

	return true;
}

bool CEnumerationObject::SaveData(CMemoryWriter &writer)
{
	return true;
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CEnumerationObject, "metaEnum", g_metaEnumCLSID);