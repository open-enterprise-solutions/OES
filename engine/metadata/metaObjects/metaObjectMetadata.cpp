////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : metaobject common metadata
////////////////////////////////////////////////////////////////////////////

#include "metaObjectMetadata.h"
#include "metaModuleObject.h"

#define defaultMetaID 1000
#define initModuleName wxT("initModule")

//***********************************************************************
//*                         metadata                                    * 
//***********************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CMetaObject, IMetaObject)

//*****************************************************************************************
//*                                  MetadataObject                                       *
//*****************************************************************************************

CMetaObject::CMetaObject() : IMetaObject(st_configuration)
{
	m_metaID = defaultMetaID;

	m_commonModule = new CMetaModuleObject(initModuleName);
	m_commonModule->SetClsid(g_metaModuleCLSID);

	//set child/parent
	m_commonModule->SetParent(this);
	AddChild(m_commonModule);
}

CMetaObject::~CMetaObject()
{
	wxDELETE(m_commonModule);
}

bool CMetaObject::LoadData(CMemoryReader &dataReader)
{
	return m_commonModule->LoadMeta(dataReader);
}

bool CMetaObject::SaveData(CMemoryWriter &dataWritter)
{
	return m_commonModule->SaveMeta(dataWritter);
}

//***********************************************************************
//*                          common value object                        *
//***********************************************************************

#include "metadata/metadata.h"

bool CMetaObject::OnCreateMetaObject()
{
	m_commonModule->GenerateGuid();
	return true;
}

bool CMetaObject::OnLoadMetaObject()
{
	CValue *dataRef = NULL;

	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!moduleManager->FindCompileModule(m_commonModule, dataRef))
	{
		m_commonModule->SetMetadata(m_metaData);

		if (!moduleManager->AddCompileModule(m_commonModule, moduleManager))
			return false;
	}

	m_commonModule->SetFullPath(m_sFullPath);
	m_metaData->AppendSecondaryMetadata(m_commonModule);
	m_commonModule->SetMetaID(m_metaData->GenerateNewID());

	return m_commonModule->OnLoadMetaObject();
}

bool CMetaObject::OnSaveMetaObject()
{
	return m_commonModule->OnSaveMetaObject();
}

bool CMetaObject::OnDeleteMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!moduleManager->RemoveCompileModule(m_commonModule))
		return false;

	if (!m_commonModule->OnDeleteMetaObject())
		return false;

	m_metaData->RemoveSecondaryMetadata(m_commonModule);
	return true;
}

bool CMetaObject::OnRunMetaObject()
{
	return m_commonModule->OnRunMetaObject();
}

bool CMetaObject::OnCloseMetaObject()
{
	return m_commonModule->OnCloseMetaObject();
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CMetaObject, "metaCommonMetadata", g_metaCommonMetadataCLSID);
