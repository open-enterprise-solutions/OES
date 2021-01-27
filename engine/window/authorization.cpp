#include "authorization.h"

///////////////////////////////////////////////////////////////////////////

authorizationWnd::authorizationWnd(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* bSizerCtrl = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bSizerHeader = new wxBoxSizer(wxVERTICAL);

	m_staticTextLogin = new wxStaticText(this, wxID_ANY, wxT("Login:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextLogin->Wrap(-1);
	bSizerHeader->Add(m_staticTextLogin, 0, wxALL, 10);

	m_staticTextPassword = new wxStaticText(this, wxID_ANY, wxT("Password:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextPassword->Wrap(-1);
	bSizerHeader->Add(m_staticTextPassword, 0, wxALL, 9);

	bSizerCtrl->Add(bSizerHeader, 0, 0, 5);

	wxBoxSizer* bSizerBottom = new wxBoxSizer(wxVERTICAL);

	m_comboBoxLogin = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	bSizerBottom->Add(m_comboBoxLogin, 0, wxALL | wxEXPAND, 5);
	m_textCtrlPassword = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	bSizerBottom->Add(m_textCtrlPassword, 0, wxALL | wxEXPAND, 5);
	bSizerCtrl->Add(bSizerBottom, 1, wxEXPAND, 5);
	bSizer->Add(bSizerCtrl, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerButtons = new wxBoxSizer(wxHORIZONTAL);

	m_buttonOK = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_buttonOK, 0, wxALL, 5);

	m_buttonCancel = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_buttonCancel, 0, wxALL, 5);

	bSizer->Add(bSizerButtons, 1, wxALIGN_RIGHT, 5);

	this->SetSizer(bSizer);
	this->Layout();

	this->Centre(wxBOTH);
}

authorizationWnd::~authorizationWnd()
{
}
