////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxwidgets community
//	Description : main frame window
////////////////////////////////////////////////////////////////////////////

#include "mainFrame.h"
#include "mainFrameChild.h"

//common 
#include "common/reportManager.h"
#include "theme/luna_auitabart.h"
#include "theme/luna_dockart.h"
#include "window/property/objinspect.h"
#include "forms/visualEditor.h"
#include "window/mainFrame.h"

#include <wx/artprov.h>
#include <wx/config.h>
#include <wx/clipbrd.h>
#include <wx/cmdproc.h>

#include <wx/ffile.h>
#include <wx/tokenzr.h>
#include <wx/sysopt.h>

//***********************************************************************************
//*                                 Constructor                                     *
//***********************************************************************************

CMainFrame::CMainFrame(const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	: wxDocParentFrameAnyBase(this)
{
	Create(title, pos, size);
}

#include "compiler/value.h"

bool CMainFrame::Create(const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
{
	m_docManager = new CReportManager();

	if (!wxAuiMDIParentFrame::Create(NULL, wxID_ANY, title, pos, size, style, name))
		return false;

	this->Bind(wxEVT_MENU, &CMainFrame::OnExit, this, wxID_EXIT);
	this->Bind(wxEVT_CLOSE_WINDOW, &CMainFrame::OnCloseWindow, this);

	this->SetArtProvider(new CLunaTabArt());

	// notify wxAUI which valueFrame to use
	m_mgr.SetManagedWindow(this);
	m_mgr.SetArtProvider(new CLunaDockArt());

	return true;
}

CDocChildFrame *CMainFrame::CreateChildFrame(CView *m_view, wxPoint m_pos_wnd, wxSize m_size_wnd, long style)
{
	CDocument *m_doc = m_view->GetDocument();

	// create a child valueFrame of appropriate class for the current mode
	CDocChildFrame *subvalueFrame = new CDocChildFrame(m_doc, m_view, CMainFrame::Get(), wxID_ANY, m_doc->GetTitle(), m_pos_wnd, m_size_wnd, style);
	subvalueFrame->SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

	subvalueFrame->SetIcon(m_doc->GetIcon());

	subvalueFrame->Iconize(false); // restore the window if minimized
	subvalueFrame->SetFocus();     // focus on my window
	subvalueFrame->Raise();        // bring window to front
	subvalueFrame->Maximize();

	return subvalueFrame;
}

// bring window to front
void CMainFrame::Raise()
{
#if __WXMSW__
	keybd_event(0, 0, 0, 0); // Simulate a key press
#endif
	wxAuiMDIParentFrame::Raise();
}

CMainFrame::~CMainFrame()
{
	objectInspectorDestroy();
	visualEditorDatabaseDestroy();

	reportManagerDestroy();
	
	// deinitialize the valueFrame manager
	m_mgr.UnInit();
}