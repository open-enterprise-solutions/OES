////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : metamodule object
////////////////////////////////////////////////////////////////////////////

#include "metaModuleObject.h"
#include "database/databaseLayer.h"
#include "metadata/objects/baseObject.h"
#include "appData.h"

//***********************************************************************
//*                           ModuleObject                              *
//***********************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CMetaModuleObject, IMetaObject)
wxIMPLEMENT_DYNAMIC_CLASS(CMetaCommonModuleObject, CMetaModuleObject)

//***********************************************************************
//*                           Metamodule                                *
//***********************************************************************

CMetaModuleObject::CMetaModuleObject(const wxString &name, const wxString &alias, const wxString &comment) : IMetaObject(name, alias, comment)
{
}

bool CMetaModuleObject::LoadData(CMemoryReader &reader)
{
	reader.r_stringZ(m_sModule);
	return true;
}

bool CMetaModuleObject::SaveData(CMemoryWriter &writer)
{
	writer.w_stringZ(m_sModule);
	return true;
}

//***********************************************************************
//*                           System metadata                           *
//***********************************************************************

#include "compiler/debugger/debugClient.h"
#include "compiler/debugger/debugServer.h"
#include "metadata/metadata.h"

bool CMetaModuleObject::OnCreateMetaObject()
{
	//initialize debugger server
	unsigned int nNumber = 1 + m_sModule.Replace('\n', '\n');

	if (appData->IsDesignerMode()) {
		debugServer->InitializeBreakpoints(GetDocPath(), 0, nNumber);
	}
	else {
		debugClient->InitializeBreakpoints(GetDocPath(), 0, nNumber);
	}

	return true;
}

bool CMetaModuleObject::OnLoadMetaObject()
{
	//initialize debugger server
	unsigned int nNumber = 1 + m_sModule.Replace('\n', '\n');
	if (appData->IsDesignerMode()) {
		debugServer->InitializeBreakpoints(GetDocPath(), 0, nNumber);
	}
	else {
		debugClient->InitializeBreakpoints(GetDocPath(), 0, nNumber);
	}

	return true;
}

bool CMetaModuleObject::OnSaveMetaObject()
{
	//initialize debugger server
	if (appData->IsDesignerMode()) {
		return debugServer->SaveBreakpoints(GetDocPath());
	}

	return true;
}

bool CMetaModuleObject::OnDeleteMetaObject()
{
	return true;
}

bool CMetaModuleObject::OnRunMetaObject()
{
	return true;
}

bool CMetaModuleObject::OnCloseMetaObject()
{
	return true;
}

//***********************************************************************
//*                           Metamodule                                *
//***********************************************************************

CMetaCommonModuleObject::CMetaCommonModuleObject(const wxString &name, const wxString &alias, const wxString &comment) : CMetaModuleObject(name, alias, comment)
{
}

//***********************************************************************
//*                          common value object                        *
//***********************************************************************

bool CMetaCommonModuleObject::OnCreateMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!moduleManager->AddCommonModuleAndRun(this))
		return false;

	return CMetaModuleObject::OnCreateMetaObject();
}

bool CMetaCommonModuleObject::OnLoadMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!moduleManager->FindCommonModule(this))
	{
		if (!moduleManager->AddCommonModule(this))
			return false;

		return CMetaModuleObject::OnLoadMetaObject();
	}

	return CMetaModuleObject::OnLoadMetaObject();
}

bool CMetaCommonModuleObject::OnSaveMetaObject()
{
	return CMetaModuleObject::OnSaveMetaObject();
}

bool CMetaCommonModuleObject::OnDeleteMetaObject()
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!moduleManager->RemoveCommonModule(this))
		return false;

	return CMetaModuleObject::OnDeleteMetaObject();
}

bool CMetaCommonModuleObject::OnRenameMetaObject(const wxString &sNewName)
{
	IModuleManager *moduleManager = m_metaData->GetModuleManager();
	wxASSERT(moduleManager);

	if (!moduleManager->RenameCommonModule(this, sNewName))
		return false;

	return CMetaModuleObject::OnRenameMetaObject(sNewName);
}

bool CMetaCommonModuleObject::OnRunMetaObject()
{
	return true;
}

bool CMetaCommonModuleObject::OnCloseMetaObject()
{
	return true;
}

//***********************************************************************
//*                       Register in runtime                           *
//***********************************************************************

METADATA_REGISTER(CMetaCommonModuleObject, "metaCommonModule", g_metaCommonModuleCLSID);
METADATA_REGISTER(CMetaModuleObject, "metaModule", g_metaModuleCLSID);
