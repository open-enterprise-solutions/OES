#ifndef _MAINAPP_H__
#define _MAINAPP_H__

#include <wx/app.h>
#include <wx/aui/framemanager.h>
#include <wx/socket.h>

#include "appData.h"

class CDesignerApp : public wxApp
{
	wxLocale m_locale;

public:

	virtual bool OnInit();
#if wxUSE_ON_FATAL_EXCEPTION
	virtual void OnUnhandledException() override;
#endif
#if wxUSE_ON_FATAL_EXCEPTION && wxUSE_STACKWALKER
	virtual void OnFatalException() override;
#endif
	virtual int OnRun();
	virtual int OnExit();
	
	virtual int FilterEvent(wxEvent& event) override;

protected:

	void ComputeBuildID(); 

	//global process events:
	void OnKeyEvent(wxKeyEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnSetFocus(wxFocusEvent &event);

private:

	unsigned int m_buildID;
};

wxDECLARE_APP(CDesignerApp);

#endif 