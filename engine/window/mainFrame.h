#ifndef _MAINFRAME_H__
#define _MAINFRAME_H__

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/docview.h>
#include <wx/stc/stc.h>

class IMetaObject;
class CReportManager;
class CDocChildFrame;

#include "appData.h"

class CView;

#include "settings/keybinder.h"
#include "settings/fontcolorsettings.h"
#include "settings/editorsettings.h"

#define mainFrame            	(CMainFrame::Get())
#define mainFrameInit()         (CMainFrame::InitializeFrame())
#define mainFrameDestroy()  	(CMainFrame::DestroyFrame())

#define metatreeWnd             mainFrame->GetMetadataTree()

//********************************************************************************
//*                                 ID's                                         *
//********************************************************************************

class CMainFrame : public wxAuiMDIParentFrame,
	public wxDocParentFrameAnyBase
{
	static CMainFrame *s_instance;

protected:

	// Create AUI manager 
	wxAuiManager m_mgr;

	KeyBinder             m_keyBinder;
	FontColorSettings     m_fontColorSettings;
	EditorSettings        m_editorSettings;

	wxAuiToolBar *m_toolbarDefault;
	wxAuiToolBar *m_toolbarAdditional;

public:

	/**
	* Show property in mainFrame
	*/
	void ShowProperty();

public:

	static CMainFrame* Get();

	// Force the static appData instance to Init()
	static void InitializeFrame();
	static void DestroyFrame();

public:

	static CDocChildFrame *CreateChildFrame(CView *view, wxPoint pos_wnd, wxSize size_wnd, long style = wxDEFAULT_FRAME_STYLE);

	KeyBinder             GetKeyBinder() { return m_keyBinder; };
	FontColorSettings     GetFontColorSettings() { return m_fontColorSettings; }
	EditorSettings        GetEditorSettings() { return m_editorSettings; }

	virtual wxMenu *GetDefaultMenu(int nTypeMenu) = 0;
	virtual CMetadataTree *GetMetadataTree() { return NULL; }
	virtual void CreateGUI() = 0;

	virtual wxAuiToolBar *GetDefaultToolbar() { return m_toolbarDefault; }
	virtual wxAuiToolBar *GetAdditionalToolbar() { return m_toolbarAdditional; }

	void OnActivateView(bool activate, wxView *activeView, wxView *deactiveView);

	// bring window to front
	virtual void Raise() override;

protected:

	// hook the document manager into event handling chain here
	virtual bool TryBefore(wxEvent& event) override
	{
		// It is important to send the event to the base class first as
		// wxMDIParentFrame overrides its TryBefore() to send the menu events
		// to the currently active child valueFrame and the child must get them
		// before our own TryProcessEvent() is executed, not afterwards.
		return wxAuiMDIParentFrame::TryBefore(event) || TryProcessEvent(event);
	}

protected:

	CMainFrame(const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString& name = wxFrameNameStr);

	bool Create(const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString& name = wxFrameNameStr);

public:

	~CMainFrame();

private:

	//Events 
	void OnExit(wxCommandEvent& WXUNUSED(event));
	void OnCloseWindow(wxCloseEvent& event);
};

#endif 