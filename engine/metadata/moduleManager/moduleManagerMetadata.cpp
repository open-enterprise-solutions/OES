////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : meta-access 
////////////////////////////////////////////////////////////////////////////

#include "moduleManager.h"
#include "metadata/metadata.h"
#include "compiler/methods.h"
#include "compiler/valueMap.h"

wxIMPLEMENT_DYNAMIC_CLASS(IModuleManager::CMetadataValue, CValue);

IModuleManager::CMetadataValue::CMetadataValue(IMetadata *metaData) :
	CValue(eValueTypes::TYPE_VALUE, true),
	m_methods(new CMethods()), m_metaData(metaData)
{
}

IModuleManager::CMetadataValue::~CMetadataValue()
{
	wxDELETE(m_methods);
}

enum
{
	enCommonModules = 0,
	enCommonForms,
	enCommonTemplates,
	enConstants,
	enCatalogs,
	enDocuments,
	enEnumerations,
	enDataProcessors,
};

void IModuleManager::CMetadataValue::PrepareNames() const
{
	SEng aAttributes[] = {
			{"commonModules","commonModules"},
			{"commonForms","commonForms"},
			{"commonTemplates","commonTemplates"},
			{"constants","constants"},
			{"catalogs","catalogs"},
			{"documents","documents"},
			{"enumerations","enumerations"},
			{"dataProcessors","dataProcessors"},
	};

	int nCountA = sizeof(aAttributes) / sizeof(aAttributes[0]);
	m_methods->PrepareAttributes(aAttributes, nCountA);
}

CValue IModuleManager::CMetadataValue::Method(CMethodParameters &aParams)
{
	return CValue();
}

//****************************************************************************
//*                              Override attribute                          *
//****************************************************************************

void IModuleManager::CMetadataValue::SetAttribute(CAttributeParameters &aParams, CValue &cVal)
{
}

CValue IModuleManager::CMetadataValue::GetAttribute(CAttributeParameters &aParams)//�������� ��������
{
	std::map<wxString, CValue> m_aMetaObjects;

	switch (aParams.GetAttributeIndex())
	{
	case enCommonModules:
	{
		for (auto obj : m_metaData->GetMetaObjects(g_metaCommonModuleCLSID))
		{
			m_aMetaObjects.insert_or_assign(obj->GetName(), obj);
		}
	} break;
	case enCommonForms:
	{
		for (auto obj : m_metaData->GetMetaObjects(g_metaCommonFormCLSID))
		{
			m_aMetaObjects.insert_or_assign(obj->GetName(), obj);
		}
	} break;
	case enCommonTemplates:
	{
		for (auto obj : m_metaData->GetMetaObjects(g_metaTemplateCLSID))
		{
			m_aMetaObjects.insert_or_assign(obj->GetName(), obj);
		}
	} break;
	case enConstants:
	{
		for (auto obj : m_metaData->GetMetaObjects(g_metaConstantCLSID))
		{
			m_aMetaObjects.insert_or_assign(obj->GetName(), obj);
		}
	} break;
	case enCatalogs:
	{
		for (auto obj : m_metaData->GetMetaObjects(g_metaCatalogCLSID))
		{
			m_aMetaObjects.insert_or_assign(obj->GetName(), obj);
		}
	} break;
	case enDocuments:
	{
		for (auto obj : m_metaData->GetMetaObjects(g_metaDocumentCLSID))
		{
			m_aMetaObjects.insert_or_assign(obj->GetName(), obj);
		}
	} break;
	case enEnumerations:
	{
		for (auto obj : m_metaData->GetMetaObjects(g_metaEnumerationCLSID))
		{
			m_aMetaObjects.insert_or_assign(obj->GetName(), obj);
		}
	} break;
	case enDataProcessors:
	{
		for (auto obj : m_metaData->GetMetaObjects(g_metaDataProcessorCLSID))
		{
			m_aMetaObjects.insert_or_assign(obj->GetName(), obj);
		}
	} break;
	}

	return new CValueStructure(m_aMetaObjects);
}