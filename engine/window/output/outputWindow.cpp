////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : output window
////////////////////////////////////////////////////////////////////////////

#include "outputWindow.h"
#include "window/mainFrame.h"
#include "window/settings/fontcolorsettings.h"

wxBEGIN_EVENT_TABLE(COutputWindow, wxStyledTextCtrl)
EVT_LEFT_DCLICK(COutputWindow::OnDoubleClick)
EVT_KEY_DOWN(COutputWindow::OnKeyDown)
EVT_CONTEXT_MENU(COutputWindow::OnContextMenu)
wxEND_EVENT_TABLE()

#define DEF_LINENUMBER_ID 0
#define DEF_IMAGE_ID 1

COutputWindow::COutputWindow(wxWindow* parent, wxWindowID winid)
	: wxStyledTextCtrl(parent, winid, wxDefaultPosition, wxDefaultSize)
{
	// initialize styles
	StyleClearAll();

	//set Lexer to LEX_CONTAINER: This will trigger the styleneeded event so you can do your own highlighting
	SetLexer(wxSTC_LEX_CONTAINER);

	//Set margin cursor
	for (int margin = 0; margin < GetMarginCount(); margin++)
		SetMarginCursor(margin, wxSTC_CURSORARROW);

	MarkerDefine(eMessage, wxSTC_MARK_SHORTARROW, *wxWHITE, *wxBLACK);
	MarkerDefine(eWarning, wxSTC_MARK_SHORTARROW, *wxWHITE, *wxYELLOW);
	MarkerDefine(eError, wxSTC_MARK_SHORTARROW, *wxWHITE, *wxRED);

	/** Enumeration of commands and child windows. */
	enum
	{
		idcmdUndo = 10,
		idcmdRedo = 11,
		idcmdCut = 12,
		idcmdCopy = 13,
		idcmdPaste = 14,
		idcmdDelete = 15,
		idcmdSelectAll = 16
	};

	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_CTRL, (int) 'A', idcmdSelectAll);
	entries[1].Set(wxACCEL_CTRL, (int) 'C', idcmdCopy);

	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
}

void COutputWindow::SetFontColorSettings(const FontColorSettings& settings)
{
	// Since we don't save enough information to reapply styles to the existing text,
	// clear everything. This is lame, but changing font settings isn't something that
	// happens often enough for it to really make much difference.
	ClearAll();
	SetBackgroundColour(settings.GetColors(FontColorSettings::DisplayItem_Default).backColor);

	SetMarginType(DEF_LINENUMBER_ID, wxSTC_MARGIN_NUMBER);
	SetMarginWidth(DEF_LINENUMBER_ID, 0);

	// set margin as unused
	SetMarginType(DEF_IMAGE_ID, wxSTC_MARGIN_SYMBOL);
	SetMarginMask(DEF_IMAGE_ID, ~(1024 | 256 | 512 | 128 | 64 | wxSTC_MASK_FOLDERS));
	StyleSetBackground(DEF_IMAGE_ID, *wxWHITE);

	SetMarginWidth(DEF_IMAGE_ID, FromDIP(16));
	SetMarginSensitive(DEF_IMAGE_ID, true);

	SetEditable(false);
}

void COutputWindow::OnDoubleClick(wxMouseEvent& event)
{
	wxTextCoord col, row;
	HitTest(event.GetPosition(), &col, &row);

	//wxString line = GetLineText(row);
	//appData->GotoError(line);
}

void COutputWindow::OutputMessage(const wxString& message)
{
	SharedOutput(message, eStatusMessage::eMessage);
}

void COutputWindow::OutputWarning(const wxString& message)
{
	SharedOutput(message, eStatusMessage::eWarning);
}

void COutputWindow::OutputError(const wxString& message)
{
	SharedOutput(message, eStatusMessage::eError);
}

void COutputWindow::SharedOutput(const wxString& message, eStatusMessage status)
{
	int beforeAppendPosition = GetInsertionPoint();
	int beforeAppendLastPosition = GetLastPosition();

	Freeze();

	SetEditable(true);
	AppendText(message + "\n");
	SetEditable(false);

	int nLastLine = GetLineCount() - 2;

	if (status == eMessage) MarkerAdd(nLastLine, eMessage);
	else if (status == eWarning) MarkerAdd(nLastLine, eWarning);
	else if (status == eError) MarkerAdd(nLastLine, eError);

	Thaw();

	SetInsertionPoint(beforeAppendPosition);

	if (beforeAppendPosition == beforeAppendLastPosition)
	{
		SetInsertionPoint(GetLastPosition());
		ShowPosition(GetLastPosition());
		ScrollLines(-1);
	}

	// update output window 
	mainFrame->Update();
}

int COutputWindow::GetCurrentLine() const
{
	long pos = GetInsertionPoint();

	long x, y;
	PositionToXY(pos, &x, &y);

	return y;
}