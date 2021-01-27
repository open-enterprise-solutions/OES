
#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueCheckbox, CValueWindow)

//****************************************************************************
//*                              Checkbox                                    *
//****************************************************************************

CValueCheckbox::CValueCheckbox() : CValueWindow()
{
	PropertyCategory *m_categoryButton = new PropertyCategory("Checkbox");

	//property
	m_categoryButton->AddProperty("name");
	m_categoryButton->AddProperty("checked");
	m_categoryButton->AddProperty("label");

	//event
	m_categoryButton->AddEvent("OnButtonPressed");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["checked"] = new Property("checked", PropertyType::PT_BOOL, this);
	m_properties["label"] = new Property("label", PropertyType::PT_WXSTRING, this);

	m_events["OnButtonPressed"] = new Event("OnCheckboxClicked", "123", this);
}

wxObject* CValueCheckbox::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxCheckBox *m_checkbox = new wxCheckBox((wxWindow *)parent, wxID_ANY,
		m_label,
		m_pos,
		m_size,
		m_window_style | m_style);

	m_checkbox->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &CValueCheckbox::OnClickedCheckbox, this);

	return m_checkbox;
}

void CValueCheckbox::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueCheckbox::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxCheckBox *m_checkbox = dynamic_cast<wxCheckBox *>(wxobject);

	if (m_checkbox)
	{
		m_checkbox->SetLabel(m_label);
		m_checkbox->SetValue(m_checked != false);
	}

	UpdateWindow(m_checkbox);
}

void CValueCheckbox::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*            Property                                             *
//*******************************************************************

void CValueCheckbox::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["checked"]->SetValue(m_checked);
	m_properties["label"]->SetValue(m_label);
}

void CValueCheckbox::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_checked = m_properties["checked"]->GetValueAsInteger();
	m_label = m_properties["label"]->GetValueAsString();
}