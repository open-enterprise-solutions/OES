////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : constants
////////////////////////////////////////////////////////////////////////////

#include "constants.h"
#include "metadata/metadata.h"

wxIMPLEMENT_DYNAMIC_CLASS(CConstantObject, CAttributeObject)

//***********************************************************************
//*                         Attributes                                  * 
//***********************************************************************

CConstantObject::CConstantObject() : CAttributeObject() {}

bool CConstantObject::LoadData(CMemoryReader &reader)
{
	if (!ReadConstInDB())
		return false;

	return CAttributeObject::LoadData(reader);
}

bool CConstantObject::SaveData(CMemoryWriter &writer)
{
	if (!SaveConstInDB())
		return false;

	return CAttributeObject::SaveData(writer);
}

bool CConstantObject::DeleteData()
{
	if (!DeleteConstInDB())
		return false;

	return CAttributeObject::DeleteData();
}

//***********************************************************************
//*                          Read&save property                         *
//***********************************************************************

void CConstantObject::ReadProperty()
{
	CAttributeObject::ReadProperty();
}

void CConstantObject::SaveProperty()
{
	CAttributeObject::SaveProperty();
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CConstantObject, "metaConstant", g_metaConstantCLSID);