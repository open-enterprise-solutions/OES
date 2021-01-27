////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : dataProcessor - manager
////////////////////////////////////////////////////////////////////////////

#include "dataProcessorManager.h"
#include "metadata/metadata.h"
#include "compiler/methods.h"
#include "baseObject.h"

wxIMPLEMENT_DYNAMIC_CLASS(CManagerDataProcessor, CValue);

CMetaCommonModuleObject *CManagerDataProcessor::GetModuleManager() { return m_metaObject->GetModuleManager(); }

CManagerDataProcessor::CManagerDataProcessor(IMetaObject *metaObject) : CValue(eValueTypes::TYPE_VALUE, true),
m_methods(new CMethods())
{
	m_metaObject = dynamic_cast<CMetaObjectDataProcessorValue *>(metaObject);
}

CManagerDataProcessor::~CManagerDataProcessor()
{
	wxDELETE(m_methods);
}

wxString CManagerDataProcessor::GetTypeString() const
{
	return wxT("dataProcessorManager.") + m_metaObject->GetName();
}

wxString CManagerDataProcessor::GetString() const
{
	return wxT("dataProcessorManager.") + m_metaObject->GetName();
}