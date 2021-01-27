#include "tableBox.h"

//***********************************************************************************
//*                           IMPLEMENT_DYNAMIC_CLASS                               *
//***********************************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CValueTableBoxColumn, IControlElement);

//***********************************************************************************
//*                            CValueTableBoxColumn                                 *
//***********************************************************************************

CValueTableBoxColumn::CValueTableBoxColumn() : IControlElement(),
m_markup(true),
m_name("tablebox_column"),
m_title("newColumn"),
m_type(eValueTypes::TYPE_STRING),
m_width(wxDVC_DEFAULT_WIDTH),
m_align(wxALIGN_LEFT),

m_model_id(wxNOT_FOUND),

m_visible(true),
m_resizable(true),
m_sortable(false),
m_reorderable(true)
{
	m_category = new PropertyCategory("TableBoxColumn");

	PropertyCategory *m_categoryInfo = new PropertyCategory("Info");
	m_categoryInfo->AddProperty("name");
	m_categoryInfo->AddProperty("title");
	m_category->AddCategory(m_categoryInfo);

	PropertyCategory *m_categoryData = new PropertyCategory("Data");
	m_categoryData->AddProperty("source");
	m_categoryData->AddProperty("type");
	m_category->AddCategory(m_categoryData);

	PropertyCategory *m_categoryStyle = new PropertyCategory("Style");
	m_categoryStyle->AddProperty("width");
	m_categoryStyle->AddProperty("align");
	m_categoryStyle->AddProperty("icon");
	m_categoryStyle->AddProperty("visible");
	m_categoryStyle->AddProperty("resizable");
	m_categoryStyle->AddProperty("sortable");
	m_categoryStyle->AddProperty("reorderable");
	m_category->AddCategory(m_categoryStyle);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["title"] = new Property("title", PropertyType::PT_WXSTRING, this);

	m_properties["source"] = new Property("source", PropertyType::PT_SOURCE, this);
	m_properties["type"] = new Property("type", PropertyType::PT_TYPE_SELECT, this);

	m_properties["width"] = new Property("width", PropertyType::PT_UINT, this);
	m_properties["align"] = new PropertyOption("align", GetAlign(), this);
	m_properties["icon"] = new Property("icon", PropertyType::PT_BITMAP, this);
	m_properties["visible"] = new Property("visible", PropertyType::PT_BOOL, this);
	m_properties["resizable"] = new Property("resizable", PropertyType::PT_BOOL, this);
	m_properties["sortable"] = new Property("sortable", PropertyType::PT_BOOL, this);
	m_properties["reorderable"] = new Property("reorderable", PropertyType::PT_BOOL, this);
}

wxObject* CValueTableBoxColumn::Create(wxObject* parent, IVisualHost *visualHost)
{
	CDataViewColumnObject *m_newColumn = new CDataViewColumnObject(m_title, m_model_id == wxNOT_FOUND ? m_obj_id : m_model_id, m_width, (wxAlignment)m_align, wxDATAVIEW_COL_REORDERABLE);

	m_newColumn->SetTitle(m_title);
	m_newColumn->SetWidth(m_width);
	m_newColumn->SetAlignment((wxAlignment)m_align);

	m_newColumn->SetBitmap(m_icon);
	m_newColumn->SetHidden(!m_visible);
	m_newColumn->SetSortable(m_sortable);
	m_newColumn->SetResizeable(m_resizable);

	m_newColumn->SetControlID(m_obj_id);

	CValueViewRenderer *m_colRenderer = m_newColumn->GetRenderer();
	wxASSERT(m_colRenderer);

	//switch (m_type)
	//{
	//case eValueTypes::TYPE_BOOLEAN: m_colRenderer->SetRendererType(eRendererType::enToggle); break;
	//case eValueTypes::TYPE_NUMBER: m_colRenderer->SetRendererType(eRendererType::enText); break;
	//case eValueTypes::TYPE_DATE: m_colRenderer->SetRendererType(eRendererType::enDate); break;
	//case eValueTypes::TYPE_STRING: m_colRenderer->SetRendererType(eRendererType::enText); break;
	//default: m_colRenderer->SetRendererType(eRendererType::enText);
	//}

	return m_newColumn;
}

void CValueTableBoxColumn::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	wxDataViewCtrl *m_tableCtrl = dynamic_cast<wxDataViewCtrl *>(wxparent);
	wxASSERT(m_tableCtrl);
	CDataViewColumnObject *m_columnObject = dynamic_cast<CDataViewColumnObject *>(wxobject);
	wxASSERT(m_columnObject);
	m_tableCtrl->AppendColumn(m_columnObject);
}

void CValueTableBoxColumn::OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	IControlElement *m_parentControl = GetParent(); int idx = wxNOT_FOUND;

	for (unsigned int i = 0; i < m_parentControl->GetChildCount(); i++)
	{
		CValueTableBoxColumn *child = dynamic_cast<CValueTableBoxColumn *>(m_parentControl->GetChild(i));
		wxASSERT(child);
		if (m_model_id != wxNOT_FOUND && m_model_id == child->m_model_id) { idx = i; break; }
		else if (m_model_id == wxNOT_FOUND && m_obj_id == child->m_obj_id) { idx = i; break; }
	}

	wxDataViewCtrl *m_tableCtrl = dynamic_cast<wxDataViewCtrl *>(wxparent);
	wxASSERT(m_tableCtrl);
	CDataViewColumnObject *m_columnObject = dynamic_cast<CDataViewColumnObject *>(wxobject);
	wxASSERT(m_columnObject);

	m_columnObject->SetTitle(m_title);
	m_columnObject->SetWidth(m_width);
	m_columnObject->SetAlignment((wxAlignment)m_align);

	m_columnObject->SetBitmap(m_icon);
	m_columnObject->SetHidden(!m_visible);
	m_columnObject->SetSortable(m_sortable);
	m_columnObject->SetResizeable(m_resizable);

	m_columnObject->SetColModel(m_model_id == wxNOT_FOUND ? m_obj_id : m_model_id);

	CValueViewRenderer *m_colRenderer = m_columnObject->GetRenderer();
	wxASSERT(m_colRenderer);

	//switch (m_type)
	//{
	//case eValueTypes::TYPE_BOOLEAN: m_colRenderer->SetRendererType(eRendererType::enToggle); break;
	//case eValueTypes::TYPE_NUMBER: m_colRenderer->SetRendererType(eRendererType::enText); break;
	//case eValueTypes::TYPE_DATE: m_colRenderer->SetRendererType(eRendererType::enDate); break;
	//case eValueTypes::TYPE_STRING: m_colRenderer->SetRendererType(eRendererType::enText); break;
	//default: m_colRenderer->SetRendererType(eRendererType::enText);
	//}

	m_tableCtrl->DeleteColumn(m_columnObject);
	m_tableCtrl->InsertColumn(idx, m_columnObject);
}

void CValueTableBoxColumn::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
	wxDataViewCtrl *m_tableCtrl = dynamic_cast<wxDataViewCtrl *>(visualHost->GetWxObject(GetParent()));
	wxASSERT(m_tableCtrl);
	CDataViewColumnObject *m_columnObject = dynamic_cast<CDataViewColumnObject *>(obj);
	wxASSERT(m_columnObject);
	m_columnObject->SetHidden(true);
	m_tableCtrl->DeleteColumn(m_columnObject);
}

//***********************************************************************************
//*                                  Property                                       *
//***********************************************************************************

void CValueTableBoxColumn::ReadProperty()
{
	m_properties["name"]->SetValue(m_name);
	m_properties["title"]->SetValue(m_title);
	m_properties["type"]->SetValue(m_type);
	m_properties["width"]->SetValue(m_width);
	m_properties["align"]->SetValue(m_align);
	//m_properties["icon"]->SetValue(m_icon);
	m_properties["visible"]->SetValue(m_visible);
	m_properties["resizable"]->SetValue(m_resizable);
	m_properties["sortable"]->SetValue(m_sortable);
	m_properties["reorderable"]->SetValue(m_reorderable);

	//set model id
	m_properties["source"]->SetValue(m_model_id);
}

void CValueTableBoxColumn::SaveProperty()
{
	m_name = m_properties["name"]->GetValueAsString();
	m_title = m_properties["title"]->GetValueAsString();
	m_type = m_properties["type"]->GetValueAsInteger();
	m_width = m_properties["width"]->GetValueAsInteger();
	m_align = m_properties["align"]->GetValueAsInteger();
	m_icon = m_properties["icon"]->GetValueAsBitmap();
	m_visible = m_properties["visible"]->GetValueAsInteger();
	m_resizable = m_properties["resizable"]->GetValueAsInteger();
	m_sortable = m_properties["sortable"]->GetValueAsInteger();
	m_reorderable = m_properties["reorderable"]->GetValueAsInteger();

	//set model id
	m_model_id = m_properties["source"]->GetValueAsInteger();
}