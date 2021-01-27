////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : stack window
////////////////////////////////////////////////////////////////////////////

#include "stackWindow.h"
#include "compiler/debugger/debugServer.h"
#include "utils/fs/fs.h"

wxBEGIN_EVENT_TABLE(CStackWindow, wxPanel)
EVT_DEBUG(CStackWindow::OnDebugEvent)
wxEND_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "utils/bitmaps_res.h"

CStackWindow::CStackWindow(wxWindow *parent, int id) : wxPanel(parent, id), m_treeCtrl(new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT))
{
	debugServer->AddHandler(this);

	SetSizer(new wxBoxSizer(wxHORIZONTAL));
	ClearAndCreate();
	GetSizer()->Add(m_treeCtrl, 1, wxEXPAND);

	/*wxImageList *imageList = new wxImageList(16, 16, false, 0);
	imageList->Add(wxMEMORY_IMAGE(Breakline_png));
	m_treeCtrl->SetImageList(imageList, wxIMAGE_LIST_NORMAL);*/
}

void CStackWindow::SetStack(CMemoryReader &commandReader)
{
	ClearAndCreate();

	unsigned int count = commandReader.r_u32();

	for (unsigned int i = 0; i < count; i++)
	{
		wxString sModule; commandReader.r_stringZ(sModule);
		wxString sLine; sLine << commandReader.r_u32();

		long index = m_treeCtrl->InsertItem(m_treeCtrl->GetItemCount(), sModule);

		m_treeCtrl->SetItem(index, 0, sModule);
		m_treeCtrl->SetItem(index, 1, sLine);
	}
}

void CStackWindow::OnDebugEvent(wxDebugEvent& event)
{
	switch (event.GetEventId())
	{
	case EventId_LeaveLoop:
	case EventId_SessionEnd: ClearAndCreate(); break;
	}
}

void CStackWindow::ClearAndCreate()
{
	m_treeCtrl->ClearAll();
	m_treeCtrl->AppendColumn("Module", wxLIST_FORMAT_LEFT, 750);
	m_treeCtrl->AppendColumn("Line",   wxLIST_FORMAT_LEFT, 100);
}

CStackWindow::~CStackWindow()
{
	//debugServer->RemoveHandler(this);
}

