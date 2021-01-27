
#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueGauge, CValueWindow)

//****************************************************************************
//*                             Gauge                                        *
//****************************************************************************

CValueGauge::CValueGauge() : CValueWindow(), m_range(100), m_value(30)
{
	PropertyCategory *m_categoryButton = new PropertyCategory("Gauge");

	//property
	m_categoryButton->AddProperty("name");
	m_categoryButton->AddProperty("range");
	m_categoryButton->AddProperty("value");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["range"] = new Property("range", PropertyType::PT_INT, this);
	m_properties["value"] = new Property("value", PropertyType::PT_INT, this);
}

wxObject* CValueGauge::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxGauge *m_gauge = new wxGauge((wxWindow *)parent, wxID_ANY,
		m_range,
		m_pos,
		m_size,
		m_style | m_window_style);

	return m_gauge;
}

void CValueGauge::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueGauge::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxGauge *m_gauge = dynamic_cast<wxGauge *>(wxobject);

	if (m_gauge)
	{
		m_gauge->SetRange(m_range);
		m_gauge->SetValue(m_value);
	}

	UpdateWindow(m_gauge);
}

void CValueGauge::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*            Property                                             *
//*******************************************************************

void CValueGauge::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["range"]->SetValue(m_range);
	m_properties["value"]->SetValue(m_value);
}

void CValueGauge::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_range = m_properties["range"]->GetValueAsInteger();
	m_value = m_properties["value"]->GetValueAsInteger();
}