////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : metatree events
////////////////////////////////////////////////////////////////////////////

#include "metatreeWnd.h"

void CMetadataTree::CMetadataTreeWnd::OnLeftDClick(wxMouseEvent &event)
{
	wxTreeItemId curItem = HitTest(event.GetPosition());
	if (curItem.IsOk()) { SelectItem(curItem); m_ownerTree->ActivateItem(curItem); } //event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnLeftUp(wxMouseEvent &event)
{
	event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnLeftDown(wxMouseEvent &event)
{
	wxTreeItemId curItem = HitTest(event.GetPosition());
	if (curItem.IsOk()) { SelectItem(curItem); m_ownerTree->PropertyItem(); }
	event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnRightUp(wxMouseEvent &event)
{
	wxTreeItemId curItem = HitTest(event.GetPosition());

	if (curItem.IsOk())
	{
		SelectItem(curItem); SetFocus();
		wxMenu *m_defaultMenu = new wxMenu;
		m_ownerTree->PrepareContextMenu(m_defaultMenu, curItem);

		for (auto def_menu : m_defaultMenu->GetMenuItems())
		{
			if (def_menu->GetId() == ID_METATREE_NEW
				|| def_menu->GetId() == ID_METATREE_EDIT
				|| def_menu->GetId() == ID_METATREE_REMOVE
				|| def_menu->GetId() == ID_METATREE_PROPERTY)
			{
				continue;
			}

			GetEventHandler()->Bind(wxEVT_MENU, &CMetadataTree::CMetadataTreeWnd::OnCommandItem, this, def_menu->GetId());
		}

		PopupMenu(m_defaultMenu, event.GetPosition());

		for (auto def_menu : m_defaultMenu->GetMenuItems())
		{
			if (def_menu->GetId() == ID_METATREE_NEW
				|| def_menu->GetId() == ID_METATREE_EDIT
				|| def_menu->GetId() == ID_METATREE_REMOVE
				|| def_menu->GetId() == ID_METATREE_PROPERTY)
			{
				continue;
			}

			GetEventHandler()->Unbind(wxEVT_MENU, &CMetadataTree::CMetadataTreeWnd::OnCommandItem, this, def_menu->GetId());
		}

		delete m_defaultMenu;
	}

	m_ownerTree->PropertyItem(); event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnRightDown(wxMouseEvent &event)
{
	wxTreeItemId curItem = HitTest(event.GetPosition());

	if (curItem.IsOk())
	{
		SelectItem(curItem); SetFocus();
		wxMenu *m_defaultMenu = new wxMenu;
		m_ownerTree->PrepareContextMenu(m_defaultMenu, curItem);

		for (auto def_menu : m_defaultMenu->GetMenuItems())
		{
			if (def_menu->GetId() == ID_METATREE_NEW
				|| def_menu->GetId() == ID_METATREE_EDIT
				|| def_menu->GetId() == ID_METATREE_REMOVE
				|| def_menu->GetId() == ID_METATREE_PROPERTY)
			{
				continue;
			}

			GetEventHandler()->Bind(wxEVT_MENU, &CMetadataTree::CMetadataTreeWnd::OnCommandItem, this, def_menu->GetId());
		}

		PopupMenu(m_defaultMenu, event.GetPosition());

		for (auto def_menu : m_defaultMenu->GetMenuItems())
		{
			if (def_menu->GetId() == ID_METATREE_NEW
				|| def_menu->GetId() == ID_METATREE_EDIT
				|| def_menu->GetId() == ID_METATREE_REMOVE
				|| def_menu->GetId() == ID_METATREE_PROPERTY)
			{
				continue;
			}

			GetEventHandler()->Unbind(wxEVT_MENU, &CMetadataTree::CMetadataTreeWnd::OnCommandItem, this, def_menu->GetId());
		}

		delete m_defaultMenu;
	}

	m_ownerTree->PropertyItem(); event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnRightDClick(wxMouseEvent &event)
{
	event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnKeyUp(wxKeyEvent &event)
{
	event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnKeyDown(wxKeyEvent &event)
{
	event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnMouseMove(wxMouseEvent &event)
{
	event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnCreateItem(wxCommandEvent &event)
{
	m_ownerTree->CreateItem(); event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnEditItem(wxCommandEvent &event)
{
	m_ownerTree->EditItem(); event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnRemoveItem(wxCommandEvent &event)
{
	m_ownerTree->RemoveItem(); event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnPropertyItem(wxCommandEvent &event)
{
	m_ownerTree->PropertyItem(); event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnCommandItem(wxCommandEvent &event)
{
	m_ownerTree->CommandItem(event.GetId()); event.Skip();
}

#include <wx/clipbrd.h>

void CMetadataTree::CMetadataTreeWnd::OnCopyItem(wxCommandEvent &event)
{
	wxTreeItemId item = GetSelection();

	if (!item.IsOk())
		return;

	// Write some text to the clipboard
	if (wxTheClipboard->Open())
	{
		IMetaObject *metaObject = m_ownerTree->GetMetaObject(item);

		if (metaObject)
		{
			CMemoryWriter dataWritter;

			if (metaObject->SaveMeta(dataWritter))
			{
				wxCustomDataObject *dataCustomObject = new wxCustomDataObject;
				dataCustomObject->SetData(dataWritter.size(), dataWritter.pointer());

				// This data objects are held by the clipboard,
				// so do not delete them in the app.
				wxTheClipboard->SetData(dataCustomObject);
				wxTheClipboard->Close();
			}
		}
	}

	event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnPasteItem(wxCommandEvent &event)
{
	wxTreeItemId item = GetSelection();

	if (!item.IsOk())
		return;

	// Read some text
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
		}
		else
		{
			wxCustomDataObject data;
			wxTheClipboard->GetData(data);

			CMemoryReader dateReader(data.GetData(), data.GetDataSize());
			m_ownerTree->CreateItem();
		}

		wxTheClipboard->Close();
	}

	event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnDebugEvent(wxDebugEvent &event)
{
	if (m_ownerTree->m_docParent)
		return;

	switch (event.GetEventId())
	{
	case EventId_EnterLoop: m_ownerTree->EditModule(event.GetModuleName(), event.GetLine(), true); break;
	}
}

void CMetadataTree::CMetadataTreeWnd::OnPropertyModified(wxFramePropertyEvent &event)
{
	Property *p = event.GetFrameProperty();
	wxASSERT(p);
	if (p->GetType() == PropertyType::PT_WXNAME)
	{
		IMetaObject *obj = dynamic_cast<IMetaObject *>(p->GetObject());
		wxASSERT(obj);
		if (!m_ownerTree->RenameMetaObject(obj, p->GetValue()))
			event.Skip();
	}
}

void CMetadataTree::CMetadataTreeWnd::OnCollapsing(wxTreeEvent &event)
{
	if (GetRootItem() == event.GetItem())
		event.Veto();
	else
		event.Skip();
}

void CMetadataTree::CMetadataTreeWnd::OnExpanding(wxTreeEvent &event)
{
	event.Skip();
}
