#include "sizers.h"
#include "forms/pageWindow.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueSizerItem, CValueSizer)

//************************************************************************************
//*                            Support item                                          *
//************************************************************************************

wxObject* GetParentFormVisualEditor(IVisualHost* visualEdit, wxObject* wxobject)
{
	IControlElement *obj = visualEdit->GetObjectBase(wxobject);
	IControlElement *objParent = obj->GetParent();

	wxASSERT(objParent);

	wxObject *objItem = visualEdit->GetWxObject(objParent);

	if (objParent->GetClassName() == wxT("page"))
	{
		CPageWindow *objPage = dynamic_cast<CPageWindow *>(objItem);
		wxASSERT(objPage);
		return objPage->GetSizer();
	}
	else
	{
		return objItem;
	}
}

wxObject* GetChildFormVisualEditor(IVisualHost* m_visualEdit, wxObject* wxobject, unsigned int childIndex)
{
	IControlElement * obj = m_visualEdit->GetObjectBase(wxobject);
	if (childIndex >= obj->GetChildCount()) return NULL;
	return m_visualEdit->GetWxObject(obj->GetChild(childIndex));
}

OptionList *CValueSizerItem::GetDefaultOptionBorder()
{
	OptionList *m_opt_list = new OptionList();

	m_opt_list->AddOption("left", wxLEFT);
	m_opt_list->AddOption("right", wxRIGHT);
	m_opt_list->AddOption("bottom", wxBOTTOM);
	m_opt_list->AddOption("top", wxTOP);

	return m_opt_list;
}

OptionList *CValueSizerItem::GetDefaultOptionState()
{
	OptionList *m_opt_list = new OptionList();

	m_opt_list->AddOption("shrink", wxSHRINK);
	m_opt_list->AddOption("expand", wxEXPAND);
	//m_opt_list->AddOption("shaped", wxSHAPED);

	return m_opt_list;
}

//************************************************************************************
//*                            ValueSizerItem                                        *
//************************************************************************************

CValueSizerItem::CValueSizerItem() : CValueSizer(),
m_proportion(0), m_flag_border(wxALL), m_border(5), m_flag_state(wxSHRINK)
{
	m_properties.clear(); delete m_category;
	m_category = new PropertyCategory("Property");

	PropertyCategory *m_categorysizer = new PropertyCategory("SizerItem");
	m_categorysizer->AddProperty("border");
	m_categorysizer->AddProperty("proportion");
	m_categorysizer->AddProperty("flag_border");
	m_categorysizer->AddProperty("flag_state");

	m_category->AddCategory(m_categorysizer);

	m_properties["border"] = new Property("border", PropertyType::PT_INT, this);
	m_properties["proportion"] = new Property("proportion", PropertyType::PT_INT, this);
	m_properties["flag_border"] = new PropertyBitlist("flag_border", GetDefaultOptionBorder(), this);
	m_properties["flag_state"] = new PropertyOption("flag_state", GetDefaultOptionState(), this);
}

void CValueSizerItem::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	// Get parent sizer
	wxObject* parent = GetParentFormVisualEditor(visualHost, wxobject);
	wxSizer* sizer = wxDynamicCast(parent, wxSizer);

	// Get child window
	wxObject* child = GetChildFormVisualEditor(visualHost, wxobject, 0);
	if (NULL == child)
	{
		wxLogError(wxT("The SizerItem component has no child - this should not be possible!"));
		return;
	}

	// Get IObject for property access
	CValueSizerItem* obj = wxDynamicCast(visualHost->GetObjectBase(wxobject), CValueSizerItem);
	IControlElement* childObj = visualHost->GetObjectBase(child);

	// Add the spacer
	if (childObj->GetClassName() == _("spacer"))
	{
		sizer->Add(childObj->GetPropertyAsInteger(_("width")),
			childObj->GetPropertyAsInteger(_("height")),
			obj->m_proportion,
			obj->m_flag_border | obj->m_flag_state,
			obj->m_border
		);
		return;
	}

	// Add the child ( window or sizer ) to the sizer
	wxWindow* windowChild = wxDynamicCast(child, wxWindow);
	wxSizer* sizerChild = wxDynamicCast(child, wxSizer);

	if (windowChild != NULL)
	{
		sizer->Add(windowChild,
			obj->m_proportion,
			obj->m_flag_border | obj->m_flag_state,
			obj->m_border);
	}
	else if (sizerChild != NULL)
	{
		sizer->Add(sizerChild,
			obj->m_proportion,
			obj->m_flag_border | obj->m_flag_state,
			obj->m_border);
	}
}

void CValueSizerItem::OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	// Get parent sizer
	wxObject* parent = GetParentFormVisualEditor(visualHost, wxobject);
	wxSizer* sizer = wxDynamicCast(parent, wxSizer);

	// Get child window
	wxObject* child = GetChildFormVisualEditor(visualHost, wxobject, 0);
	if (NULL == child)
	{
		wxLogError(wxT("The SizerItem component has no child - this should not be possible!"));
		return;
	}

	// Get IObject for property access
	CValueSizerItem* obj = wxDynamicCast(visualHost->GetObjectBase(wxobject), CValueSizerItem);
	IControlElement* childObj = visualHost->GetObjectBase(child);

	// Add the spacer
	if (childObj->GetClassName() == _("spacer"))
	{
		sizer->Detach(sizer->GetItemCount());
	}

	if (childObj->GetClassName() == _("spacer"))
	{
		sizer->Add(childObj->GetPropertyAsInteger(_("width")),
			childObj->GetPropertyAsInteger(_("height")),
			obj->m_proportion,
			obj->m_flag_border | obj->m_flag_state,
			obj->m_border
		);
		return;
	}

	// Add the child ( window or sizer ) to the sizer
	wxWindow* windowChild = wxDynamicCast(child, wxWindow);
	wxSizer* sizerChild = wxDynamicCast(child, wxSizer);

	if (windowChild != NULL)
	{
		sizer->Detach(windowChild);
	}
	else if (sizerChild != NULL)
	{
		sizer->Detach(sizerChild);
	}

	IControlElement *m_parentControl = GetParent(); int idx = wxNOT_FOUND;

	for (unsigned int i = 0; i < m_parentControl->GetChildCount(); i++)
	{
		IControlElement *child = m_parentControl->GetChild(i);
		if (m_obj_id == child->GetControlID()) { idx = i; break; }
	}

	if (windowChild != NULL)
	{
		if (idx == wxNOT_FOUND)
		{
			sizer->Add(windowChild,
				obj->m_proportion,
				obj->m_flag_border | obj->m_flag_state,
				obj->m_border);
		}
		else
		{
			sizer->Insert(idx, windowChild,
				obj->m_proportion,
				obj->m_flag_border | obj->m_flag_state,
				obj->m_border);
		}
	}
	else if (sizerChild != NULL)
	{
		if (idx == wxNOT_FOUND)
		{
			sizer->Add(sizerChild,
				obj->m_proportion,
				obj->m_flag_border | obj->m_flag_state,
				obj->m_border);
		}
		else
		{
			sizer->Insert(idx, sizerChild,
				obj->m_proportion,
				obj->m_flag_border | obj->m_flag_state,
				obj->m_border);
		}
	}
}

//**********************************************************************************
//*                                    Property                                    *
//**********************************************************************************

void CValueSizerItem::ReadProperty()
{
	m_properties["border"]->SetValue(m_border);
	m_properties["proportion"]->SetValue(m_proportion);
	m_properties["flag_border"]->SetValue(m_flag_border);
	m_properties["flag_state"]->SetValue(m_flag_state);
}

void CValueSizerItem::SaveProperty()
{
	m_border = m_properties["border"]->GetValueAsInteger();
	m_proportion = m_properties["proportion"]->GetValueAsInteger();
	m_flag_border = m_properties["flag_border"]->GetValueAsInteger();
	m_flag_state = m_properties["flag_state"]->GetValueAsInteger();
}
