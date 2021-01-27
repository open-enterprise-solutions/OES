////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : catalog manager
////////////////////////////////////////////////////////////////////////////

#include "catalogManager.h"
#include "metadata/metadata.h"
#include "compiler/methods.h"
#include "baseObject.h"

wxIMPLEMENT_DYNAMIC_CLASS(CManagerCatalog, CValue);

CMetaCommonModuleObject *CManagerCatalog::GetModuleManager() { return m_metaObject->GetModuleManager(); }

CManagerCatalog::CManagerCatalog(IMetaObject *metaObject) : CValue(eValueTypes::TYPE_VALUE, true),
m_methods(new CMethods())
{
	m_metaObject = dynamic_cast<CMetaObjectCatalogValue *>(metaObject);
}

CManagerCatalog::~CManagerCatalog()
{
	wxDELETE(m_methods);
}

#include "reference/reference.h"

CValue CManagerCatalog::EmptyRef()
{
	return new CValueReference(m_metaObject);
}

wxString CManagerCatalog::GetTypeString() const
{
	return wxT("catalogManager.") + m_metaObject->GetName();
}

wxString CManagerCatalog::GetString() const
{
	return wxT("catalogManager.") + m_metaObject->GetName();
}