
#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueStaticLine, CValueWindow)

//****************************************************************************
//*                             StaticLine                                   *
//****************************************************************************

CValueStaticLine::CValueStaticLine() : CValueWindow()
{
	PropertyCategory *m_categoryButton = new PropertyCategory("StaticLine");

	//property
	m_categoryButton->AddProperty("name");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
}

wxObject* CValueStaticLine::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxStaticLine *m_staticline = new wxStaticLine((wxWindow *)parent, wxID_ANY,
		m_pos,
		m_size,
		m_style | m_window_style);

	return m_staticline;
}

void CValueStaticLine::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueStaticLine::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxStaticLine *m_staticline = dynamic_cast<wxStaticLine *>(wxobject);

	if (m_staticline)
	{
	}

	UpdateWindow(m_staticline);
}

void CValueStaticLine::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*                             Property                            *
//*******************************************************************

void CValueStaticLine::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
}

void CValueStaticLine::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
}