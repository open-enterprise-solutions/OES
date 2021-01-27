////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : base manager for catalogs, docs etc..  
////////////////////////////////////////////////////////////////////////////

#include "baseManager.h"
#include "compiler/methods.h"
#include "compiler/valueMap.h"

#include "metadata/metadata.h"
#include "metadata/metaObjects/metaObject.h"

wxIMPLEMENT_DYNAMIC_CLASS(CManagerBase, CValue);

CManagerBase::CManagerBase() : CValue(eValueTypes::TYPE_VALUE, true), m_methods(new CMethods()), m_metaData(NULL) {}

CManagerBase::CManagerBase(IMetadata *metaData) : CValue(eValueTypes::TYPE_VALUE, true), m_methods(new CMethods()), m_metaData(metaData) {}

CManagerBase::~CManagerBase() { wxDELETE(m_methods); }

enum
{
	enConstants = 0,
	enCatalogs,
	enDocuments,
	enEnumerations,
	enDataProcessors
};

void CManagerBase::PrepareNames() const
{
	SEng aAttributes[] =
	{
		{"constants","constants"},
		{"catalogs","catalogs"},
		{"documents","documents"},
		{"enumerations","enumerations"},
		{"dataProcessors","dataProcessors"},
	};

	int nCountA = sizeof(aAttributes) / sizeof(aAttributes[0]);
	m_methods->PrepareAttributes(aAttributes, nCountA);
}

#include "catalogManager.h"
#include "documentManager.h"
#include "enumerationManager.h"
#include "dataProcessorManager.h"

#include "metadata/objects/constantsManager.h"

CValue CManagerBase::GetAttribute(CAttributeParameters &aParams)
{
	switch (aParams.GetAttributeIndex())
	{
	case enConstants:
	{
		std::map<wxString, CValue> constMananager;

		for (auto obj : m_metaData->GetMetaObjects(g_metaConstantCLSID))
		{
			constMananager.insert_or_assign(obj->GetName(), new CManagerConstants(obj));
		}

		return new CValueStructure(constMananager);
	}
	case enCatalogs:
	{
		std::map<wxString, CValue> catMananager;

		for (auto obj : m_metaData->GetMetaObjects(g_metaCatalogCLSID))
		{
			catMananager.insert_or_assign(obj->GetName(), new CManagerCatalog(obj));
		}

		return new CValueStructure(catMananager);
	}
	case enDocuments:
	{
		std::map<wxString, CValue> docMananager;

		for (auto obj : m_metaData->GetMetaObjects(g_metaDocumentCLSID))
		{
			docMananager.insert_or_assign(obj->GetName(), new CManagerDocument(obj));
		}

		return new CValueStructure(docMananager);

	}
	case enEnumerations:
	{
		std::map<wxString, CValue> enumMananager;

		for (auto obj : m_metaData->GetMetaObjects(g_metaEnumerationCLSID))
		{
			enumMananager.insert_or_assign(obj->GetName(), new CManagerEnumeration(obj));
		}

		return new CValueStructure(enumMananager);
	}
	case enDataProcessors:
	{
		std::map<wxString, CValue> dataProcessoMananagerr;

		for (auto obj : m_metaData->GetMetaObjects(g_metaDataProcessorCLSID))
		{
			dataProcessoMananagerr.insert_or_assign(obj->GetName(), new CManagerDataProcessor(obj));
		}

		return new CValueStructure(dataProcessoMananagerr);
	}
	}

	return CValue();
}