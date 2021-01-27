#include "userWnd.h"

userWnd::userWnd(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizerUser = new wxBoxSizer(wxVERTICAL);

	m_auiToolBarUser = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT);
	m_toolOk = m_auiToolBarUser->AddTool(wxID_ANY, wxT("tool"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL);
	m_toolSave = m_auiToolBarUser->AddTool(wxID_ANY, wxT("tool"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL);
	m_toolClose = m_auiToolBarUser->AddTool(wxID_ANY, wxT("tool"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL);
	m_auiToolBarUser->Realize();

	bSizerUser->Add(m_auiToolBarUser, 0, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizerHeader = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bSizerText = new wxBoxSizer(wxVERTICAL);

	m_staticTextLogin = new wxStaticText(this, wxID_ANY, wxT("Login:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextLogin->Wrap(-1);
	bSizerText->Add(m_staticTextLogin, 0, wxALL | wxEXPAND, 8);

	m_staticTextPassword = new wxStaticText(this, wxID_ANY, wxT("Password:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextPassword->Wrap(-1);
	bSizerText->Add(m_staticTextPassword, 0, wxALL | wxEXPAND, 8);
	bSizerHeader->Add(bSizerText, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerControl = new wxBoxSizer(wxVERTICAL);

	m_textCtrlLogin = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizerControl->Add(m_textCtrlLogin, 0, wxALL, 5);

	m_textCtrlPassword = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	bSizerControl->Add(m_textCtrlPassword, 0, wxALL, 5);

	bSizerHeader->Add(bSizerControl, 1, wxEXPAND, 5);
	bSizerUser->Add(bSizerHeader, 1, wxEXPAND, 5);

	this->SetSizer(bSizerUser);
	this->Layout();
	bSizerUser->Fit(this);

	this->Centre(wxBOTH);
}

userWnd::~userWnd()
{
}
