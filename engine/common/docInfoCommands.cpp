////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko 
//	Description : document/view implementation for metaObject 
////////////////////////////////////////////////////////////////////////////

#include "docInfo.h"
#include "window/mainFrame.h"

// Called by valueFramework if created automatically by the default document
// manager class: gives view a chance to initialise
bool CView::OnCreate(CDocument *WXUNUSED(doc), long WXUNUSED(flags)) { return true; }

void CView::OnCreateToolbar(wxAuiToolBar *toolbar) {}

void CView::OnRemoveToolbar(wxAuiToolBar *toolbar) {}

void CView::OnActivateView(bool activate, wxView *activeView, wxView *deactiveView)
{
	mainFrame->OnActivateView(activate, activeView, deactiveView);
}

void CView::Activate(bool activate)
{
	if (GetDocument() && GetDocumentManager() && CMainFrame::Get())
	{
		OnActivateView(activate, this, GetDocumentManager()->GetCurrentView());
		GetDocumentManager()->ActivateView(this, activate);
	}
}
