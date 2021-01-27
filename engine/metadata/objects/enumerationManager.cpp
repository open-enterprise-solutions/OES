////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : enumeration manager
////////////////////////////////////////////////////////////////////////////

#include "enumerationManager.h"
#include "metadata/metadata.h"
#include "compiler/methods.h"
#include "baseObject.h"

wxIMPLEMENT_DYNAMIC_CLASS(CManagerEnumeration, CValue);

CMetaCommonModuleObject *CManagerEnumeration::GetModuleManager() { return m_metaObject->GetModuleManager(); }

CManagerEnumeration::CManagerEnumeration(IMetaObject *metaObject) : CValue(eValueTypes::TYPE_VALUE, true),
m_methods(new CMethods())
{
	m_metaObject = dynamic_cast<CMetaObjectEnumerationValue *>(metaObject);
	wxASSERT(m_metaObject);
}

CManagerEnumeration::~CManagerEnumeration()
{
	wxDELETE(m_methods);
}

wxString CManagerEnumeration::GetTypeString() const
{
	return wxT("enumerationManager.") + m_metaObject->GetName();
}

wxString CManagerEnumeration::GetString() const
{
	return wxT("enumerationManager.") + m_metaObject->GetName();
}