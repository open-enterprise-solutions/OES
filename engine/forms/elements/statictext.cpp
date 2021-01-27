
#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueStaticText, CValueWindow)

//****************************************************************************
//*                              StaticText                                  *
//****************************************************************************

CValueStaticText::CValueStaticText() : CValueWindow()
{
	PropertyCategory *m_categoryButton = new PropertyCategory("Statictext");

	//property
	m_categoryButton->AddProperty("name");
	m_categoryButton->AddProperty("markup");
	m_categoryButton->AddProperty("wrap");
	m_categoryButton->AddProperty("label");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["markup"] = new Property("markup", PropertyType::PT_BOOL, this);
	m_properties["wrap"] = new Property("wrap", PropertyType::PT_INT, this);
	m_properties["label"] = new Property("label", PropertyType::PT_WXSTRING, this);
}

wxObject* CValueStaticText::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxStaticText *m_static_text = new wxStaticText((wxWindow *)parent, wxID_ANY,
		m_label,
		m_pos,
		m_size,
		m_style | m_window_style);

	return m_static_text;
}

void CValueStaticText::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueStaticText::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxStaticText *m_static_text = dynamic_cast<wxStaticText *>(wxobject);

	if (m_static_text)
	{
		m_static_text->SetLabel(m_label);
		m_static_text->Wrap(m_wrap);

		if (m_markup != false)
		{
			m_static_text->SetLabelMarkup(m_label);
		}
	}

	UpdateWindow(m_static_text);
}

void CValueStaticText::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*                              Property                           *
//*******************************************************************

void CValueStaticText::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["markup"]->SetValue(m_markup);
	m_properties["wrap"]->SetValue(m_wrap);
	m_properties["label"]->SetValue(m_label);
}

void CValueStaticText::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_markup = m_properties["markup"]->GetValueAsInteger();
	m_wrap = m_properties["wrap"]->GetValueAsInteger();
	m_label = m_properties["label"]->GetValueAsString();
}
