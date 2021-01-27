////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxFormBuilder community
//	Description : error dialog window
////////////////////////////////////////////////////////////////////////////

#include "errorDialog.h"

#define DEF_LINENUMBER_ID 0
#define DEF_IMAGE_ID 1

CErrorDialog::CErrorDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	wxBoxSizer* m_bSizerMain = new wxBoxSizer(wxHORIZONTAL);

	m_errorWnd = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(300, 250), 0, wxEmptyString);

	// initialize styles
	m_errorWnd->StyleClearAll();

	//set Lexer to LEX_CONTAINER: This will trigger the styleneeded event so you can do your own highlighting
	m_errorWnd->SetLexer(wxSTC_LEX_CONTAINER);

	//Set margin cursor
	for (int margin = 0; margin < m_errorWnd->GetMarginCount(); margin++)
		m_errorWnd->SetMarginCursor(margin, wxSTC_CURSORARROW);

	m_errorWnd->SetMarginType(DEF_LINENUMBER_ID, wxSTC_MARGIN_NUMBER);
	m_errorWnd->SetMarginWidth(DEF_LINENUMBER_ID, 0);

	// set margin as unused
	m_errorWnd->SetMarginType(DEF_IMAGE_ID, wxSTC_MARGIN_SYMBOL);
	m_errorWnd->SetMarginMask(DEF_IMAGE_ID, ~(1024 | 256 | 512 | 128 | 64 | wxSTC_MASK_FOLDERS));
	m_errorWnd->StyleSetBackground(DEF_IMAGE_ID, *wxWHITE);

	m_errorWnd->SetMarginWidth(DEF_IMAGE_ID, FromDIP(16));
	m_errorWnd->SetMarginSensitive(DEF_IMAGE_ID, true);

	m_bSizerMain->Add(m_errorWnd, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* m_bSizerButtons = new wxBoxSizer(wxVERTICAL);

	m_buttonGotoDesigner = new wxButton(this, wxID_ANY, wxT("Go to designer"), wxDefaultPosition, wxDefaultSize, 0);
	m_bSizerButtons->Add(m_buttonGotoDesigner, 0, wxALL | wxEXPAND, 5);

	m_buttonClose = new wxButton(this, wxID_ANY, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0);
	m_bSizerButtons->Add(m_buttonClose, 0, wxALL | wxEXPAND, 5);

	m_checkLoopVariables = new wxCheckBox(this, wxID_ANY, wxT("Loop last error"), wxDefaultPosition, wxDefaultSize, 0);
	m_bSizerButtons->Add(m_checkLoopVariables, 0, wxALL | wxEXPAND, 5);

	m_bSizerMain->Add(m_bSizerButtons, 0, wxEXPAND, 5);

	this->SetSizer(m_bSizerMain);
	this->Layout();
	m_bSizerMain->Fit(this);

	this->Centre(wxBOTH);

	// Connect Events
	m_buttonGotoDesigner->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CErrorDialog::OnButtonGotoDesignerClick), NULL, this);
	m_buttonClose->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CErrorDialog::OnButtonCloseClick), NULL, this);

	/** Enumeration of commands and child windows. */
	enum
	{
		idcmdUndo = 10,
		idcmdRedo = 11,
		idcmdCut = 12,
		idcmdCopy = 13,
		idcmdPaste = 14,
		idcmdDelete = 15,
		idcmdSelectAll = 16
	};

	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_CTRL, (int) 'A', idcmdSelectAll);
	entries[1].Set(wxACCEL_CTRL, (int) 'C', idcmdCopy);

	wxAcceleratorTable accel(2, entries);
	m_errorWnd->SetAcceleratorTable(accel);
}

void CErrorDialog::OnButtonGotoDesignerClick(wxCommandEvent &event)
{
	EndModal(m_checkLoopVariables->GetValue() ? 2 : 1); Destroy(); event.Skip();
}

void CErrorDialog::OnButtonCloseClick(wxCommandEvent &event)
{
	EndModal(3); Destroy(); event.Skip();
}

CErrorDialog::~CErrorDialog()
{
	// Disconnect Events
	m_buttonGotoDesigner->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CErrorDialog::OnButtonGotoDesignerClick), NULL, this);
	m_buttonClose->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CErrorDialog::OnButtonCloseClick), NULL, this);
}
