////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : main events
////////////////////////////////////////////////////////////////////////////

#include "mainapp.h"
#include "window/mainFrame.h"
#include "window/mainFrameChild.h"

void CDesignerApp::OnKeyEvent(wxKeyEvent &event)
{
	if (event.GetKeyCode() == WXK_ESCAPE)
	{
		if (!appData->IsDesignerMode())
		{
			wxAuiNotebook *m_nootebook = mainFrame->GetNotebook();
			wxASSERT(m_nootebook);
			if (m_nootebook->GetPageCount() > 0)
			{
				CDocChildFrame *m_childFrame = dynamic_cast<CDocChildFrame *>(m_nootebook->GetPage(m_nootebook->GetSelection()));
				if (m_childFrame) m_childFrame->Close();
			}
		}
	}

	event.Skip();
}

#include "forms/visualEditor.h"

void CDesignerApp::OnMouseEvent(wxMouseEvent &event)
{
	if (event.LeftIsDown() || event.RightIsDown())
	{
		wxWindow *m_currentWnd = dynamic_cast<wxWindow *>(event.GetEventObject()); wxWindow *m_window = m_currentWnd;
		while (m_window && !m_window->IsKindOf(CLASSINFO(CVisualEditorContextForm::CVisualEditor))) { m_window = m_window->GetParent(); }
		if (m_window)
		{
			CVisualEditorContextForm::CVisualEditor *m_visualEditor = dynamic_cast<CVisualEditorContextForm::CVisualEditor *>(m_window);

			if (m_visualEditor)
			{
				if (event.GetEventType() == wxEVT_LEFT_DOWN) m_visualEditor->OnLeftClickFromApp(m_currentWnd);
				else if (event.GetEventType() == wxEVT_RIGHT_DOWN) m_visualEditor->OnRightClickFromApp(m_currentWnd, event);
			}
		}
	}

	event.Skip(); 
}

void CDesignerApp::OnSetFocus(wxFocusEvent &event)
{
	wxWindow *m_window = dynamic_cast<wxWindow *>(event.GetEventObject());
	while (m_window && !m_window->IsKindOf(CLASSINFO(CDocChildFrame))) { m_window = m_window->GetParent(); }

	if (m_window)
	{
		wxAuiNotebook *m_nootebook = mainFrame->GetNotebook();
		CDocChildFrame *m_pageWnd = dynamic_cast<CDocChildFrame *>(m_window); size_t newSelPos;
		for (newSelPos = 0; newSelPos < m_nootebook->GetPageCount(); newSelPos++) { if (m_nootebook->GetPage(newSelPos) == m_pageWnd) break; }
		if (newSelPos != m_nootebook->GetSelection()) m_pageWnd->Activate();
	}

	event.Skip();
}

int CDesignerApp::FilterEvent(wxEvent& event)
{
	if (event.GetEventType() == wxEVT_KEY_DOWN)
	{
		OnKeyEvent((wxKeyEvent&)event);
	}
	else if (event.GetEventType() == wxEVT_LEFT_DOWN || event.GetEventType() == wxEVT_RIGHT_DOWN)
	{
		OnMouseEvent((wxMouseEvent&)event);
	}
	else if (event.GetEventType() == wxEVT_SET_FOCUS)
	{
		OnSetFocus((wxFocusEvent&)event);
	}

	return Event_Skip;
}