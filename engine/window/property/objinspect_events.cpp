#include "objinspect.h"
#include "window/mainFrame.h"

void CObjectInspector::SelectObject(IObjectBase *selobj, bool force)
{
	ShowProperty();

	m_currentSel = selobj;
	m_evthandler = NULL;
	Create(force);
}

void CObjectInspector::SelectObject(IObjectBase *selobj, wxEvtHandler *m_handler, bool force)
{
	ShowProperty();

	m_currentSel = selobj;
	m_evthandler = m_handler;
	
	Create(force);
}

void CObjectInspector::ShowProperty()
{
	mainFrame->ShowProperty();
}

void CObjectInspector::RefreshProperty()
{
	Freeze();

	int pageNumber = m_pg->GetSelectedPage();
	wxString pageName;
	if (pageNumber != wxNOT_FOUND)
	{
		pageName = m_pg->GetPageName(pageNumber);
	}

	m_currentSel->OnPropertyCreated();

	wxPGProperty *m_lastProperty = NULL;

	if (m_pageName == wxT("Properties")) m_lastProperty = m_pg->GetSelectedProperty();
	else if (m_pageName == wxT("Events"))  m_lastProperty = m_eg->GetSelectedProperty();

	wxString m_lastPropName = m_lastProperty->GetName(); 

	// Clear Property Grid Manager
	m_pg->Clear();
	// Now we do the same thing for event grid...
	m_eg->Clear();

	m_propMap.clear();
	m_eventMap.clear();

	PropertyMap propMap, dummyPropMap;
	EventMap eventMap, dummyEventMap;

	// We create the categories with the properties of the object organized by "classes"
	CreateCategory(m_currentSel->GetClassName(), m_currentSel, propMap, false);
	CreateCategory(m_currentSel->GetClassName(), m_currentSel, eventMap, true);

	IObjectBase* parent = m_currentSel->GetParent();
	if (parent)
	{
		if (parent->IsItem())
		{
			CreateCategory(m_currentSel->GetClassName(), parent, dummyPropMap, false);
			CreateCategory(m_currentSel->GetClassName(), parent, dummyEventMap, true);
		}
	}

	// Select previously selected page, or first page
	if (m_pg->GetPageCount() > 0)
	{
		int pageIndex = m_pg->GetPageByName(pageName);
		if (wxNOT_FOUND != pageIndex)
		{
			m_pg->SelectPage(pageIndex);
		}
		else
		{
			m_pg->SelectPage(0);
		}
	}

	m_pg->Refresh();
	m_pg->Update();
	m_eg->Refresh();
	m_eg->Update();

	Thaw();

	if (m_pageName == wxT("Properties"))
	{
		wxPGProperty* p = m_pg->GetPropertyByName(m_lastPropName);
		if (p)
		{
			m_pg->SelectProperty(p, true);
			m_pg->SetFocus();
		}
	}
	else if (m_pageName == wxT("Events"))
	{
		wxPGProperty* p = m_eg->GetPropertyByName(m_lastPropName);
		if (p)
		{
			m_eg->SelectProperty(p, true);
			m_eg->SetFocus();
		}
	}
}

IObjectBase* CObjectInspector::GetSelectedObject()
{
	return m_currentSel;
}