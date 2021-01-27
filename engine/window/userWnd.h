#ifndef _USERWND_H__
#define _USERWND_H__

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
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>

class userWnd : public wxDialog
{
	private:

	protected:
		wxAuiToolBar* m_auiToolBarUser;
		wxAuiToolBarItem* m_toolOk;
		wxAuiToolBarItem* m_toolSave;
		wxAuiToolBarItem* m_toolClose;
		wxStaticText* m_staticTextLogin;
		wxStaticText* m_staticTextPassword;
		wxTextCtrl* m_textCtrlLogin;
		wxTextCtrl* m_textCtrlPassword;

	public:

		userWnd( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Current user"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
		~userWnd();

};

#endif 