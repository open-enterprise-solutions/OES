////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : simple grid
////////////////////////////////////////////////////////////////////////////

#include "valuegrid.h"
#include "methods.h"
#include "utils/stringutils.h"

#include "window/grid/gridCommon.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueGrid, CValue);

CMethods CValueGrid::m_methods;

CValueGrid::CValueGrid() : CValue(eValueTypes::TYPE_VALUE) {}

CValueGrid::~CValueGrid() {}

enum
{
	enShowGrid = 0,
};

void CValueGrid::PrepareNames() const
{
	SEng aMethods[] =
	{
		{"showGrid","showGrid()"},
	};

	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods.PrepareMethods(aMethods, nCountM);
}

void CValueGrid::SetAttribute(CAttributeParameters &aParams, CValue &cVal)
{
}

CValue CValueGrid::GetAttribute(CAttributeParameters &aParams)
{
	return CValue();
}

#include "common/reportManager.h"

CValue CValueGrid::Method(CMethodParameters &aParams)
{
	CValue ret;

	switch (aParams.GetMethodIndex())
	{
	case enShowGrid: ShowGrid(aParams[0].GetType() == eValueTypes::TYPE_EMPTY ? reportManager->MakeNewDocumentName() : aParams[0].ToString()); break;
	}

	return ret;
}

#include "common/cmdProc.h"
#include "window/mainFrame.h"
#include "window/mainFrameChild.h"
#include "common/templates/template.h"
#include "common/reportManager.h"

bool CValueGrid::ShowGrid(const wxString &sTitle)
{
	CGridEditDocument *m_document = new CGridEditDocument();

	reportManager->AddDocument(m_document);

	m_document->SetCommandProcessor(m_document->CreateCommandProcessor());

	m_document->SetTitle(sTitle);
	m_document->SetFilename(sTitle, true);

	wxScopedPtr<CGridEditView> view(new CGridEditView());
	if (!view) return false;

	view->SetDocument(m_document);

	// create a child valueFrame of appropriate class for the current mode
	CDocChildFrame *subvalueFrame = new CDocChildFrame(m_document, view.get(), CMainFrame::Get(), wxID_ANY, sTitle, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
	subvalueFrame->SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

	if (view->OnCreate(m_document, wxDOC_NEW)) { view->ShowFrame(); }
	return view.release() != NULL;
}

//**********************************************************************
//*                       Runtime register                             *
//**********************************************************************

VALUE_REGISTER(CValueGrid, "tableDocument", TEXT2CLSID("VL_GRID"));