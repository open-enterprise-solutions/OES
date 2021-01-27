////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : window object
////////////////////////////////////////////////////////////////////////////

#include "window.h"
#include "forms/visualEditorBase.h"

wxIMPLEMENT_ABSTRACT_CLASS(CValueWindow, IControlElement)

//***********************************************************************************
//*                                    ValueWindow                                  *
//***********************************************************************************

CValueWindow::CValueWindow() : IControlElement()
{
	PropertyCategory *m_categoryBase = new PropertyCategory("Window");

	//m_categoryBase->AddProperty("id");
	//m_categoryBase->AddProperty("pos");
	//m_categoryBase->AddProperty("size");
	m_categoryBase->AddProperty("minimum_size");
	m_categoryBase->AddProperty("maximum_size");
	m_categoryBase->AddProperty("font");
	m_categoryBase->AddProperty("fg");
	m_categoryBase->AddProperty("bg");
	//m_categoryBase->AddProperty("window_name");


	m_categoryBase->AddProperty("tooltip");
	m_categoryBase->AddProperty("context_menu");
	m_categoryBase->AddProperty("context_help");
	m_categoryBase->AddProperty("enabled");
	m_categoryBase->AddProperty("visible");


	//m_properties["id"] = new Property("id", PropertyType::PT_WXSTRING, this);
	//m_properties["pos"] = new Property("pos", PropertyType::PT_WXPOINT, this);
	//m_properties["size"] = new Property("size", PropertyType::PT_WXSIZE, this);
	m_properties["minimum_size"] = new Property("minimum_size", PropertyType::PT_WXSIZE, this);
	m_properties["maximum_size"] = new Property("maximum_size", PropertyType::PT_WXSIZE, this);
	m_properties["font"] = new Property("font", PropertyType::PT_WXFONT, this);
	m_properties["fg"] = new Property("fg", PropertyType::PT_WXCOLOUR, this);
	m_properties["bg"] = new Property("bg", PropertyType::PT_WXCOLOUR, this);
	//m_properties["window_name"] = new Property("window_name", PropertyType::PT_WXSTRING, this);


	m_properties["tooltip"] = new Property("tooltip", PropertyType::PT_WXSTRING, this);
	m_properties["context_menu"] = new Property("context_menu", PropertyType::PT_BOOL, this);
	m_properties["context_help"] = new Property("context_help", PropertyType::PT_WXSTRING, this);
	m_properties["enabled"] = new Property("enabled", PropertyType::PT_BOOL, this);
	m_properties["visible"] = new Property("visible", PropertyType::PT_BOOL, this);

	m_category = new PropertyCategory("wndProperty");
	m_category->AddCategory(m_categoryBase);
}

//***********************************************************************************
//*                                  Update                                       *
//***********************************************************************************

void CValueWindow::UpdateWindow(wxWindow* window)
{
	// All of the properties of the wxWindow object are applied in this function
	if (!window) return;
	// Minimum size
	if (m_minimum_size != wxDefaultSize) window->SetMinSize(m_minimum_size);
	// Maximum size
	if (m_maximum_size != wxDefaultSize) window->SetMaxSize(m_maximum_size);
	// Font
	if (m_font.IsOk()) window->SetFont(m_font);
	// Background
	if (m_bg.IsOk()) window->SetBackgroundColour(m_bg);
	// Foreground
	if (m_fg.IsOk()) window->SetForegroundColour(m_fg);
	// Extra Style
	if (m_window_extra_style) window->SetExtraStyle(m_window_extra_style);
	// Enabled
	window->Enable(m_enabled);
	// Hidden
	window->Show(m_visible);
	// Tooltip
	window->SetToolTip(m_tooltip);
	//after lay out 
	if (m_minimum_size != wxDefaultSize || m_maximum_size != wxDefaultSize) window->Layout();
}

//***********************************************************************************
//*                                  Attribute                                      *
//***********************************************************************************

void CValueWindow::SetAttribute(CAttributeParameters &aParams, CValue &cVal)
{
	IControlElement::SetAttribute(aParams, cVal);
}

//***********************************************************************************
//*                                  Property                                       *
//***********************************************************************************

void CValueWindow::ReadProperty()
{
	IControlElement::ReadProperty();

	//m_properties["id"]->SetValue(m_id);
	//m_properties["pos"]->SetValue(m_pos);
	//m_properties["size"]->SetValue(m_size);
	m_properties["minimum_size"]->SetValue(m_minimum_size);
	m_properties["maximum_size"]->SetValue(m_maximum_size);
	m_properties["font"]->SetValue(m_font);
	m_properties["fg"]->SetValue(m_fg);
	m_properties["bg"]->SetValue(m_bg);
	//m_properties["window_name"]->SetValue(m_window_name);

	m_properties["tooltip"]->SetValue(m_tooltip);
	m_properties["context_menu"]->SetValue(m_context_menu);
	m_properties["context_help"]->SetValue(m_context_help);
	m_properties["enabled"]->SetValue(m_enabled);
	m_properties["visible"]->SetValue(m_visible);

	//if we have sizerItem then call him readpropery 
	IControlElement *m_sizeritem = GetParent();
	if (m_sizeritem &&
		m_sizeritem->GetClassName() == wxT("sizerItem"))
	{
		m_sizeritem->ReadProperty();
	}
}

void CValueWindow::SaveProperty()
{
	IControlElement::SaveProperty();

	//m_id = m_properties["id"]->GetValueAsString();
	//m_pos = m_properties["pos"]->GetValueAsPoint();
	//m_size = m_properties["size"]->GetValueAsSize();
	m_minimum_size = m_properties["minimum_size"]->GetValueAsSize();
	m_maximum_size = m_properties["maximum_size"]->GetValueAsSize();
	m_font = m_properties["font"]->GetValueAsFont();
	m_fg = m_properties["fg"]->GetValueAsColour();
	m_bg = m_properties["bg"]->GetValueAsColour();
	//m_window_name = m_properties["window_name"]->GetValueAsString();

	m_tooltip = m_properties["tooltip"]->GetValueAsString();
	m_context_menu = m_properties["context_menu"]->GetValueAsInteger();
	m_context_help = m_properties["context_help"]->GetValueAsString();
	m_enabled = m_properties["enabled"]->GetValueAsInteger();
	m_visible = m_properties["visible"]->GetValueAsInteger();

	//if we have sizerItem then call him savepropery 
	IControlElement *m_sizeritem = GetParent();
	if (m_sizeritem &&
		m_sizeritem->GetClassName() == wxT("sizerItem"))
	{
		m_sizeritem->SaveProperty();
	}
}