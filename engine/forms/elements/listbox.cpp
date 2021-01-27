
#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueListBox, CValueWindow)

//****************************************************************************
//*                             Listbox                                      *
//****************************************************************************

CValueListBox::CValueListBox() : CValueWindow()
{
	PropertyCategory *m_categoryButton = new PropertyCategory("Listbox");

	//property
	m_categoryButton->AddProperty("name");
	m_categoryButton->AddProperty("choices");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["choices"] = new Property("choices", PropertyType::PT_STRINGLIST, this);
}

wxObject* CValueListBox::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxListBox *m_listbox = new wxListBox((wxWindow*)parent, wxID_ANY,
		m_pos,
		m_size,
		0,
		NULL,
		m_style | m_window_style);

	// choices
	for (unsigned int i = 0; i < m_choices.Count(); i++)
		m_listbox->Append(m_choices[i]);

	return m_listbox;
}

void CValueListBox::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueListBox::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxListBox *m_listbox = dynamic_cast<wxListBox*>(wxobject);

	if (m_listbox)
	{
	}

	UpdateWindow(m_listbox);
}

void CValueListBox::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*            Property                                             *
//*******************************************************************

void CValueListBox::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["choices"]->SetValue(m_choices);
}

void CValueListBox::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_choices = m_properties["choices"]->GetValueAsArrayString();
}
