////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : form selector
////////////////////////////////////////////////////////////////////////////

#include "formSelector.h"
#include "metadata/metaObjects/metaFormObject.h"
#include "window/mainFrame.h"

CSelectTypeForm::CSelectTypeForm(IMetaFormObject *metaObject)
	: wxDialog(CMainFrame::Get(), wxID_ANY, _("Select type type"), wxDefaultPosition, wxSize(480, 320), wxDEFAULT_DIALOG_STYLE | wxDIALOG_ADAPTATION_ANY_SIZER), m_metaObject(metaObject)
{
}

CSelectTypeForm::~CSelectTypeForm()
{
}

void CSelectTypeForm::CreateSelector()
{
	wxBoxSizer* bSizerMain = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer* sbSizerMain = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Select type form")), wxVERTICAL);

	for (auto choice : m_aChoices) {
		wxRadioButton *radioButton = new wxRadioButton(sbSizerMain->GetStaticBox(), choice.m_choice, choice.m_name, wxDefaultPosition, wxDefaultSize);
		radioButton->Connect(wxEVT_RADIOBUTTON, wxCommandEventHandler(CSelectTypeForm::OnFormTypeChanged), NULL, this);
		sbSizerMain->Add(radioButton, 1, wxALL | wxEXPAND, 5);
	}

	wxRadioButton *radioButton = new wxRadioButton(sbSizerMain->GetStaticBox(), formDefaultID, formDefaultName, wxDefaultPosition, wxDefaultSize);
	radioButton->Connect(wxEVT_RADIOBUTTON, wxCommandEventHandler(CSelectTypeForm::OnFormTypeChanged), NULL, this);
	sbSizerMain->Add(radioButton, 1, wxALL | wxEXPAND, 5);

	bSizerMain->Add(sbSizerMain, 1, wxEXPAND, 5);

	wxBoxSizer* bSizerHeader = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bSizerLeft = new wxBoxSizer(wxVERTICAL);

	m_staticTextName = new wxStaticText(this, wxID_ANY, wxT("Name"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextName->Wrap(-1);

	bSizerLeft->Add(m_staticTextName, 0, wxALL, 5);

	m_staticTextAlias = new wxStaticText(this, wxID_ANY, wxT("Alias"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextAlias->Wrap(-1);

	bSizerLeft->Add(m_staticTextAlias, 0, wxALL, 5);

	m_staticTextComment = new wxStaticText(this, wxID_ANY, wxT("Comment"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextComment->Wrap(-1);

	bSizerLeft->Add(m_staticTextComment, 0, wxALL, 5);
	bSizerHeader->Add(bSizerLeft, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerRight = new wxBoxSizer(wxVERTICAL);

	m_textCtrlName = new wxTextCtrl(this, wxID_ANY, m_metaObject->GetName(), wxDefaultPosition, wxDefaultSize, 0);
	bSizerRight->Add(m_textCtrlName, 0, wxALL | wxEXPAND, 1);

	m_textCtrlAlias = new wxTextCtrl(this, wxID_ANY, m_metaObject->GetAlias(), wxDefaultPosition, wxDefaultSize, 0);
	bSizerRight->Add(m_textCtrlAlias, 0, wxALL | wxEXPAND, 1);

	m_textCtrlComment = new wxTextCtrl(this, wxID_ANY, m_metaObject->GetComment(), wxDefaultPosition, wxDefaultSize, 0);
	bSizerRight->Add(m_textCtrlComment, 0, wxALL | wxEXPAND, 1);

	bSizerHeader->Add(bSizerRight, 1, wxEXPAND, 5);
	bSizerMain->Add(bSizerHeader, 1, wxEXPAND, 10);

	wxBoxSizer* bSizerBottom = new wxBoxSizer(wxHORIZONTAL);

	m_sdbSizerOK = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
	m_sdbSizerOK->Connect(wxEVT_BUTTON, wxCommandEventHandler(CSelectTypeForm::OnButtonOk), NULL, this);

	bSizerBottom->Add(m_sdbSizerOK, 0, wxALL, 5);

	m_sdbSizerCancel = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	m_sdbSizerCancel->Connect(wxEVT_BUTTON, wxCommandEventHandler(CSelectTypeForm::OnButtonCancel), NULL, this);
	bSizerBottom->Add(m_sdbSizerCancel, 0, wxALL, 5);

	bSizerMain->Add(bSizerBottom, 0, wxEXPAND, 5);

	this->SetSizer(bSizerMain);
	this->Layout();
}

void CSelectTypeForm::OnButtonOk(wxCommandEvent &event)
{
	m_metaObject->SetName(m_textCtrlName->GetValue());
	m_metaObject->SetAlias(m_textCtrlAlias->GetValue());
	m_metaObject->SetComment(m_textCtrlComment->GetValue());

	EndModal(m_choice);
	event.Skip();
}

void CSelectTypeForm::OnButtonCancel(wxCommandEvent & event)
{
	EndModal(formDefaultID);
	event.Skip();
}

void CSelectTypeForm::OnFormTypeChanged(wxCommandEvent &event)
{
	m_choice = event.GetId();
	event.Skip();
}