
#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueChoice, CValueWindow)

//****************************************************************************
//*                             Choice                                       *
//****************************************************************************

CValueChoice::CValueChoice() : CValueWindow()
{
	PropertyCategory *m_categoryButton = new PropertyCategory("Choice");

	//property
	m_categoryButton->AddProperty("name");
	m_categoryButton->AddProperty("choices");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["choices"] = new Property("choices", PropertyType::PT_STRINGLIST, this);
}

wxObject* CValueChoice::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxString *strings = new wxString[m_choices.GetCount()];
	for (unsigned int i = 0; i < m_choices.GetCount(); i++)
		strings[i] = m_choices[i];

	wxChoice *m_choice = new wxChoice((wxWindow*)parent, wxID_ANY,
		m_pos,
		m_size,
		(int)m_choices.Count(),
		strings,
		m_window_style);

	int sel = m_selection;
	if (sel < (int)m_choices.GetCount()) m_choice->SetSelection(sel);

	delete[]strings;
	return m_choice;
}

void CValueChoice::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueChoice::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxChoice *m_choice = dynamic_cast<wxChoice *>(wxobject);

	if (m_choice)
	{
	}

	UpdateWindow(m_choice);
}

void CValueChoice::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*            Property                                             *
//*******************************************************************

void CValueChoice::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["choices"]->SetValue(m_choices);
}

void CValueChoice::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_choices = m_properties["choices"]->GetValueAsArrayString();
}
