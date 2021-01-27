
#include "sizers.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueStaticBoxSizer, CValueSizer)

//****************************************************************************
//*                             StaticBoxSizer                               *
//****************************************************************************

CValueStaticBoxSizer::CValueStaticBoxSizer() : CValueSizer()
{
	PropertyCategory *m_categoryStaticBox = new PropertyCategory("StaticBox");

	m_category->AddProperty("name");
	m_category->AddProperty("orient");
	m_category->AddProperty("label");

	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["orient"] = new PropertyOption("orient", GetOrient(), this);
	m_properties["label"] = new Property("label", PropertyType::PT_WXSTRING, this);

	m_categoryStaticBox->AddProperty("font");
	
	m_categoryStaticBox->AddProperty("fg");
	m_categoryStaticBox->AddProperty("bg");

	m_categoryStaticBox->AddProperty("tooltip");
	m_categoryStaticBox->AddProperty("context_menu");
	m_categoryStaticBox->AddProperty("context_help");
	m_categoryStaticBox->AddProperty("enabled");
	m_categoryStaticBox->AddProperty("visible");

	//static box
	m_properties["font"] = new Property("font", PropertyType::PT_WXFONT, this);

	m_properties["fg"] = new Property("fg", PropertyType::PT_WXCOLOUR, this);
	m_properties["bg"] = new Property("bg", PropertyType::PT_WXCOLOUR, this);

	m_properties["tooltip"] = new Property("tooltip", PropertyType::PT_WXSTRING, this);
	m_properties["context_menu"] = new Property("context_menu", PropertyType::PT_BOOL, this);
	m_properties["context_help"] = new Property("context_help", PropertyType::PT_WXSTRING, this);
	m_properties["enabled"] = new Property("enabled", PropertyType::PT_BOOL, this);
	m_properties["visible"] = new Property("visible", PropertyType::PT_BOOL, this);

	//category 
	m_category->AddCategory(m_categoryStaticBox);
}

wxObject* CValueStaticBoxSizer::Create(wxObject* parent, IVisualHost *visualHost)  
{
	wxStaticBox* m_staticBox = new wxStaticBox((wxWindow *)parent, wxID_ANY, m_label);
	return new wxStaticBoxSizer(m_staticBox, m_orient);
}

#include "forms/visualEditor.h"

void CValueStaticBoxSizer::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	wxStaticBoxSizer *m_staticboxsizer = dynamic_cast<wxStaticBoxSizer *>(wxobject);	
	wxStaticBox* m_wndBox = m_staticboxsizer->GetStaticBox();
	wxASSERT(m_wndBox);
	if (visualHost->IsDesignerHost()) m_wndBox->PushEventHandler(new CDesignerWindow::HighlightPaintHandler(m_wndBox));
}

void CValueStaticBoxSizer::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxStaticBoxSizer *m_staticboxsizer = dynamic_cast<wxStaticBoxSizer *>(wxobject);
	wxStaticBox* m_wndBox = m_staticboxsizer->GetStaticBox();
	wxASSERT(m_wndBox);
	if (m_staticboxsizer)
	{
		m_wndBox->SetLabel(m_label);
		m_wndBox->SetMinSize(m_minimum_size);

		m_wndBox->SetFont(m_font);
		m_wndBox->SetForegroundColour(m_fg);
		m_wndBox->SetBackgroundColour(m_bg);
		m_wndBox->Enable(m_enabled);
		m_wndBox->Show(m_visible);
		m_wndBox->SetToolTip(m_tooltip);

		m_staticboxsizer->SetOrientation(m_orient);
		m_staticboxsizer->SetMinSize(m_minimum_size);

		//after lay out 
		if (m_minimum_size != wxDefaultSize) m_wndBox->Layout();
	}

	UpdateSizer(m_staticboxsizer);
}

void CValueStaticBoxSizer::Cleanup(wxObject* wxobject, IVisualHost *visualHost)
{
	wxStaticBoxSizer *m_staticboxsizer = dynamic_cast<wxStaticBoxSizer *>(wxobject);
	wxStaticBox* m_wndBox = m_staticboxsizer->GetStaticBox();
	wxASSERT(m_wndBox);
	if (visualHost->IsDesignerHost()) m_wndBox->PopEventHandler(true);
}

//**********************************************************************************
//*                           Property                                             *
//**********************************************************************************

void CValueStaticBoxSizer::ReadProperty()
{
	CValueSizer::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["orient"]->SetValue(m_orient);
	m_properties["label"]->SetValue(m_label);

	m_properties["font"]->SetValue(m_font);

	m_properties["fg"]->SetValue(m_fg);
	m_properties["bg"]->SetValue(m_bg);

	m_properties["tooltip"]->SetValue(m_tooltip);
	m_properties["context_menu"]->SetValue(m_context_menu);
	m_properties["context_help"]->SetValue(m_context_help);
	m_properties["enabled"]->SetValue(m_enabled);
	m_properties["visible"]->SetValue(m_visible);
}

void CValueStaticBoxSizer::SaveProperty()
{
	CValueSizer::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_orient = (wxOrientation)m_properties["orient"]->GetValueAsInteger();
	m_label = m_properties["label"]->GetValueAsString();

	m_font = m_properties["font"]->GetValueAsFont();

	m_fg = m_properties["fg"]->GetValueAsColour();
	m_bg = m_properties["bg"]->GetValueAsColour();

	m_tooltip = m_properties["tooltip"]->GetValueAsString();
	m_context_menu = m_properties["context_menu"]->GetValueAsInteger();
	m_context_help = m_properties["context_help"]->GetValueAsString();
	m_enabled = m_properties["enabled"]->GetValueAsInteger();
	m_visible = m_properties["visible"]->GetValueAsInteger();
}