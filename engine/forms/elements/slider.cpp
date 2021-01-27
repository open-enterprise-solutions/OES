
#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueSlider, CValueWindow)

//****************************************************************************
//*                             Slider                                       *
//****************************************************************************

CValueSlider::CValueSlider() : CValueWindow()
{
	PropertyCategory *m_categoryButton = new PropertyCategory("Slider");

	//property
	m_categoryButton->AddProperty("name");
	m_categoryButton->AddProperty("minvalue");
	m_categoryButton->AddProperty("maxvalue");
	m_categoryButton->AddProperty("value");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["minvalue"] = new Property("minvalue", PropertyType::PT_INT, this);
	m_properties["maxvalue"] = new Property("maxvalue", PropertyType::PT_INT, this);
	m_properties["value"] = new Property("value", PropertyType::PT_WXSTRING, this);
}

wxObject* CValueSlider::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxSlider *m_slider = new wxSlider((wxWindow *)parent, wxID_ANY,
		m_value,
		m_minValue,
		m_maxValue,
		m_pos,
		m_size,
		m_style | m_window_style | m_window_style);

	return m_slider;
}

void CValueSlider::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueSlider::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxSlider *m_slider = dynamic_cast<wxSlider *>(wxobject);

	if (m_slider)
	{
		m_slider->SetMin(m_minValue);
		m_slider->SetMax(m_maxValue);
		m_slider->SetValue(m_value);
	}

	UpdateWindow(m_slider);
}


void CValueSlider::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*                           Property                              *
//*******************************************************************

void CValueSlider::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["minvalue"]->SetValue(m_minValue);
	m_properties["maxvalue"]->SetValue(m_maxValue);
	m_properties["value"]->SetValue(m_value);
}

void CValueSlider::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_minValue = m_properties["minvalue"]->GetValueAsInteger();
	m_maxValue = m_properties["maxvalue"]->GetValueAsInteger();
	m_value = m_properties["value"]->GetValueAsInteger();
}