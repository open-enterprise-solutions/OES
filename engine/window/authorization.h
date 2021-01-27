#ifndef _authorization_h__
#define _authorization_h__

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>

class authorizationWnd : public wxDialog
{
private:

protected:
	wxStaticText* m_staticTextLogin;
	wxStaticText* m_staticTextPassword;
	wxComboBox* m_comboBoxLogin;
	wxTextCtrl* m_textCtrlPassword;
	wxButton* m_buttonOK;
	wxButton* m_buttonCancel;

public:

	authorizationWnd(wxWindow* parent = NULL, wxWindowID id = wxID_ANY, const wxString& title = wxT("authorization"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(420, 130), long style = wxDEFAULT_DIALOG_STYLE);
	~authorizationWnd();

	void SetLogin(const wxString& login) { m_comboBoxLogin->SetValue(login); }
	void SetPassword(const wxString& password) { m_textCtrlPassword->SetValue(password); }
};

#endif 