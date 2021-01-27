////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : output window
////////////////////////////////////////////////////////////////////////////

#include "outputWindow.h"
#include "window/mainFrame.h"

///////////////////////////////////////////////////////////////////////////////
// COutputWindow
///////////////////////////////////////////////////////////////////////////////

COutputWindow* COutputWindow::s_instance = NULL;

COutputWindow* COutputWindow::Get()
{
	if (!s_instance)
	{
		s_instance = new COutputWindow(CMainFrame::Get(), wxID_ANY);
		s_instance->SetFontColorSettings(mainFrame->GetFontColorSettings());
	}

	return s_instance;
}