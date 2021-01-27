////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxwidgets community
//	Description : main frame window
////////////////////////////////////////////////////////////////////////////

#include "mainFrameDesigner.h"
#include <wx/config.h>

//********************************************************************************
//*                                Hotkey support                                *
//********************************************************************************

void CMainFrameDesigner::SetDefaultHotKeys()
{
	// Setup the hotkeys.
	m_keyBinder.SetShortcut(wxID_NEW, wxT("Ctrl+N"));
	m_keyBinder.SetShortcut(wxID_SAVE, wxT("Ctrl+S"));
	m_keyBinder.SetShortcut(wxID_UNDO, wxT("Ctrl+Z"));
	m_keyBinder.SetShortcut(wxID_REDO, wxT("Ctrl+Y"));

	m_keyBinder.SetShortcut(wxID_CUT, wxT("Ctrl+X"));
	m_keyBinder.SetShortcut(wxID_COPY, wxT("Ctrl+C"));
	m_keyBinder.SetShortcut(wxID_PASTE, wxT("Ctrl+V"));
	m_keyBinder.SetShortcut(wxID_SELECTALL, wxT("Ctrl+A"));

	m_keyBinder.SetShortcut(wxID_DEBUG_START, wxT("F5")); //RUN 
	m_keyBinder.SetShortcut(wxID_DEBUG_START_WITHOUT_DEBUGGING, wxT("Ctrl+F5")); // RUN WITHOUT DEBUGGER 

	m_keyBinder.SetShortcut(wxID_DEBUG_STEP_INTO, wxT("F11")); //STEP INTO 
	m_keyBinder.SetShortcut(wxID_DEBUG_STEP_OVER, wxT("F10")); // STEP OVER 

	m_keyBinder.SetShortcut(wxID_DEBUG_STOP_PROGRAM, wxT("Ctrl+Break")); // STEP OVER 

	m_keyBinder.SetShortcut(wxID_DEBUG_NEXT_POINT, wxT("F9"));
}

//********************************************************************************
//*                                Default menu                                  *
//********************************************************************************

void CMainFrameDesigner::InitializeDefaultMenu()
{
	wxMenuBar *m_menuBar = new wxMenuBar;

	// and its menu bar
	m_menuFile = new wxMenu();

	m_menuFile->Append(wxID_NEW);
	m_menuFile->Append(wxID_OPEN);

	m_menuFile->Append(wxID_CLOSE);
	m_menuFile->Append(wxID_SAVE);
	m_menuFile->Append(wxID_SAVEAS);
	m_menuFile->Append(wxID_REVERT, _("Re&vert..."));

	m_menuFile->AppendSeparator();
	m_menuFile->Append(wxID_PRINT);
	m_menuFile->Append(wxID_PRINT_SETUP, "Print &Setup...");
	m_menuFile->Append(wxID_PREVIEW);

	m_menuFile->AppendSeparator();
	m_menuFile->Append(wxID_EXIT);

	m_menuBar->Append(m_menuFile, wxGetStockLabel(wxID_FILE));

	// A nice touch: a history of files visited. Use this menu.
	m_docManager->FileHistoryUseMenu(m_menuFile);

#if wxUSE_CONFIG
	m_docManager->FileHistoryLoad(*wxConfig::Get());
#endif // wxUSE_CONFIG

	m_menuEdit = new wxMenu;
	m_menuEdit->Append(wxID_UNDO);
	m_menuEdit->Append(wxID_REDO);
	m_menuEdit->AppendSeparator();
	m_menuEdit->Append(wxID_CUT);
	m_menuEdit->Append(wxID_COPY);
	m_menuEdit->Append(wxID_PASTE);
	m_menuEdit->Append(wxID_DELETE);
	m_menuEdit->Append(wxID_SELECTALL);
	m_menuEdit->AppendSeparator();
	m_menuEdit->Append(wxID_FIND);

	m_menuBar->Append(m_menuEdit, wxGetStockLabel(wxID_EDIT));

	m_menuDebug = new wxMenu;
	m_menuDebug->Append(wxID_DEBUG_START, _("Run enterprise"));
	m_menuDebug->Append(wxID_DEBUG_START_WITHOUT_DEBUGGING, _("Run enterprise without debugging"));

	m_menuDebug->AppendSeparator();

	m_menuDebug->Append(wxID_DEBUG_NEXT_POINT, _("Continue"))->Enable(false);
	m_menuDebug->Append(wxID_DEBUG_PAUSE, _("Pause"), _("Pause"))->Enable(false);
	m_menuDebug->Append(wxID_DEBUG_STEP_INTO, _("Step into"))->Enable(false);
	m_menuDebug->Append(wxID_DEBUG_STEP_OVER, _("Step over"))->Enable(false);
	m_menuDebug->Append(wxID_DEBUG_STOP_DEBUGGING, _("Stop debugging"), _("Stop debugging"))->Enable(false);
	m_menuDebug->Append(wxID_DEBUG_STOP_PROGRAM, _("Stop debugging program"), _("Stop program"))->Enable(false);
	
	m_menuDebug->AppendSeparator();

	m_menuDebug->Append(wxID_DEBUG_REMOVE_ALL_DEBUGPOINTS, _("Remove all breakpoits"));

	m_menuConfiguration = new wxMenu;
	m_menuConfiguration->Append(wxID_CONFIGURATION_LOAD, _("Load configuraion")); 
	m_menuConfiguration->Append(wxID_CONFIGURATION_SAVE, _("Save configuration"));

	m_menuBar->Append(m_menuConfiguration, _("Configuration"));
	m_menuBar->Append(m_menuDebug, _("Debug"));

	m_menuSetting = new wxMenu;
	m_menuBar->Append(m_menuSetting, _("Tools"));
	m_menuSetting->Append(wxID_SETTING, _("Setting..."));


	m_menuAdministration = new wxMenu;
	m_menuAdministration->Append(wxID_USERS, _("Users"));
	m_menuBar->Append(m_menuAdministration, _("Administration"));

	m_keyBinder.AddCommandsFromMenuBar(m_menuBar);

	SetDefaultHotKeys();
	SetMenuBar(m_menuBar);

	Bind(wxEVT_MENU, &CMainFrameDesigner::OnConfiguration, this, wxID_CONFIGURATION_LOAD, wxID_CONFIGURATION_SAVE);
	Bind(wxEVT_MENU, &CMainFrameDesigner::OnStartDebug, this, wxID_DEBUG_START);
	Bind(wxEVT_MENU, &CMainFrameDesigner::OnStartDebugWithoutDebug, this, wxID_DEBUG_START_WITHOUT_DEBUGGING);
	Bind(wxEVT_MENU, &CMainFrameDesigner::OnRunDebugCommand, this, wxID_DEBUG_EDIT_POINT, wxID_DEBUG_REMOVE_ALL_DEBUGPOINTS);
	Bind(wxEVT_MENU, &CMainFrameDesigner::OnToolsSettings, this, wxID_SETTING);
	Bind(wxEVT_MENU, &CMainFrameDesigner::OnUsers, this, wxID_USERS);

	m_keyBinder.UpdateWindow(this);
	m_keyBinder.UpdateMenuBar(GetMenuBar());
}