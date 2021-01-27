#ifndef _MAINFRAME_DESIGNER_H__
#define _MAINFRAME_DESIGNER_H__

#include "mainFrame.h"

enum
{
	wxID_DEBUG_START = 10000,
	wxID_DEBUG_START_WITHOUT_DEBUGGING = 10001,

	wxID_DEBUG_EDIT_POINT = 1000,
	wxID_DEBUG_STEP_OVER = 1001,
	wxID_DEBUG_STEP_INTO = 1002,

	wxID_DEBUG_PAUSE = 1004,
	wxID_DEBUG_STOP_DEBUGGING = 1005,
	wxID_DEBUG_STOP_PROGRAM = 1006,

	wxID_DEBUG_NEXT_POINT = 1007,

	wxID_DEBUG_REMOVE_ALL_DEBUGPOINTS
};

//menu  
enum
{
	wxID_DEBUG = wxID_HIGHEST + 1,
    wxID_SETTING,
	wxID_USERS 
};

enum
{
	wxID_CONFIGURATION_LOAD = wxID_USERS + 1,
	wxID_CONFIGURATION_SAVE,
};

#include "compiler/debugger/debugEvent.h"

class CMainFrameDesigner : public CMainFrame
{
	wxMenu *m_menuFile;
	wxMenu *m_menuEdit;
	wxMenu *m_menuConfiguration;
	wxMenu *m_menuDebug; 
	wxMenu *m_menuSetting;
	wxMenu *m_menuAdministration;

	CMetadataTree *m_metadataTree;

protected:

	void InitializeDefaultMenu();

	void CreateObjectTree();
	void CreatePropertyManager();
	void CreateMessageAndDebugBar();

	void CreateWideGui();

	/**
	* Adds the default profile to the hot keys.
	*/
	void SetDefaultHotKeys();

	/**
	* Updates all of the open editors with the current font, color, etc.
	* options.
	*/
	void UpdateEditorOptions();

public:

	CMainFrameDesigner(const wxString& title = wxT("Designer"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	virtual ~CMainFrameDesigner();

	virtual wxMenu *GetDefaultMenu(int nTypeMenu)
	{
		switch (nTypeMenu)
		{
		    case wxID_FILE: return GetMenuFile();
			case wxID_EDIT: return GetMenuEdit();
			case wxID_DEBUG: return GetMenuDebug();
			case wxID_SETTING: return GetMenuSetting();
		}

		return NULL; 
	}

	virtual CMetadataTree *GetMetadataTree() { return m_metadataTree; }
	virtual void CreateGUI() override;

	wxMenu *GetMenuFile() { return m_menuFile; }
	wxMenu *GetMenuEdit() { return m_menuEdit; }
	wxMenu *GetMenuDebug() { return m_menuDebug; }
	wxMenu *GetMenuSetting() { return m_menuSetting; }

	virtual bool Show(bool show = false) override;

protected:

	//events 
	void OnStartDebug(wxCommandEvent& WXUNUSED(event));
	void OnStartDebugWithoutDebug(wxCommandEvent& WXUNUSED(event));

	void OnConfiguration(wxCommandEvent& event);
	void OnRunDebugCommand(wxCommandEvent& event);
	void OnToolsSettings(wxCommandEvent& event);
	void OnUsers(wxCommandEvent& event);

	void OnDebugEvent(wxDebugEvent &event);
	void OnToolbarClicked(wxEvent &event);
};

#endif 