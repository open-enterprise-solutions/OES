
#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueRadioButton, CValueWindow)

//****************************************************************************
//*                             Radiobutton                                  *
//****************************************************************************

CValueRadioButton::CValueRadioButton() : CValueWindow()
{
	PropertyCategory *m_categoryButton = new PropertyCategory("Radiobutton");

	//property
	m_categoryButton->AddProperty("name");
	m_categoryButton->AddProperty("selected");
	m_categoryButton->AddProperty("label");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["selected"] = new Property("selected", PropertyType::PT_BOOL, this);
	m_properties["label"] = new Property("label", PropertyType::PT_WXSTRING, this);
}

wxObject* CValueRadioButton::Create(wxObject* parent, IVisualHost *visualHost) 
{
	wxRadioButton *m_radiobutton = new wxRadioButton((wxWindow *)parent, wxID_ANY,
		m_label,
		m_pos,
		m_size,
		m_style | m_window_style);

	return m_radiobutton;
}

void CValueRadioButton::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueRadioButton::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxRadioButton *m_radiobutton = dynamic_cast<wxRadioButton *>(wxobject);

	if (m_radiobutton)
	{
		m_radiobutton->SetLabel(m_label);
		m_radiobutton->SetValue(m_selected != false);
	}

	UpdateWindow(m_radiobutton);
}

void CValueRadioButton::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*                             Property                            *
//*******************************************************************

void CValueRadioButton::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["selected"]->SetValue(m_selected);
	m_properties["label"]->SetValue(m_label);
}

void CValueRadioButton::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_selected = m_properties["selected"]->GetValueAsInteger();
	m_label = m_properties["label"]->GetValueAsString();
}