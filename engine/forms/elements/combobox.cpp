
#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueComboBox, CValueWindow)

//****************************************************************************
//*                             ComboBox                                     *
//****************************************************************************

CValueComboBox::CValueComboBox() : CValueWindow()//, m_combobox(NULL)
{
	PropertyCategory *m_categoryButton = new PropertyCategory("Combobox");

	//property
	m_categoryButton->AddProperty("name");
	m_categoryButton->AddProperty("choices");
	m_categoryButton->AddProperty("value");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["choices"] = new Property("choices", PropertyType::PT_STRINGLIST, this);
	m_properties["value"] = new Property("value", PropertyType::PT_WXSTRING, this);
}

wxObject* CValueComboBox::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxComboBox *m_combobox = new wxComboBox((wxWindow *)parent, wxID_ANY,
		m_value,
		m_pos,
		m_size,
		0,
		NULL,
		m_style | m_window_style);

	// choices
	for (unsigned int i = 0; i < m_choices.GetCount(); i++)
		m_combobox->Append(m_choices[i]);

	int sel = m_selection;
	if (sel > -1 && sel < (int)m_choices.GetCount()) m_combobox->SetSelection(sel);

	return m_combobox;
}

void CValueComboBox::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueComboBox::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxComboBox *m_combobox = dynamic_cast<wxComboBox *>(wxobject);

	if (m_combobox)
	{
		m_combobox->SetValue(m_value);
	}

	UpdateWindow(m_combobox);
}

void CValueComboBox::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*            Property                                             *
//*******************************************************************

void CValueComboBox::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["choices"]->SetValue(m_choices);
	m_properties["value"]->SetValue(m_value);
}

void CValueComboBox::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_choices = m_properties["choices"]->GetValueAsArrayString();
	m_value = m_properties["value"]->GetValueAsString();
}
