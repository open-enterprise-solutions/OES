////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : main app
////////////////////////////////////////////////////////////////////////////

#include "mainApp.h"

#include "database/databaseLayer.h"
#include "window/authorization.h"

#include <wx/apptrait.h>
#include <wx/clipbrd.h>
#include <wx/cmdline.h>
#include <wx/config.h>
#include <wx/docview.h>
#include <wx/debugrpt.h>
#include <wx/evtloop.h>
#include <wx/fs_arc.h>
#include <wx/fs_filter.h>
#include <wx/fs_mem.h>
#include <wx/splash.h>
#include <wx/stdpaths.h>
#include <wx/sysopt.h>

#include "resources/splashLogo.xpm"

#if wxVERSION_NUMBER >= 2905 && wxVERSION_NUMBER <= 3100
#include <wx/xrc/xh_auinotbk.h>
#elif wxVERSION_NUMBER > 3100
#include <wx/xrc/xh_aui.h>
#endif

bool CDesignerApp::OnInit()
{
	wxDateTime::SetCountry(wxDateTime::Country::USA);

	m_locale.AddCatalogLookupPathPrefix(_T("lang"));
	m_locale.AddCatalog(m_locale.GetCanonicalName());

	ComputeBuildID();

	return m_locale.Init(wxLANGUAGE_ENGLISH);
}

static const wxCmdLineEntryDesc s_cmdLineDesc[] = {
	{ wxCMD_LINE_OPTION, "m", "mode", "Run mode for this application. ", wxCMD_LINE_VAL_STRING, NULL },
	{ wxCMD_LINE_SWITCH, "d", "debug", "Enable debugging for enterprise application.", wxCMD_LINE_VAL_NONE, NULL },

	//embedded mode 
	{ wxCMD_LINE_OPTION, "embedded", "embedded", "Start enterprise in embedded mode", wxCMD_LINE_VAL_NONE, NULL },
	{ wxCMD_LINE_OPTION, "ib", "ib", "Start enterprise from current path", wxCMD_LINE_VAL_STRING, NULL },

	//server mode 
	{ wxCMD_LINE_OPTION, "srv", "srv", "Start enterprise using server address", wxCMD_LINE_VAL_STRING, NULL },
	{ wxCMD_LINE_OPTION, "port", "port", "Start enterprise using port", wxCMD_LINE_VAL_STRING, NULL },

	{ wxCMD_LINE_OPTION, "l", "l", "Start enterprise from current login", wxCMD_LINE_VAL_STRING, NULL },
	{ wxCMD_LINE_OPTION, "p", "p", "Start enterprise from current password", wxCMD_LINE_VAL_STRING, NULL },

	{ wxCMD_LINE_SWITCH, "h", "help", "Show this help message.", wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_PARAM, NULL, NULL, "File to open.", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_NONE, NULL, NULL, NULL, wxCMD_LINE_VAL_NONE, 0 }
};

wxIMPLEMENT_APP(CDesignerApp);

int CDesignerApp::OnRun()
{
	// Abnormal Termination Handling
#if wxUSE_ON_FATAL_EXCEPTION && wxUSE_STACKWALKER
	::wxHandleFatalExceptions(true);
#elif defined(_WIN32) && defined(__MINGW32__)
	// Structured Exception handlers are stored in a linked list at FS:[0]
	// THIS MUST BE A LOCAL VARIABLE - windows won't use an object outside of the thread's stack valueFrame
	EXCEPTION_REGISTRATION ex;
	ex.handler = StructuredExceptionHandler;
	asm volatile ("movl %%fs:0, %0" : "=r" (ex.prev));
	asm volatile ("movl %0, %%fs:0" : : "r" (&ex));
#endif

	// Parse command line
	wxCmdLineParser parser(s_cmdLineDesc, argc, argv);
	if (parser.Parse() != 0) { return 1; }

	wxString mode;
	parser.Found(wxT("m"), &mode);

	bool debugEnable = parser.Found(wxT("d"));

	if (debugEnable) wxSystemOptions::SetOption("debug.enable", "true");
	else wxSystemOptions::SetOption("debug.enable", "false");

	eRunMode m_modeRun = eRunMode::DESIGNER_MODE;
	if (mode == wxT("enterprise")) m_modeRun = eRunMode::ENTERPRISE_MODE;

	// Get the data directory
	wxStandardPaths& stdPaths = wxStandardPaths::Get();
	wxString dataDir = stdPaths.GetDataDir();
	
	bool embeddedMode = parser.Found(wxT("embedded"));

	wxString pathIB, serverIB, portIB, userIB, passwordIB;

	if (parser.Found("ib", &pathIB)) {
		dataDir = pathIB;
	}

	parser.Found("srv", &serverIB);
	parser.Found("port", &portIB);

	parser.Found("l", &userIB);
	parser.Found("p", &passwordIB);

	wxSplashScreen* m_splashWnd = new wxSplashScreen(wxBitmap(splashLogo_xpm, wxBITMAP_TYPE_XPM),
		wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT,
		6000, NULL, -1, wxDefaultPosition, wxDefaultSize,
		wxBORDER_SIMPLE);

	m_splashWnd->Centre();
	m_splashWnd->Show(true);

	wxApp::SetTopWindow(m_splashWnd);

	//Needed to get the splashscreen to paint
	m_splashWnd->Update();

	if (m_modeRun == eRunMode::ENTERPRISE_MODE)
	{
		// Using a space so the initial 'w' will not be capitalized in wxLogGUI dialogs
		wxApp::SetAppName(wxT("OES Enterprise"));

		// Creating the wxConfig manually so there will be no space
		// The old config (if any) is returned, delete it
		delete wxConfigBase::Set(new wxConfig(wxT("Enterprise")));
	}
	else
	{
		// Using a space so the initial 'w' will not be capitalized in wxLogGUI dialogs
		wxApp::SetAppName(wxT("OES Designer"));

		// Creating the wxConfig manually so there will be no space
		// The old config (if any) is returned, delete it
		delete wxConfigBase::Set(new wxConfig(wxT("Designer")));
	}

	// Log to stderr while working on the command line
	delete wxLog::SetActiveTarget(new wxLogStderr);

	// Message output to the same as the log target
	delete wxMessageOutput::Set(new wxMessageOutputLog);

	// Init handlers
	wxInitAllImageHandlers();

	wxXmlResource::Get()->InitAllHandlers();
#if wxVERSION_NUMBER >= 2905 && wxVERSION_NUMBER <= 3100
	wxXmlResource::Get()->AddHandler(new wxAuiNotebookXmlHandler);
#elif wxVERSION_NUMBER > 3100
	wxXmlResource::Get()->AddHandler(new wxAuiXmlHandler);
#endif

#ifdef __WXMSW__
	::DisableProcessWindowsGhosting();
#endif 

#if _DEBUG 
	wxLog::AddTraceMask(wxTRACE_MemAlloc);
	wxLog::AddTraceMask(wxTRACE_ResAlloc);
#if wxUSE_LOG
#if defined(__WXGTK__)
	wxLog::AddTraceMask("clipboard");
#elif defined(__WXMSW__)
	wxLog::AddTraceMask(wxTRACE_OleCalls);
#endif
#endif // wxUSE_LOG
#endif

#if wxUSE_LIBPNG
	wxImage::AddHandler(new wxPNGHandler);
#endif

	// Support loading files from memory
	// Used to load the XRC preview, but could be useful elsewhere
	wxFileSystem::AddHandler(new wxMemoryFSHandler);

	// Support for loading files from archives
	wxFileSystem::AddHandler(new wxArchiveFSHandler);
	wxFileSystem::AddHandler(new wxFilterFSHandler);

	wxSocketBase::Initialize();

	// Init appData
	appDataCreate(dataDir);

	// If connection is failed then exit from application 
	if (!objectDatabase->IsOpen())
	{
		m_splashWnd->Destroy();
		wxMessageBox(objectDatabase->GetErrorMessage(), wxT("Failed to connection!"), wxOK | wxCENTRE | wxICON_ERROR);
		return 1;
	}

	// This is not necessary for Enterprise to work. However, Windows sets the Current Working Directory
	// to the directory from which a .md file was opened, if opened from Windows Explorer.
	// This puts an unneccessary lock on the directory.
	// This changes the CWD to the already locked app directory as a workaround
#ifdef __WXMSW__
	::wxSetWorkingDirectory(dataDir);
#endif

	if (!appData->Initialize(m_modeRun, userIB, passwordIB))
	{
		m_splashWnd->Destroy();
		return 1;
	}

	m_splashWnd->Destroy();
	return wxApp::OnRun();
}

#include "compiler/valueOLE.h"

void CDesignerApp::OnUnhandledException()
{
	wxApp::OnUnhandledException();
}

void CDesignerApp::OnFatalException()
{
	//release all created com-objects
	CValueOLE::ReleaseCoObjects();

	//generate dump
	wxDebugReport report;
	report.AddExceptionDump();

	//close connection to db
	objectDatabase->Close();

	//decr socket
	if (wxThread::IsMain()) {
		wxSocketBase::Shutdown();
	}

	// Allow clipboard data to persist after close
	if (wxTheClipboard->IsOpened())
	{
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->Flush();
			wxTheClipboard->Close();
		}
	}

	//show error
	wxDebugReportPreviewStd preview;
	if (preview.Show(report)) report.Process();
}

int CDesignerApp::OnExit()
{
	wxSocketBase::Shutdown();

	appDataDestroy();

	if (!wxTheClipboard->IsOpened())
	{
		if (!wxTheClipboard->Open())
		{
			return wxApp::OnExit();
		}
	}

	// Allow clipboard data to persist after close
	wxTheClipboard->Flush();
	wxTheClipboard->Close();

	return wxApp::OnExit();
}