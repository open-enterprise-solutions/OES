////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxwidgets community
//	Description : main frame window
////////////////////////////////////////////////////////////////////////////

#include "mainFrame.h" 
#include "common/reportManager.h"

//***********************************************************************************
//*                                 mainFrame                                       *
//***********************************************************************************

CMainFrame* CMainFrame::s_instance = NULL;

CMainFrame* CMainFrame::Get() { return s_instance; }

#include "window/mainFrameDesigner.h"
#include "window/mainFrameEnterprise.h"

void CMainFrame::InitializeFrame()
{
	if (appData->IsDesignerMode())
	{
		s_instance = new CMainFrameDesigner;
		s_instance->CreateGUI();
	}
	else if (appData->IsEnterpriseMode())
	{
		s_instance = new CMainFrameEnterprise;
		s_instance->CreateGUI();
	}
	else
	{
		s_instance = NULL;
	}

	wxTheApp->SetTopWindow(s_instance);
}

void CMainFrame::DestroyFrame()
{
	if (s_instance) { s_instance->Destroy(); s_instance = NULL; }
}