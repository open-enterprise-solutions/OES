#include "tableBox.h"
#include "forms/visualEditor.h"

//***********************************************************************************
//*                           IMPLEMENT_DYNAMIC_CLASS                               *
//***********************************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CValueTableBox, CValueWindow);

//***********************************************************************************
//*                                 Special tablebox func                           *
//***********************************************************************************

void CValueTableBox::AddNewColumn()
{
	CValueTableBoxColumn *m_newTableBoxColumn = new CValueTableBoxColumn();
	m_newTableBoxColumn->ReadProperty();

	m_visualHostContext->InsertObject(m_newTableBoxColumn, this);
	m_visualHostContext->ResolveSubtreeNameConflicts(m_newTableBoxColumn, this);

	m_newTableBoxColumn->SaveProperty();
	m_newTableBoxColumn->IncrRef();

	m_visualHostContext->RefreshEditor();
}

#include "appData.h"
#include "forms/visualEditorView.h"
#include "compiler/valueType.h"

void CValueTableBox::CreateColumns(wxDataViewCtrl *tableCtrl)
{
	if (appData->IsDesignerMode())
		return;

	wxDataViewCtrl *m_tableCtrl = tableCtrl ? tableCtrl : dynamic_cast<wxDataViewCtrl *>(GetWxObject());

	wxASSERT(m_tableCtrl);

	CVisualDocument *m_visualDocument = m_frameOwner->GetVisualDocument();

	//clear all controls 
	for (unsigned int idx = 0; idx < GetChildCount(); idx++)
	{
		IControlElement *m_childColumn = GetChild(idx);

		wxASSERT(m_childColumn);

		if (m_visualDocument)
		{
			CVisualView *m_visualView = m_visualDocument->GetVisualView();
			wxASSERT(m_visualView);
			m_visualView->RemoveControl(m_childColumn, this);
		}

		m_childColumn->SetParent(NULL);
		m_childColumn->DecrRef();
	}

	//clear all children
	RemoveAllChildren();

	//clear all old columns
	m_tableCtrl->ClearColumns();

	//create new columns
	IValueTable::IValueTableColumn *m_columns = m_tableModel->GetColumns();

	for (unsigned int idx = 0; idx < m_columns->GetColumnCount(); idx++)
	{
		IValueTable::IValueTableColumn::IValueTableColumnInfo *m_columnInfo = m_columns->GetColumnInfo(idx);

		CValueTableBoxColumn *m_newTableBoxColumn = new CValueTableBoxColumn();

		m_newTableBoxColumn->SetParent(this);
		this->AddChild(m_newTableBoxColumn);

		CValueTypeDescription *m_typeDescription = m_columnInfo->GetColumnTypes();

		if (m_typeDescription) m_newTableBoxColumn->m_type = m_typeDescription->GetLongTypes()[0]; //TODO
		else m_newTableBoxColumn->m_type = eValueTypes::TYPE_STRING;

		m_newTableBoxColumn->m_title = m_columnInfo->GetColumnCaption();
		m_newTableBoxColumn->m_model_id = m_columnInfo->GetColumnID();
		m_newTableBoxColumn->m_width = m_columnInfo->GetColumnWidth();

		m_newTableBoxColumn->ReadProperty();

		CVisualEditorContextForm::ResolveNameConflict(m_newTableBoxColumn);

		m_newTableBoxColumn->IncrRef();

		if (m_visualDocument)
		{
			CVisualView *m_visualView = m_visualDocument->GetVisualView();
			wxASSERT(m_visualView);
			m_visualView->CreateControl(m_newTableBoxColumn, this);
		}
	}

	if (m_visualDocument)
	{
		CVisualView *m_visualView = m_visualDocument->GetVisualView();
		wxASSERT(m_visualView);
		//fix size in parent window 
		wxWindow *m_wndParent = m_visualView->GetParent();
		if (m_wndParent) m_wndParent->Layout();
	}
}

//***********************************************************************************
//*                              CValueTableBox                                     *
//***********************************************************************************

CValueTableBox::CValueTableBox() : CValueWindow(),
m_name("tablebox"), m_tableModel(NULL), m_tableCurrentLine(NULL), m_dataSource(0)
{
	PropertyCategory *m_categoryTable = new PropertyCategory("TableBox");
	m_categoryTable->AddProperty("name");
	m_category->AddCategory(m_categoryTable);

	PropertyCategory *m_categoryData = new PropertyCategory("Data");
	m_categoryData->AddProperty("data_source");
	m_category->AddCategory(m_categoryData);

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["data_source"] = new Property("data_source", PropertyType::PT_OPTION_CUSTOM, this);
}

CValueTableBox::~CValueTableBox()
{
	if (m_tableModel)
		m_tableModel->DecrRef();

	if (m_tableCurrentLine)
		m_tableCurrentLine->DecrRef(); 
}

#include "metadata/metaObjectsDefines.h"
#include "metadata/objects/baseObject.h"

wxObject* CValueTableBox::Create(wxObject* parent, IVisualHost *visualHost)
{
	wxDataViewCtrl *m_table = new wxDataViewCtrl((wxWindow *)parent, wxID_ANY,
		m_pos,
		m_size,
		wxDV_SINGLE | wxDV_HORIZ_RULES | wxDV_VERT_RULES | wxDV_ROW_LINES);

	if (!visualHost->IsDemonstration())
	{
		m_table->Bind(wxEVT_DATAVIEW_COLUMN_HEADER_CLICK, &CValueTableBox::OnColumnClick, this);
		m_table->Bind(wxEVT_DATAVIEW_COLUMN_REORDERED, &CValueTableBox::OnColumnReordered, this);

		//system events:
		m_table->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &CValueTableBox::OnSelectionChanged, this);

		m_table->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &CValueTableBox::OnItemActivated, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_COLLAPSED, &CValueTableBox::OnItemCollapsed, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_EXPANDED, &CValueTableBox::OnItemExpanded, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_COLLAPSING, &CValueTableBox::OnItemCollapsing, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_EXPANDING, &CValueTableBox::OnItemExpanding, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_START_EDITING, &CValueTableBox::OnItemStartEditing, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_EDITING_STARTED, &CValueTableBox::OnItemEditingStarted, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_EDITING_DONE, &CValueTableBox::OnItemEditingDone, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, &CValueTableBox::OnItemValueChanged, this);

#if wxUSE_DRAG_AND_DROP 
		m_table->Bind(wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, &CValueTableBox::OnItemBeginDrag, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, &CValueTableBox::OnItemDropPossible, this);
		m_table->Bind(wxEVT_DATAVIEW_ITEM_DROP, &CValueTableBox::OnItemDrop, this);
#endif // wxUSE_DRAG_AND_DROP

#if wxUSE_DRAG_AND_DROP && wxUSE_UNICODE
		m_table->EnableDragSource(wxDF_UNICODETEXT);
		m_table->EnableDropTarget(wxDF_UNICODETEXT);
#endif // wxUSE_DRAG_AND_DROP && wxUSE_UNICODE
	}

	if (!visualHost->IsDesignerHost()) {

		if (m_dataSource == FORM_ACTION) {
			m_tableModel = dynamic_cast<IDataObjectList *>(m_frameOwner->GetSourceObject());
		}
	}

	return m_table;
}

void CValueTableBox::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	wxDataViewCtrl *m_tableCtrl = dynamic_cast<wxDataViewCtrl *>(wxobject);

	if (m_tableCtrl)
	{
		if (!visualHost->IsDesignerHost() &&
			m_tableModel != m_tableCtrl->GetModel())
		{
			m_tableCtrl->AssociateModel(m_tableModel);

			if (m_tableModel->AutoCreateColumns())
				CreateColumns(m_tableCtrl);
		}
	}
}

void CValueTableBox::Update(wxObject* wxobject, IVisualHost *visualHost)
{
	wxDataViewCtrl *m_tableCtrl = dynamic_cast<wxDataViewCtrl *>(wxobject);

	if (m_tableCtrl)
	{
		if (!visualHost->IsDesignerHost() &&
			m_tableModel != m_tableCtrl->GetModel())
		{
			m_tableCtrl->AssociateModel(m_tableModel);

			if (m_tableModel->AutoCreateColumns())
				CreateColumns(m_tableCtrl);
		}
	}

	UpdateWindow(m_tableCtrl);
}

void CValueTableBox::OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
}

void CValueTableBox::Cleanup(wxObject* obj, IVisualHost *visualHost)
{
	wxDataViewCtrl *m_tableCtrl = dynamic_cast<wxDataViewCtrl *>(obj);

	if (m_tableCtrl)
	{
	}
}

//***********************************************************************************
//*                                  Property                                       *
//***********************************************************************************

void CValueTableBox::ReadProperty()
{
	CValueWindow::ReadProperty();

	m_properties["name"]->SetValue(m_name);
	m_properties["data_source"]->SetValue(m_dataSource);
}

void CValueTableBox::SaveProperty()
{
	CValueWindow::SaveProperty();

	m_name = m_properties["name"]->GetValueAsString();
	m_dataSource = m_properties["data_source"]->GetValueAsInteger();
}