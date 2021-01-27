#ifndef _USERLISTWND_H__
#define _USERLISTWND_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/gdicmn.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibar.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/dataview.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

class userListWnd : public wxDialog
{
	wxAuiToolBar* m_auiToolBarUsers;
	wxAuiToolBarItem* m_tool1;
	wxAuiToolBarItem* m_tool2;
	wxAuiToolBarItem* m_tool3;
	wxAuiToolBarItem* m_tool4;
	wxAuiToolBarItem* m_tool5;
	wxDataViewCtrl* m_dataViewUsers;

public:

	userListWnd(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("User list"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(614, 317), long style = wxDEFAULT_DIALOG_STYLE);
	~userListWnd();

	void OnDataViewUsersOnRightDClick(wxMouseEvent& event)
	{
		wxMenu *m_menu = new wxMenu;
		m_menu->Append(1, "oiugfds");
		m_dataViewUsers->PopupMenu(m_menu, event.GetPosition());
		event.Skip();
	}

	void OnToolClicked(wxCommandEvent& event) 
	{
		event.Skip(); 
	}
};

#endif 