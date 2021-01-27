////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : constants manager
////////////////////////////////////////////////////////////////////////////

#include "constantsManager.h"

wxIMPLEMENT_DYNAMIC_CLASS(CManagerConstants, CValue);

CManagerConstants::CManagerConstants(IMetaObject *metaConst) : CValue(eValueTypes::TYPE_VALUE, true)
{
	m_metaConst = dynamic_cast<CConstantObject *>(metaConst);
}

wxString CManagerConstants::GetTypeString() const
{
	return wxT("constmanager.") + m_metaConst->GetName();
}

wxString CManagerConstants::GetString() const
{
	return wxT("constmanager.") + m_metaConst->GetName();
}

CManagerConstants::~CManagerConstants()
{
}