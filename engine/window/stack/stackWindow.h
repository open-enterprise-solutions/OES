#if !defined(_STACKBAR_H__)
#define _STACKBAR_H__

#include <wx/panel.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>

#include "compiler/debugger/debugEvent.h"

class CStackWindow;
class CMemoryReader;

#define stackWindow           (CStackWindow::Get())

class CStackWindow : public wxPanel
{
	wxListCtrl *m_treeCtrl;
	static CStackWindow *s_instance;

	CStackWindow(wxWindow *parent, int id = wxID_ANY);

public:

	static CStackWindow* Get();

	virtual ~CStackWindow();

	void ClearAndCreate();
	void SetStack(CMemoryReader &commandReader);

protected:

	//events 
	void OnDebugEvent(wxDebugEvent& event);

	wxDECLARE_EVENT_TABLE();
};

#endif // !defined(_STACKBAR_H__)
