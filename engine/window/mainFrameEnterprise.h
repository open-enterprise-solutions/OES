#ifndef _MAINFRAME_ENTERPRISE_H__
#define _MAINFRAME_ENTERPRISE_H__

#include "mainFrame.h"

enum
{
	wxID_ALL_OPERATIONS = 10000
};

class CMainFrameEnterprise : public CMainFrame
{
	wxMenu *m_menuFile;
	wxMenu *m_menuEdit;

	wxMenu *m_menuOperations;

	CMetadataTree *m_metadataTree;

protected:

	void InitializeDefaultMenu();

	void CreatePropertyManager();
	void CreateMessageAndDebugBar();

	void CreateWideGui();

	/**
	* Adds the default profile to the hot keys.
	*/
	void SetDefaultHotKeys();

public:

	CMainFrameEnterprise(const wxString& title = wxT("Enterprise"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	virtual void CreateGUI();

	virtual wxMenu *GetDefaultMenu(int nTypeMenu)
	{
		return NULL;
	}

	//events:
	void OnClickAllOperaions(wxCommandEvent &event);
};

#endif 