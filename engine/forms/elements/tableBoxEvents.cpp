#include "tableBox.h" 
#include "forms/visualEditor.h"

void CValueTableBox::OnColumnClick(wxDataViewEvent &event)
{
	CDataViewColumnObject *m_columnValue = dynamic_cast<CDataViewColumnObject *>(event.GetDataViewColumn());
	wxASSERT(m_columnValue);
	if (m_visualHostContext)
	{
		CVisualEditorContextForm::CVisualEditor *m_visualEditor = m_visualHostContext->GetVisualEditor();
		wxASSERT(m_visualEditor);
		IControlElement *m_columnControl = m_visualEditor->GetObjectBase(m_columnValue);
		wxASSERT(m_columnControl);
		m_visualHostContext->SelectObject(m_columnControl);
	}

	event.Skip();
}

void CValueTableBox::OnColumnReordered(wxDataViewEvent &event)
{
	CDataViewColumnObject *m_columnValue = dynamic_cast<CDataViewColumnObject *>(event.GetDataViewColumn());
	wxASSERT(m_columnValue);
	if (m_visualHostContext)
	{
		CVisualEditorContextForm::CVisualEditor *m_visualEditor = m_visualHostContext->GetVisualEditor();
		wxASSERT(m_visualEditor);
		IControlElement *m_columnControl = m_visualEditor->GetObjectBase(m_columnValue);
		wxASSERT(m_columnControl);

		if (ChangeChildPosition(m_columnControl, event.GetColumn()))
			m_visualHostContext->RefreshEditor();
	}

	event.Skip();
}

//*********************************************************************
//*                          System event                             *
//*********************************************************************

#include "metadata/objects/baseObject.h"

void CValueTableBox::OnSelectionChanged(wxDataViewEvent &event)
{
	// event is a wxDataViewEvent
	wxDataViewItem item = event.GetItem();

	if (m_tableCurrentLine)
		m_tableCurrentLine->DecrRef();

	m_tableCurrentLine = NULL; 

	if (!item.IsOk())
		return;

	m_tableCurrentLine = m_tableModel->GetRowAt(item);
	m_tableCurrentLine->IncrRef();

	if (m_dataSource == FORM_ACTION)
	{
		IDataObjectList *objDataList = dynamic_cast<IDataObjectList *>(m_frameOwner->GetSourceObject());

		if (objDataList) {
			objDataList->SetCurrentLine(reinterpret_cast<long>(item.GetID()));
		}
	}

	event.Skip();
}

void CValueTableBox::OnItemActivated(wxDataViewEvent &event)
{
	// event is a wxDataViewEvent
	wxDataViewItem item = event.GetItem();

	if (!item.IsOk())
		return;

	if (m_dataSource == FORM_ACTION)
	{
		IDataObjectList *objDataList = dynamic_cast<IDataObjectList *>(m_frameOwner->GetSourceObject());

		if (objDataList) {
			objDataList->SetCurrentLine(reinterpret_cast<long>(item.GetID()));
			objDataList->EditValue();
		}
	}
	else
	{
		EditValue();
	}

	event.Skip();
}

void CValueTableBox::OnItemCollapsed(wxDataViewEvent &event)
{
	event.Skip();
}

void CValueTableBox::OnItemExpanded(wxDataViewEvent &event)
{
	event.Skip();
}

void CValueTableBox::OnItemCollapsing(wxDataViewEvent &event)
{
	event.Skip();
}

void CValueTableBox::OnItemExpanding(wxDataViewEvent &event)
{
	event.Skip();
}

void CValueTableBox::OnItemStartEditing(wxDataViewEvent &event)
{
	event.Skip();
}

void CValueTableBox::OnItemEditingStarted(wxDataViewEvent &event)
{
	event.Skip();
}

void CValueTableBox::OnItemEditingDone(wxDataViewEvent &event)
{
	event.Skip();
}

void CValueTableBox::OnItemValueChanged(wxDataViewEvent &event)
{
	event.Skip();
}

#if wxUSE_DRAG_AND_DROP

void CValueTableBox::OnItemBeginDrag(wxDataViewEvent &event)
{
}

void CValueTableBox::OnItemDropPossible(wxDataViewEvent &event)
{
	if (event.GetDataFormat() != wxDF_UNICODETEXT)
		event.Veto();
	else
		event.SetDropEffect(wxDragMove);	// check 'move' drop effect
}

void CValueTableBox::OnItemDrop(wxDataViewEvent &event)
{
	wxDataViewItem item(event.GetItem());

	if (event.GetDataFormat() != wxDF_UNICODETEXT)
	{
		event.Veto();
		return;
	}
}

#endif // wxUSE_DRAG_AND_DROP