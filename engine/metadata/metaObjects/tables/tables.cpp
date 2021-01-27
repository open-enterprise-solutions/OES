////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : meta-tables
////////////////////////////////////////////////////////////////////////////

#include "tables.h"
#include "metadata/metadata.h"

wxIMPLEMENT_DYNAMIC_CLASS(CTableObject, IMetaObject)

//***********************************************************************
//*                         Attributes                                  * 
//***********************************************************************

CTableObject::CTableObject() : IMetaObject() {}

std::vector<CAttributeObject *> CTableObject::GetObjectAttributes()
{
	std::vector<CAttributeObject *> m_aAttributes;

	for (auto metaObj : m_metaObjects)
	{
		if (metaObj->GetClsid() == g_metaAttributeCLSID)
			m_aAttributes.push_back(dynamic_cast<CAttributeObject *>(metaObj));
	}

	return m_aAttributes;
}

bool CTableObject::LoadData(CMemoryReader &reader)
{
	return true;
}

bool CTableObject::SaveData(CMemoryWriter &writer)
{
	return true;
}

//***********************************************************************
//*                          Read&save property                         *
//***********************************************************************

void CTableObject::ReadProperty()
{
	IMetaObject::ReadProperty();
}

void CTableObject::SaveProperty()
{
	IMetaObject::SaveProperty();
}

//***********************************************************************
//*                           System metadata                           *
//***********************************************************************

wxArrayString CTableObject::GetAttributes()
{
	wxArrayString m_attributes;

	for (auto attribute : m_metaObjects)
		m_attributes.Add(attribute->GetName());

	return m_attributes;
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CTableObject, "tabularSection", g_metaTableCLSID);