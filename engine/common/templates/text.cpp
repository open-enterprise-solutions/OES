#include "text.h"
#include "window/mainFrame.h"

// ----------------------------------------------------------------------------
// CTextEditView implementation
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(CTextEditView, CView);

wxBEGIN_EVENT_TABLE(CTextEditView, CView)
EVT_MENU(wxID_COPY, CTextEditView::OnCopy)
EVT_MENU(wxID_PASTE, CTextEditView::OnPaste)
EVT_MENU(wxID_SELECTALL, CTextEditView::OnSelectAll)
wxEND_EVENT_TABLE()

bool CTextEditView::OnCreate(CDocument *doc, long flags)
{	
	m_text = new wxStyledTextCtrl(m_viewFrame, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxBORDER_THEME);

	//Set margin cursor
	for (int margin = 0; margin < m_text->GetMarginCount(); margin++)
		m_text->SetMarginCursor(margin, wxSTC_CURSORARROW);

	m_text->SetReadOnly(flags == wxDOC_READONLY);

	return CView::OnCreate(doc, flags);
}

void CTextEditView::OnDraw(wxDC *WXUNUSED(dc))
{
	// nothing to do here, wxTextCtrl draws itself
}

bool CTextEditView::OnClose(bool deleteWindow)
{
	Activate(false);

	if (deleteWindow)
	{
		GetFrame()->Destroy();
		SetFrame(NULL);
	}

	return CView::OnClose(deleteWindow);
}

// ----------------------------------------------------------------------------
// CTextDocument: wxDocument and wxTextCtrl married
// ----------------------------------------------------------------------------

wxIMPLEMENT_CLASS(CTextDocument, CDocument);

bool CTextDocument::OnCreate(const wxString& path, long flags)
{
	if (!CDocument::OnCreate(path, flags))
		return false;

	// subscribe to changes in the text control to update the document state
	// when it's modified
	GetTextCtrl()->Connect
	(
		wxEVT_TEXT,
		wxCommandEventHandler(CTextDocument::OnTextChange),
		NULL,
		this
	);

	return true;
}

// Since text windows have their own method for saving to/loading from files,
// we override DoSave/OpenDocument instead of Save/LoadObject
bool CTextDocument::DoSaveDocument(const wxString& filename)
{
	return GetTextCtrl()->SaveFile(filename);
}

bool CTextDocument::DoOpenDocument(const wxString& filename)
{
	if (!GetTextCtrl()->LoadFile(filename))
		return false;

	// we're not modified by the user yet
	Modify(false);

	return true;
}

bool CTextDocument::IsModified() const
{
	wxStyledTextCtrl* wnd = GetTextCtrl();
	return CDocument::IsModified() || (wnd && wnd->IsModified());
}

void CTextDocument::Modify(bool modified)
{
	CDocument::Modify(modified);

	wxStyledTextCtrl* wnd = GetTextCtrl();
	if (wnd && !modified)
	{
		wnd->DiscardEdits();
	}
}

void CTextDocument::OnTextChange(wxCommandEvent& event)
{
	Modify(true);

	event.Skip();
}

// ----------------------------------------------------------------------------
// CTextEditDocument implementation
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(CTextEditDocument, CTextDocument);

wxStyledTextCtrl* CTextEditDocument::GetTextCtrl() const
{
	wxView* view = GetFirstView();
	return view ? wxDynamicCast(view, CTextEditView)->GetText() : NULL;
}