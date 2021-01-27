////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxwidgets community
//	Description : main frame window
////////////////////////////////////////////////////////////////////////////

#include "mainFrameDesigner.h"
#include "compiler/debugger/debugServer.h"
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>

CMainFrameDesigner::CMainFrameDesigner(const wxString& title,
	const wxPoint& pos,
	const wxSize& size) : CMainFrame(title, pos, size)
{
	Connect(wxEVT_DEBUG_EVENT, wxDebugEventHandler(CMainFrameDesigner::OnDebugEvent), NULL, this);
	debugServer->AddHandler(this);
}

CMainFrameDesigner::~CMainFrameDesigner()
{
	Disconnect(wxEVT_DEBUG_EVENT, wxDebugEventHandler(CMainFrameDesigner::OnDebugEvent), NULL, this);
	debugServer->RemoveHandler(this);
}

void CMainFrameDesigner::CreateGUI()
{
	InitializeDefaultMenu();
	CreateWideGui();
}

#include "window/metatree/metatreeWnd.h"

bool CMainFrameDesigner::Show(bool show)
{
	if (!metatreeWnd->Load()) return false;
	return CMainFrame::Show(show);
}