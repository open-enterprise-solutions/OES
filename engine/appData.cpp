////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxFormBuider
//	Description : app info
////////////////////////////////////////////////////////////////////////////

#include "appData.h"

//sandbox
#include "metadata/moduleManager/moduleManager.h"
#include "metadata/metadata.h"

//databases
#include "database/odbc/odbcDatabaseLayer.h"
#include "database/postgres/postgresDatabaseLayer.h"
#include "database/firebird/firebirdDatabaseLayer.h"

//mainFrame
#include "window/mainFrame.h"

//utils 
#include "utils/bitmaps.h"
#include "utils/events.h"
#include "common/objectbase.h"
#include "utils/stringutils.h"
#include "utils/typeconv.h"

///////////////////////////////////////////////////////////////////////////////
// ApplicationData
///////////////////////////////////////////////////////////////////////////////

ApplicationData* ApplicationData::s_instance = NULL;

ApplicationData* ApplicationData::Get(const wxString &rootdir) { if (!s_instance) s_instance = new ApplicationData(rootdir); return s_instance; }

void ApplicationData::Destroy()
{
	metadataDestroy();

	if (s_instance)
	{
		//Close connection and delete ptr 
		wxDELETE(s_instance->m_objDb);
		wxDELETE(s_instance);
	}
}

bool ApplicationData::Initialize(eRunMode modeRun, const wxString &user, const wxString &password)
{
	ApplicationData* m_appData = ApplicationData::Get();
	return m_appData->LoadApp(modeRun);
}

#include <wx/fs_arc.h>
#include <wx/fs_filter.h>
#include <wx/fs_mem.h>

ApplicationData::ApplicationData(const wxString &projectDir) :
	m_projectDir(projectDir), m_objDb(new FirebirdDatabaseLayer())
{
	//start new connection 
	if (m_objDb->Open(projectDir + wxT("\\") + wxT("sys.database")))
	{
		//create new session 
		m_sessionGuid = Guid::newGuid();

		// Support loading files from memory
		// Used to load the XRC preview, but could be useful elsewhere
		wxFileSystem::AddHandler(new wxMemoryFSHandler);

		// Support for loading files from archives
		wxFileSystem::AddHandler(new wxArchiveFSHandler);
		wxFileSystem::AddHandler(new wxFilterFSHandler);
	}

}

ApplicationData::~ApplicationData()
{
	//Close connection 
	wxDELETE(m_objDb);
}

bool ApplicationData::LoadApp(eRunMode runMode)
{
	m_runMode = runMode;

	if (IsDesignerMode() || IsEnterpriseMode())
	{
		mainFrameInit();

		if (!metadata->LoadMetadata())
		{
			mainFrameDestroy();
			return false;
		}

		mainFrame->SetFocus();     // focus on my window
		mainFrame->Raise();
		mainFrame->Maximize();

		mainFrame->Show(true);    // show the window	
	}

	m_projectDir = wxGetCwd();
	return true;
}

bool ApplicationData::SaveProject()
{
	return metadata->SaveMetadata();
}