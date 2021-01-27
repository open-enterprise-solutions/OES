#include "widgets.h"
#include "compiler/procUnit.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueButton, CValueWindow)

//****************************************************************************
//*                              Button                                      *
//****************************************************************************

CValueButton::CValueButton() : CValueWindow()
{
	PropertyCategory *m_categoryButton = new PropertyCategory("Button");

	//property
	m_categoryButton->AddProperty("name");
	m_categoryButton->AddProperty("markup");
	m_categoryButton->AddProperty("default");
	m_categoryButton->AddProperty("label");

	//category 
	m_category->AddCategory(m_categoryButton);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["markup"] = new Property("markup", PropertyType::PT_BOOL, this);
	m_properties["default"] = new Property("default", PropertyType::PT_BOOL, this);
	m_properties["label"] = new Property("label", PropertyType::PT_WXSTRING, this);

	//event
	m_categoryButton->AddEvent("OnButtonPressed");

	m_events["OnButtonPressed"] = new Event("OnButtonPressed", "", this);
}

wxObject* CValueButton::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxButton *m_button = new wxButton((wxWindow*)parent, wxID_ANY,
		m_label,
		m_pos,
		m_size,
		m_style | m_window_style);

	//setup event 
	m_button->Bind(wxEVT_BUTTON, &CValueButton::OnButtonPressed, this);

	return m_button;
}

void CValueButton::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueButton::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxButton *m_button = dynamic_cast<wxButton *>(wxobject);

	if (m_button)
	{
		m_button->SetLabel(m_label);

		if (m_markup != false) m_button->SetLabelMarkup(m_label);

		if (m_default) m_button->SetDefault();

		if (!IsNull(_("bitmap"))) {
			m_button->SetBitmap(GetPropertyAsBitmap(_("bitmap")));
		}

		if (!IsNull(_("disabled"))) {
			m_button->SetBitmapDisabled(GetPropertyAsBitmap(_("disabled")));
		}

		if (!IsNull(_("pressed"))) {
			m_button->SetBitmapPressed(GetPropertyAsBitmap(_("pressed")));
		}

		if (!IsNull(_("focus"))) {
			m_button->SetBitmapFocus(GetPropertyAsBitmap(_("focus")));
		}

		if (!IsNull(_("current"))) {
			m_button->SetBitmapCurrent(GetPropertyAsBitmap(_("current")));
		}

		if (!IsNull(_("position"))) {
			m_button->SetBitmapPosition(
				static_cast<wxDirection>(GetPropertyAsInteger(_("position"))));
		}

		if (!IsNull(_("margins"))) {
			m_button->SetBitmapMargins(m_margins);
		}
	}

	UpdateWindow(m_button);
}

void CValueButton::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
}

//*******************************************************************
//*                           Property                              *
//*******************************************************************

void CValueButton::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["markup"]->SetValue(m_markup);
	m_properties["default"]->SetValue(m_default);
	m_properties["label"]->SetValue(m_label);
}

void CValueButton::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_markup = m_properties["markup"]->GetValueAsInteger();
	m_default = m_properties["default"]->GetValueAsInteger();
	m_label = m_properties["label"]->GetValueAsString();
}