#include "toolbar.h"

//***********************************************************************************
//*                           IMPLEMENT_DYNAMIC_CLASS                               *
//***********************************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CValueToolBarItem, IControlElement);
wxIMPLEMENT_DYNAMIC_CLASS(CValueToolBarSeparator, IControlElement);

//***********************************************************************************
//*                           CValueToolBarItem                               *
//***********************************************************************************

CValueToolBarItem::CValueToolBarItem() : IControlElement(), m_name("toolbar_item"), m_label("newTool")
{
	m_category = new PropertyCategory("ToolBarItem");
	m_category->AddProperty("name");
	m_category->AddProperty("label");
	m_category->AddProperty("bitmap");
	m_category->AddProperty("context_menu");
	m_category->AddProperty("tooltip");
	m_category->AddProperty("enabled");

	m_category->AddProperty("action");

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["label"] = new Property("label", PropertyType::PT_WXSTRING, this);
	m_properties["bitmap"] = new Property("bitmap", PropertyType::PT_BITMAP, this);
	m_properties["context_menu"] = new Property("context_menu", PropertyType::PT_BOOL, this);
	m_properties["tooltip"] = new Property("tooltip", PropertyType::PT_WXSTRING, this);
	m_properties["enabled"] = new Property("enabled", PropertyType::PT_BOOL, this);

	m_properties["action"] = new Property("action", PropertyType::PT_OPTION_CUSTOM, this);
}

void CValueToolBarItem::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	CAuiToolBar *m_auiToolWnd = dynamic_cast<CAuiToolBar *>(visualHost->GetWxObject(GetParent()));
	wxASSERT(m_auiToolWnd);
	
	wxAuiToolBarItem* m_toolItem = m_auiToolWnd->AddTool(GetControlID(),
		GetPropertyAsString(_("label")),
		GetPropertyAsBitmap(_("bitmap")),
		wxNullBitmap,
		wxItemKind::wxITEM_NORMAL,
		GetPropertyAsString(_("help")),
		wxEmptyString,
		wxobject
	);

#if defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(_WIN64)
	m_toolItem->SetUserData((long long)wxobject);
#else 
	m_toolItem->SetUserData((long)wxobject);
#endif 

	m_auiToolWnd->EnableTool(m_toolItem->GetId(), GetPropertyAsInteger("enabled"));

	if (GetPropertyAsInteger(_("context_menu")) == 1 && !m_toolItem->HasDropDown())
		m_auiToolWnd->SetToolDropDown(m_toolItem->GetId(), true);
	else if (GetPropertyAsInteger(_("context_menu")) == 0 && m_toolItem->HasDropDown())
		m_auiToolWnd->SetToolDropDown(m_toolItem->GetId(), false);

	m_auiToolWnd->Realize();
	m_auiToolWnd->Layout();
}

void CValueToolBarItem::OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	CAuiToolBar *m_auiToolWnd = dynamic_cast<CAuiToolBar *>(visualHost->GetWxObject(GetParent()));
	wxASSERT(m_auiToolWnd);

	wxAuiToolBarItem *m_toolItem = m_auiToolWnd->FindTool(GetControlID());
	IControlElement *m_parentControl = GetParent(); int idx = wxNOT_FOUND;

	for (unsigned int i = 0; i < m_parentControl->GetChildCount(); i++)
	{
		IControlElement *child = m_parentControl->GetChild(i);
		if (m_obj_id == child->GetControlID()) { idx = i; break; }
	}

	if (m_toolItem) { m_auiToolWnd->DeleteTool(GetControlID()); }

	m_toolItem = m_auiToolWnd->InsertTool(idx, GetControlID(),
		GetPropertyAsString(_("label")),
		GetPropertyAsBitmap(_("bitmap")),
		wxNullBitmap,
		wxItemKind::wxITEM_NORMAL,
		GetPropertyAsString(_("help")),
		wxEmptyString,
		wxobject
	);

#if defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(_WIN64)
	m_toolItem->SetUserData((long long)wxobject);
#else 
	m_toolItem->SetUserData((long)wxobject);
#endif 

	m_auiToolWnd->EnableTool(m_toolItem->GetId(), GetPropertyAsInteger("enabled"));

	if (GetPropertyAsInteger(_("context_menu")) == 1 && !m_toolItem->HasDropDown())
		m_auiToolWnd->SetToolDropDown(m_toolItem->GetId(), true);
	else if (GetPropertyAsInteger(_("context_menu")) == 0 && m_toolItem->HasDropDown())
		m_auiToolWnd->SetToolDropDown(m_toolItem->GetId(), false);

	m_auiToolWnd->Realize();
	m_auiToolWnd->Layout(); 
}

void CValueToolBarItem::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
	CAuiToolBar *m_auiToolWnd = dynamic_cast<CAuiToolBar *>(visualHost->GetWxObject(GetParent()));
	
	m_auiToolWnd->DeleteTool(GetControlID());
	
	m_auiToolWnd->Realize();
	m_auiToolWnd->Layout();
}

//***********************************************************************************
//*                           CValueToolBarSeparator                                *
//***********************************************************************************

CValueToolBarSeparator::CValueToolBarSeparator() : IControlElement(), m_name("toolbar_separator")
{
	m_category = new PropertyCategory("ToolBarItem");
	m_category->AddProperty("name");

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
}

void CValueToolBarSeparator::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	CAuiToolBar *m_auiToolWnd = dynamic_cast<CAuiToolBar *>(visualHost->GetWxObject(GetParent()));
	wxASSERT(m_auiToolWnd);
	wxAuiToolBarItem *m_toolItem = m_auiToolWnd->AddSeparator();
	m_toolItem->SetId(GetControlID());
	m_auiToolWnd->Realize();
}

void CValueToolBarSeparator::OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	CAuiToolBar *m_auiToolWnd = dynamic_cast<CAuiToolBar *>(visualHost->GetWxObject(GetParent()));
	wxASSERT(m_auiToolWnd);

	wxAuiToolBarItem *m_toolItem = m_auiToolWnd->FindTool(GetControlID());
	IControlElement *m_parentControl = GetParent(); int idx = wxNOT_FOUND;

	for (unsigned int i = 0; i < m_parentControl->GetChildCount(); i++)
	{
		IControlElement *child = m_parentControl->GetChild(i);
		if (m_obj_id == child->GetControlID()) { idx = i; break; }
	}

	if (m_toolItem) { m_auiToolWnd->DeleteTool(GetControlID()); }
	m_auiToolWnd->InsertSeparator(idx, GetControlID());

	m_auiToolWnd->Realize();
}

void CValueToolBarSeparator::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
	CAuiToolBar *m_auiToolWnd = dynamic_cast<CAuiToolBar *>(visualHost->GetWxObject(GetParent()));
	if (m_auiToolWnd) m_auiToolWnd->DeleteTool(GetControlID());
}

//**********************************************************************************
//*                                  Property                                      *
//**********************************************************************************

void CValueToolBarItem::ReadProperty()
{
	m_properties["name"]->SetValue(m_name);
	m_properties["label"]->SetValue(m_label);
	//m_properties["bitmap"]->SetValue(m_bitmap);
	m_properties["context_menu"]->SetValue(m_context_menu);
	m_properties["tooltip"]->SetValue(m_tooltip);
	m_properties["enabled"]->SetValue(m_enabled);
}

void CValueToolBarSeparator::ReadProperty()
{
	m_properties["name"]->SetValue(m_name);
}

void CValueToolBarItem::SaveProperty()
{
	m_name = m_properties["name"]->GetValueAsString();
	m_label = m_properties["label"]->GetValueAsString();
	m_bitmap = m_properties["bitmap"]->GetValueAsBitmap();
	m_context_menu = m_properties["name"]->GetValueAsInteger();
	m_tooltip = m_properties["tooltip"]->GetValueAsString();
	m_enabled = m_properties["enabled"]->GetValueAsInteger();
}

void CValueToolBarSeparator::SaveProperty()
{
	m_name = m_properties["name"]->GetValueAsString();
}
