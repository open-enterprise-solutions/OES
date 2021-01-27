////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : document manager
////////////////////////////////////////////////////////////////////////////

#include "documentManager.h"
#include "metadata/metadata.h"
#include "compiler/methods.h"
#include "baseObject.h"

wxIMPLEMENT_DYNAMIC_CLASS(CManagerDocument, CValue);

CMetaCommonModuleObject *CManagerDocument::GetModuleManager() { return m_metaObject->GetModuleManager(); }

CManagerDocument::CManagerDocument(IMetaObject *metaObject) : CValue(eValueTypes::TYPE_VALUE, true),
m_methods(new CMethods())
{
	m_metaObject = dynamic_cast<CMetaObjectDocumentValue *>(metaObject);
}

CManagerDocument::~CManagerDocument()
{
	wxDELETE(m_methods);
}

#include "reference/reference.h"

CValue CManagerDocument::EmptyRef()
{
	return new CValueReference(m_metaObject);
}

wxString CManagerDocument::GetTypeString() const
{
	return wxT("documentManager.") + m_metaObject->GetName();
}

wxString CManagerDocument::GetString() const
{
	return wxT("documentManager.") + m_metaObject->GetName();
}