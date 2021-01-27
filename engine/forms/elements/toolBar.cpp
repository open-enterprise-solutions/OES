#include "toolbar.h"
#include "forms/visualEditor.h"
#include "forms/visualEditorView.h"

//***********************************************************************************
//*                           IMPLEMENT_DYNAMIC_CLASS                               *
//***********************************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CValueToolbar, CValueWindow);

//***********************************************************************************
//*                                  Custom Aui toolbar                             *
//***********************************************************************************

void CValueToolbar::AddToolItem()
{
	IControlElement *m_toolItem = new CValueToolBarItem();
	m_toolItem->ReadProperty();
	
	m_visualHostContext->InsertObject(m_toolItem, this);
	m_visualHostContext->ResolveSubtreeNameConflicts(m_toolItem, this);
	
	m_toolItem->SaveProperty();
	m_toolItem->IncrRef();

	m_visualHostContext->RefreshEditor();
}

void CValueToolbar::AddToolSeparator()
{
	IControlElement *m_toolSeparator = new CValueToolBarSeparator();
	m_toolSeparator->ReadProperty();
	
	m_visualHostContext->InsertObject(m_toolSeparator, this);
	m_visualHostContext->ResolveSubtreeNameConflicts(m_toolSeparator, this);
	
	m_toolSeparator->SaveProperty();
	m_toolSeparator->IncrRef();

	m_visualHostContext->RefreshEditor();
}

//***********************************************************************************
//*                                 Value Toolbar                                   *
//***********************************************************************************

CValueToolbar::CValueToolbar() : CValueWindow(), m_name("toolbar"),
m_bitmapsize(wxDefaultSize), m_margins(wxDefaultSize),
m_packing(1), m_separation(5), m_actionSource(wxNOT_FOUND)
{
	PropertyCategory *m_categoryToolbar = new PropertyCategory("ToolBar");
	m_categoryToolbar->AddProperty("name");
	m_categoryToolbar->AddProperty("bitmapsize");
	m_categoryToolbar->AddProperty("margins");
	m_categoryToolbar->AddProperty("packing");
	m_categoryToolbar->AddProperty("separation");

	m_category->AddCategory(m_categoryToolbar);

	PropertyCategory *m_categoryAction = new PropertyCategory("Action");
	m_categoryAction->AddProperty("action_source");

	m_category->AddCategory(m_categoryAction);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["bitmapsize"] = new Property("bitmapsize", PropertyType::PT_WXSIZE, this);
	m_properties["margins"] = new Property("margins", PropertyType::PT_WXSIZE, this);
	m_properties["packing"] = new Property("packing", PropertyType::PT_INT, this);
	m_properties["separation"] = new Property("separation", PropertyType::PT_INT, this);
	
	m_properties["action_source"] = new Property("action_source", PropertyType::PT_OPTION_CUSTOM, this);
}

wxObject* CValueToolbar::Create(wxObject* parent, IVisualHost *visualHost)
{
	CAuiToolBar *m_toolbar = new CAuiToolBar((wxWindow*)parent, wxID_ANY, m_pos, m_size, wxAUI_TB_HORZ_TEXT);

	if (m_bitmapsize != wxDefaultSize) { m_toolbar->SetToolBitmapSize(m_bitmapsize); }
	if (m_margins != wxDefaultSize) { m_toolbar->SetMargins(m_margins.GetWidth(), m_margins.GetHeight()); }

	if (!IsNull(wxT("packing"))) m_toolbar->SetToolPacking(m_packing);
	if (!IsNull(wxT("separation"))) m_toolbar->SetToolSeparation(m_separation);

	m_toolbar->Bind(wxEVT_TOOL, &CValueToolbar::OnTool, this);

	return m_toolbar;
}

void CValueToolbar::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueToolbar::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	CAuiToolBar *m_toolbar = dynamic_cast<CAuiToolBar *>(wxobject);

	if (m_toolbar)
	{
	}

	UpdateWindow(m_toolbar);
}

void CValueToolbar::OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueToolbar::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
	CAuiToolBar *m_toolbar = dynamic_cast<CAuiToolBar *>(obj);
	m_toolbar->Unbind(wxEVT_TOOL, &CValueToolbar::OnTool, this);
}

//**********************************************************************************
//*                           Property                                             *
//**********************************************************************************

void CValueToolbar::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["bitmapsize"]->SetValue(m_bitmapsize);
	m_properties["margins"]->SetValue(m_margins);
	m_properties["packing"]->SetValue(m_packing);
	m_properties["separation"]->SetValue(m_separation);

	m_properties["action_source"]->SetValue(m_actionSource);
}

void CValueToolbar::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_bitmapsize = m_properties["bitmapsize"]->GetValueAsSize();
	m_margins = m_properties["margins"]->GetValueAsSize();
	m_packing = m_properties["packing"]->GetValueAsInteger();
	m_separation = m_properties["separation"]->GetValueAsInteger();

	m_actionSource = m_properties["action_source"]->GetValueAsInteger();
}