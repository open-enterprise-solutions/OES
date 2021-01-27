﻿////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : autoComplete window 
////////////////////////////////////////////////////////////////////////////

#include "autocomplectionctrl.h"
#include "window/mainFrame.h"
#include "compiler/debugger/debugServer.h"
#include "compiler/translateModule.h"
#include "compiler/compileModule.h"
#include "metadata/moduleManager/moduleManager.h"
#include "metadata/metadata.h"
#include "common/docInfo.h" 
#include "utils/bitmaps_res.h"
#include "utils/stringutils.h"

#define DEF_LINENUMBER_ID 0
#define DEF_BREAKPOINT_ID 1
#define DEF_FOLDING_ID 2

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                          Styling                                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAutocomplectionCtrl::OnStyleNeeded(wxStyledTextEvent& event)
{
	for (int line_start = LineFromPosition(GetEndStyled()); line_start < GetLineCount(); line_start++)
	{
		//get exact start positions
		int startpos = PositionFromLine(line_start);
		int endpos = GetLineEndPosition(line_start);

		int startfoldlevel = GetFoldLevel(line_start);
		startfoldlevel &= wxSTC_FOLDFLAG_LEVELNUMBERS; //mask out the flags and only use the fold level

		wxString text = GetLine(line_start); //GetTextRange(startpos, endpos);

		//call highlighting function
		HighlightSyntax(startpos, endpos, text);
	}

	//calculate and apply foldings
	CalculateFoldLevels();
}

#define DEFINE_BLOCK(colorindex) \
{\
	wxASSERT((nStartPos) >= 0 && (nStartPos) <= nLength);\
	 if (!aBlocks.size() || aBlocks[aBlocks.size() - 1].m_nCharPos <= (nStartPos)) { \
        CTextBlock text_block;\
        text_block.m_nCharPos = nStartPos;\
        text_block.m_nColorIndex = colorindex;\
        aBlocks.push_back(text_block);\
     }\
} \

void CAutocomplectionCtrl::HighlightSyntax(unsigned int fromPos, unsigned int toPos, wxString &code)
{
	//Syntax coloring overrides
	struct CTextBlock
	{
		unsigned int m_nCharPos;
		unsigned int m_nColorIndex;
	};

	//this vector will hold the start and end position of each word to highlight
	//if you want to highlight more than one, you should pass a whole class or struct containing the offsets
	std::vector<CTextBlock> aBlocks;

	unsigned int nLength = code.length();
	if (!nLength) return;

	CTranslateModule translate;
	translate.Load(code);

	unsigned int nStartPos = 0;

	//вдруг строка начинается с комментария:
	DEFINE_BLOCK(wxSTC_C_COMMENT);

	bool bWasLeftPoint = false;

	while (!translate.IsEnd())
	{
		nStartPos = translate.GetCurrentPos();

		if (translate.IsWord())
		{
			wxString sWord = translate.GetWord();
			if (CTranslateModule::IsKeyWord(sWord) != wxNOT_FOUND && !bWasLeftPoint)
			{
				if (sWord.Left(1) == '#') DEFINE_BLOCK(wxSTC_C_PREPROCESSOR)
				else DEFINE_BLOCK(wxSTC_C_STRING)
			}
			else
			{
				DEFINE_BLOCK(wxSTC_C_WORD)
			}

			bWasLeftPoint = false;
		}
		else if (translate.IsNumber() || translate.IsString() || translate.IsDate())
		{
			if (translate.IsNumber())
			{
				translate.GetNumber();
			}
			else if (translate.IsString())
			{
				translate.GetString();
			}
			else if (translate.IsDate())
			{
				translate.GetDate();
			}
			DEFINE_BLOCK(wxSTC_C_IDENTIFIER);
			bWasLeftPoint = false;
		}
		else
		{
			char c = translate.GetByte();
			DEFINE_BLOCK(wxSTC_C_IDENTIFIER);

			bWasLeftPoint = false;
			if (c == '.') bWasLeftPoint = true;
		}

		//вдруг далее будут идти комментарии:
		nStartPos = translate.GetCurrentPos();
	}

	DEFINE_BLOCK(wxSTC_C_COMMENT)

	{
		//remove old styling
		StartStyling(fromPos); //from here
		SetStyling(toPos - fromPos, 0); //with that length and style -> cleared
	}

	for (unsigned int i = 0; i < aBlocks.size(); i++)
	{
		wxString sTextStart = code.Mid(0, aBlocks[i].m_nCharPos);
		wxString sTextEnd = wxEmptyString;

		if (i != aBlocks.size() - 1) sTextEnd = code.Mid(aBlocks[i].m_nCharPos, aBlocks[i + 1].m_nCharPos);
		else sTextEnd = code.Mid(aBlocks[i].m_nCharPos, toPos);

		const wxCharBuffer csBufferDataStart = sTextStart.utf8_str();
		const wxCharBuffer csBufferDataEnd = sTextEnd.utf8_str();

		StartStyling(fromPos + csBufferDataStart.length());
		SetStyling(csBufferDataEnd.length(), aBlocks[i].m_nColorIndex);
	}
}

void CAutocomplectionCtrl::OnMarginClick(wxStyledTextEvent& event)
{
	int line = LineFromPosition(event.GetPosition());

	switch (event.GetMargin())
	{
	case DEF_BREAKPOINT_ID:
	{
		if (IsEditable()) {

			int dwFlags = MarkerGet(line);

			//Обновляем список точек останова
			wxString sModuleName = m_document->GetFilename();

			if ((dwFlags & (1 << CAutocomplectionCtrl::Breakpoint))) {
				if (debugServer->RemoveBreakpoint(sModuleName, line))
					MarkerDelete(line, CAutocomplectionCtrl::Breakpoint);
			}
			else {
				if (debugServer->ToggleBreakpoint(sModuleName, line))
					MarkerAdd(line, CAutocomplectionCtrl::Breakpoint);
			}
		}
		break;
	}
	case DEF_FOLDING_ID: ToggleFold(line); break;
	}

	event.Skip();
}

void CAutocomplectionCtrl::OnTextChange(wxStyledTextEvent& event)
{
	int modFlags = event.GetModificationType();

	if ((modFlags & (wxSTC_MOD_BEFOREINSERT)) == 0 && 
		(modFlags & (wxSTC_MOD_BEFOREDELETE)) == 0)
		return;

	if (m_bInitialized)
	{
		CMetaModuleObject *m_moduleObject = wxStaticCast(m_document->GetMetaObject(), CMetaModuleObject);

		if (m_moduleObject)
		{
			std::string csCode = GetTextRaw();
			unsigned int length = 0; int patchLine = 0; bool hasChanged = false;

			wxString sInsert = event.GetString();

			if ((modFlags & wxSTC_MOD_BEFOREINSERT) != 0)
			{
				std::string m_strBuffer = sInsert.utf8_str();
				for (auto c : m_strBuffer)
				{
					if (c != ' ' && c != '\t' && c != '\n' && c != '\r') hasChanged = true;
					if (c == '\n') patchLine++;
				}
			}
			else
			{
				std::string m_strBuffer = csCode.substr(event.GetPosition(), event.GetLength());
				for (auto c : m_strBuffer)
				{
					if (c != ' ' && c != '\t' && c != '\n' && c != '\r') hasChanged = true;
					if (c == '\n') patchLine--;
				}
			}

			IMetaObject *metaObject = m_document->GetMetaObject();
			wxASSERT(metaObject);
			IMetadata *metaData = metaObject->GetMetadata();
			wxASSERT(metaData);

			if (hasChanged)
			{
				IModuleInfo *m_pRefData = NULL;

				IModuleManager *moduleManager = metaData->GetModuleManager();
				wxASSERT(moduleManager);

				if (moduleManager->FindCompileModule(m_document->GetMetaObject(), m_pRefData))
				{
					CCompileModule *m_compileModule = m_pRefData->GetCompileModule();
					wxASSERT(m_compileModule);
					if (!m_compileModule->m_bNeedRecompile)
						m_compileModule->m_bNeedRecompile = true;
				}
			}

			metaData->Modify(true);

			if ((modFlags & wxSTC_MOD_BEFOREINSERT) != 0) length = sInsert.Length();
			else length = wxString::FromUTF8(csCode.substr(event.GetPosition(), event.GetLength())).Length();

			if ((modFlags & wxSTC_MOD_BEFOREINSERT) != 0) csCode.insert(event.GetPosition(), sInsert.utf8_str());
			else csCode.erase(event.GetPosition(), event.GetLength());

			wxString sCode = wxString::FromUTF8(csCode);

			unsigned int startPos = 0;
			unsigned int endPos = event.GetPosition();

			std::string m_strBuffer = csCode.substr(0, endPos);

			unsigned int currLine = 0; bool needChangePos = false;

			for (unsigned int i = 0; i < m_strBuffer.size(); i++)
			{
				if (m_strBuffer[i] == '\n') {
					currLine++; startPos = i - 1;
				}
			}

			/*if (startPos != endPos)
			{
				for (auto c : m_strBuffer.substr(startPos, endPos - startPos))
				{
					if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
						needChangePos = true; break;
					}
				}
			}*/

			if (patchLine != 0) {
				debugServer->PatchBreakpoints(m_moduleObject->GetDocPath(), needChangePos ? currLine + 1 : currLine, patchLine);
			}

#ifndef _OLD_TEXT_PARSER 	
			m_precompileModule->Load(sCode);
			m_precompileModule->m_nCurLine = currLine > 0 ? currLine - 1 : 0;
			m_precompileModule->m_nCurPos = wxString::FromUTF8(m_strBuffer.substr(0, startPos)).Length();
			m_precompileModule->PatchLexem(currLine, patchLine, length, modFlags);
#else 
			m_precompileModule->Load(wxString::FromUTF8(csCode));
			m_precompileModule->PrepareLexem();
#endif
			m_moduleObject->SetModuleText(sCode);
		}
	}
}

void CAutocomplectionCtrl::OnKeyDown(wxKeyEvent &event)
{
	if (!IsEditable()) {
		event.Skip(); return;
	}

	switch (event.GetKeyCode())
	{
	case WXK_LEFT: SetEmptySelection(GetCurrentPos() - 1); break;
	case WXK_RIGHT: SetEmptySelection(GetCurrentPos() + 1); break;
	case WXK_UP:
	{
		int currentPos = GetCurrentPos();
		int line = LineFromPosition(currentPos);

		int startPos = PositionFromLine(line);
		int endPos = GetLineEndPosition(line);

		int length = currentPos - startPos;

		int startNewPos = PositionFromLine(line - 1);
		int endNewPos = GetLineEndPosition(line - 1);

		if (endNewPos - startNewPos < length)
		{
			std::string m_stringBuffer;

			for (int c = 0; c < (length - (endNewPos - startNewPos)); c++)
			{
				m_stringBuffer.push_back(' ');
			}

			InsertText(endNewPos, m_stringBuffer);
		}

		SetEmptySelection(startNewPos + length); break;
	}
	case WXK_DOWN:
	{
		int currentPos = GetCurrentPos();
		int line = LineFromPosition(currentPos);

		int startPos = PositionFromLine(line);
		int endPos = GetLineEndPosition(line);

		int length = currentPos - startPos;

		int startNewPos = PositionFromLine(line + 1);
		int endNewPos = GetLineEndPosition(line + 1);

		if (endNewPos - startNewPos < length)
		{
			std::string m_stringBuffer;

			for (int c = 0; c < (length - (endNewPos - startNewPos)); c++)
			{
				m_stringBuffer.push_back(' ');
			}

			InsertText(endNewPos, m_stringBuffer);
		}

		SetEmptySelection(startNewPos + length); break;
	}
	case WXK_RETURN: PrepareTABs(); break;
	case ' ': if (m_bEnableAutoComplete && event.ControlDown()) LoadAutoComplete(); event.Skip(); break;
	case '9': if (m_bEnableAutoComplete && event.ShiftDown()) LoadCallTip(); event.Skip(); break;
	case '0': if (m_bEnableAutoComplete && event.ShiftDown()) ct.Cancel(); event.Skip(); break;
	default: event.Skip(); break;
	}
}

void CAutocomplectionCtrl::OnDebugEvent(wxDebugEvent& event)
{
	switch (event.GetEventId())
	{
	case EventId_SessionStart: MarkerDeleteAll(CAutocomplectionCtrl::BreakLine); break;
	case EventId_EnterLoop:
	{
		wxString sModuleName = event.GetModuleName();
		if (sModuleName == m_document->GetFilename()) {
			UpdateBreakpoints(true); SetCurrentLine(event.GetLine(), true);
		}
		break;
	}
	case EventId_LeaveLoop: MarkerDeleteAll(CAutocomplectionCtrl::BreakLine); SetToolTip(NULL); m_aExpressions.clear(); break;
	case EventId_SessionEnd: MarkerDeleteAll(CAutocomplectionCtrl::BreakLine); SetToolTip(NULL); m_aExpressions.clear(); break;
	}
}

void CAutocomplectionCtrl::OnDebugToolTipEvent(wxDebugToolTipEvent &event)
{
	wxString sModuleName = event.GetModuleName();
	switch (event.GetEventId())
	{
	case EventId_SetToolTip:
	{
		if (sModuleName == m_document->GetFilename()) {
			m_aExpressions[StringUtils::MakeUpper(event.GetExpression())] = event.GetResult(); SetToolTip(event.GetResult());
		}
		break;
	}
	}
}

void CAutocomplectionCtrl::OnDebugAutocompleteEvent(wxDebugAutocompleteEvent &event)
{
	switch (event.GetEventId())
	{
	case EventId_StartAutocomplete: break;
	case EventId_ShowAutocomplete:
	{
		ac.Start(event.GetKeyWord(), event.GetCurrentPos(), event.GetKeyWord().Length(), TextHeight(GetCurrentLine()));

		wxPoint position = PointFromPosition(event.GetCurrentPos());
		position.y += TextHeight(GetCurrentLine());
		ac.Show(position); break;
	}
	}
}

void CAutocomplectionCtrl::OnMouseMove(wxMouseEvent& event)
{
	LoadToolTip(event.GetPosition()); event.Skip();
}

void CAutocomplectionCtrl::EditDebugPoint(int line)
{
	//Обновляем список точек останова
	wxString sModuleName = m_document->GetFilename();

	int dwFlags = MarkerGet(line);

	if ((dwFlags & (1 << CAutocomplectionCtrl::Breakpoint))) debugServer->RemoveBreakpoint(sModuleName, line);
	else debugServer->ToggleBreakpoint(sModuleName, line);
}

void CAutocomplectionCtrl::UpdateBreakpoints(bool deleteCurrentBreakline)
{
	MarkerDeleteAll(CAutocomplectionCtrl::Breakpoint);

	//Обновляем список точек останова
	wxString sModuleName = m_document->GetFilename();

	for (auto line : debugServer->GetDebugList(sModuleName))
	{
		int dwFlags = MarkerGet(line);

		if (!(dwFlags & (1 << CAutocomplectionCtrl::Breakpoint))) MarkerAdd(line, CAutocomplectionCtrl::Breakpoint);
	}
}

void CAutocomplectionCtrl::SetCurrentLine(int lineBreakpoint, bool setBreakLine)
{
	MarkerDeleteAll(CAutocomplectionCtrl::BreakLine);

	if (setBreakLine) MarkerAdd(lineBreakpoint - 1, CAutocomplectionCtrl::BreakLine);

	int nFirstVisibleLine = GetFirstVisibleLine(), nLinesOnScreen = LinesOnScreen();

	if (nFirstVisibleLine > (lineBreakpoint - 1)) ScrollToLine(lineBreakpoint - 1);
	else if (nFirstVisibleLine + nLinesOnScreen < (lineBreakpoint - 1)) ScrollToLine(lineBreakpoint - 1);
}

void CAutocomplectionCtrl::SetEditorSettings(const EditorSettings & settings)
{
	m_bIndentationSize = settings.GetIndentSize();

	SetIndent(m_bIndentationSize);
	SetTabWidth(m_bIndentationSize);

	bool useTabs = settings.GetUseTabs();
	bool showWhiteSpace = settings.GetShowWhiteSpace();

	SetUseTabs(useTabs);
	SetTabIndents(useTabs);
	SetBackSpaceUnIndents(useTabs);
	SetViewWhiteSpace(showWhiteSpace);

	SetMarginType(DEF_LINENUMBER_ID, wxSTC_MARGIN_NUMBER);
	SetMarginWidth(DEF_LINENUMBER_ID, 0);

	if (settings.GetShowLineNumbers())
	{
		// Figure out how wide the margin needs to be do display
		// the most number of linqes we'd reasonbly have.
		int marginSize = TextWidth(wxSTC_STYLE_LINENUMBER, "_999999");
		SetMarginWidth(DEF_LINENUMBER_ID, marginSize);
	}

	// set margin as unused
	SetMarginType(DEF_BREAKPOINT_ID, wxSTC_MARGIN_SYMBOL);
	SetMarginMask(DEF_BREAKPOINT_ID, ~(1024 | 256 | 512 | 128 | 64 | wxSTC_MASK_FOLDERS));
	StyleSetBackground(DEF_BREAKPOINT_ID, *wxWHITE);

	SetMarginWidth(DEF_BREAKPOINT_ID, 0);
	SetMarginSensitive(DEF_BREAKPOINT_ID, false);

	if (true)
	{
		int foldingMargin = FromDIP(16);

		SetMarginWidth(DEF_BREAKPOINT_ID, foldingMargin);
		SetMarginSensitive(DEF_BREAKPOINT_ID, true);
	}

	// folding
	SetMarginType(DEF_FOLDING_ID, wxSTC_MARGIN_SYMBOL);
	SetMarginMask(DEF_FOLDING_ID, wxSTC_MASK_FOLDERS);

	SetMarginWidth(DEF_FOLDING_ID, 0);
	SetMarginSensitive(DEF_FOLDING_ID, false);

	if (true)
	{
		int foldingMargin = FromDIP(16);

		SetMarginWidth(DEF_FOLDING_ID, foldingMargin);
		SetMarginSensitive(DEF_FOLDING_ID, true);
	}

	m_bEnableAutoComplete = settings.GetEnableAutoComplete();
}

inline wxColour GetInverse(const wxColour& color)
{
	unsigned char r = color.Red();
	unsigned char g = color.Green();
	unsigned char b = color.Blue();

	return wxColour(r ^ 0xFF, g ^ 0xFF, b ^ 0xFF);
}

void CAutocomplectionCtrl::SetFontColorSettings(const FontColorSettings &settings)
{
	// For some reason StyleSetFont takes a (non-const) reference, so we need to make
	// a copy before passing it in.
	wxFont font = settings.GetFont();

	StyleClearAll();
	StyleSetFont(wxSTC_STYLE_DEFAULT, font);

	SetSelForeground(true, settings.GetColors(FontColorSettings::DisplayItem_Selection).foreColor);
	SetSelBackground(true, settings.GetColors(FontColorSettings::DisplayItem_Selection).backColor);

	font = settings.GetFont(FontColorSettings::DisplayItem_Default);

	StyleSetFont(wxSTC_C_DEFAULT, font);
	StyleSetFont(wxSTC_C_IDENTIFIER, font);

	StyleSetForeground(wxSTC_C_DEFAULT, settings.GetColors(FontColorSettings::DisplayItem_Default).foreColor);
	StyleSetBackground(wxSTC_C_DEFAULT, settings.GetColors(FontColorSettings::DisplayItem_Default).backColor);

	StyleSetForeground(wxSTC_STYLE_DEFAULT, settings.GetColors(FontColorSettings::DisplayItem_Default).foreColor);
	StyleSetBackground(wxSTC_STYLE_DEFAULT, settings.GetColors(FontColorSettings::DisplayItem_Default).backColor);

	StyleSetForeground(wxSTC_C_IDENTIFIER, settings.GetColors(FontColorSettings::DisplayItem_Default).foreColor);
	StyleSetBackground(wxSTC_C_IDENTIFIER, settings.GetColors(FontColorSettings::DisplayItem_Default).backColor);

	font = settings.GetFont(FontColorSettings::DisplayItem_Comment);

	StyleSetFont(wxSTC_C_COMMENT, font);
	StyleSetFont(wxSTC_C_COMMENTLINE, font);
	StyleSetFont(wxSTC_C_COMMENTDOC, font);

	StyleSetForeground(wxSTC_C_COMMENT, settings.GetColors(FontColorSettings::DisplayItem_Comment).foreColor);
	StyleSetBackground(wxSTC_C_COMMENT, settings.GetColors(FontColorSettings::DisplayItem_Comment).backColor);

	StyleSetForeground(wxSTC_C_COMMENTLINE, settings.GetColors(FontColorSettings::DisplayItem_Comment).foreColor);
	StyleSetBackground(wxSTC_C_COMMENTLINE, settings.GetColors(FontColorSettings::DisplayItem_Comment).backColor);

	StyleSetForeground(wxSTC_C_COMMENTDOC, settings.GetColors(FontColorSettings::DisplayItem_Comment).foreColor);
	StyleSetBackground(wxSTC_C_COMMENTDOC, settings.GetColors(FontColorSettings::DisplayItem_Comment).backColor);

	font = settings.GetFont(FontColorSettings::DisplayItem_Keyword);

	StyleSetFont(wxSTC_C_WORD, font);
	StyleSetForeground(wxSTC_C_WORD, settings.GetColors(FontColorSettings::DisplayItem_Keyword).foreColor);
	StyleSetBackground(wxSTC_C_WORD, settings.GetColors(FontColorSettings::DisplayItem_Keyword).backColor);

	font = settings.GetFont(FontColorSettings::DisplayItem_Operator);
	StyleSetFont(wxSTC_C_OPERATOR, font);
	StyleSetForeground(wxSTC_C_OPERATOR, settings.GetColors(FontColorSettings::DisplayItem_Operator).foreColor);
	StyleSetBackground(wxSTC_C_OPERATOR, settings.GetColors(FontColorSettings::DisplayItem_Operator).backColor);

	font = settings.GetFont(FontColorSettings::DisplayItem_String);

	StyleSetFont(wxSTC_C_STRING, font);
	StyleSetForeground(wxSTC_C_STRING, settings.GetColors(FontColorSettings::DisplayItem_String).foreColor);
	StyleSetBackground(wxSTC_C_STRING, settings.GetColors(FontColorSettings::DisplayItem_String).backColor);

	StyleSetFont(wxSTC_C_STRINGEOL, font);
	StyleSetForeground(wxSTC_C_STRINGEOL, settings.GetColors(FontColorSettings::DisplayItem_String).foreColor);
	StyleSetBackground(wxSTC_C_STRINGEOL, settings.GetColors(FontColorSettings::DisplayItem_String).backColor);

	StyleSetFont(wxSTC_C_CHARACTER, font);
	StyleSetForeground(wxSTC_C_CHARACTER, settings.GetColors(FontColorSettings::DisplayItem_String).foreColor);
	StyleSetBackground(wxSTC_C_CHARACTER, settings.GetColors(FontColorSettings::DisplayItem_String).backColor);

	font = settings.GetFont(FontColorSettings::DisplayItem_Number);

	StyleSetFont(wxSTC_C_NUMBER, font);
	StyleSetForeground(wxSTC_C_NUMBER, settings.GetColors(FontColorSettings::DisplayItem_Number).foreColor);
	StyleSetBackground(wxSTC_C_NUMBER, settings.GetColors(FontColorSettings::DisplayItem_Number).backColor);

	StyleSetSize(wxSTC_STYLE_LINENUMBER, font.GetPointSize());

	// Set the caret color as the inverse of the background color so it's always visible.
	SetCaretForeground(GetInverse(settings.GetColors(FontColorSettings::DisplayItem_Default).backColor));
}

CAutocomplectionCtrl::CAutocomplectionCtrl() : wxStyledTextCtrl(), ac(this), ct(this) {}

CAutocomplectionCtrl::CAutocomplectionCtrl(CDocument *document, wxWindow *parent, wxWindowID id, const wxPoint & pos, const wxSize & size, long style, const wxString & name)
	: wxStyledTextCtrl(parent, id, pos, size, style, name), m_document(document), ac(this), ct(this), m_precompileModule(NULL), m_bInitialized(false)
{
	debugServer->AddHandler(this);

	// initialize styles
	StyleClearAll();

	//set Lexer to LEX_CONTAINER: This will trigger the styleneeded event so you can do your own highlighting
	SetLexer(wxSTC_LEX_CONTAINER);

	//Set margin cursor
	for (int margin = 0; margin < GetMarginCount(); margin++)
		SetMarginCursor(margin, wxSTC_CURSORARROW);

	//register event
	Connect(wxEVT_STC_MARGINCLICK, wxStyledTextEventHandler(CAutocomplectionCtrl::OnMarginClick), NULL, this);
	Connect(wxEVT_STC_STYLENEEDED, wxStyledTextEventHandler(CAutocomplectionCtrl::OnStyleNeeded), NULL, this);
	Connect(wxEVT_STC_MODIFIED, wxStyledTextEventHandler(CAutocomplectionCtrl::OnTextChange), NULL, this);

	Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(CAutocomplectionCtrl::OnKeyDown), NULL, this);

	Connect(wxEVT_DEBUG_EVENT, wxDebugEventHandler(CAutocomplectionCtrl::OnDebugEvent), NULL, this);
	Connect(wxEVT_DEBUG_TOOLTIP_EVENT, wxDebugToolTipEventHandler(CAutocomplectionCtrl::OnDebugToolTipEvent), NULL, this);
	Connect(wxEVT_DEBUG_AUTOCOMPLETE_EVENT, wxDebugAutocompleteEventHandler(CAutocomplectionCtrl::OnDebugAutocompleteEvent), NULL, this);

	Connect(wxEVT_MOTION, wxMouseEventHandler(CAutocomplectionCtrl::OnMouseMove), NULL, this);

	//set edge mode
	SetEdgeMode(wxSTC_EDGE_MULTILINE);

	// set visibility
	SetVisiblePolicy(wxSTC_VISIBLE_STRICT | wxSTC_VISIBLE_SLOP, 1);
	SetXCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);
	SetYCaretPolicy(wxSTC_CARET_EVEN | wxSTC_VISIBLE_STRICT | wxSTC_CARET_SLOP, 1);

	// Set the marker bitmaps.
	MarkerDefineBitmap(Breakpoint, wxMEMORY_BITMAP(Breakpoint_png));
	MarkerDefineBitmap(CurrentLine, wxMEMORY_BITMAP(Currentline_png));
	MarkerDefineBitmap(BreakLine, wxMEMORY_BITMAP(Breakline_png));

	//markers
	MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS, *wxWHITE, *wxBLACK);
	MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS, *wxWHITE, *wxBLACK);
	MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE, *wxWHITE, *wxBLACK);
	MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUSCONNECTED, *wxWHITE, *wxBLACK);
	MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUSCONNECTED, *wxWHITE, *wxBLACK);
	MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER, *wxWHITE, *wxBLACK);
	MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER, *wxWHITE, *wxBLACK);

	// annotations
	AnnotationSetVisible(wxSTC_ANNOTATION_BOXED);

	// Set fold flags
	SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

	// Setup the dwell time before a tooltip is displayed.
	SetMouseDwellTime(300);

	// Setup caret line
	SetCaretLineVisible(true);

	// miscellaneous
	SetLayoutCache(wxSTC_CACHE_PAGE);

	//Turn the fold markers red when the caret is a line in the group (optional)
	MarkerEnableHighlight(true);
}

bool CAutocomplectionCtrl::LoadModule()
{
	ClearAll();

	wxDELETE(m_precompileModule);

	if (m_document)
	{
		CMetaModuleObject *moduleObject = wxStaticCast(m_document->GetMetaObject(), CMetaModuleObject);
		wxASSERT(moduleObject);
		m_precompileModule = new CPrecompileModule(moduleObject);

		if (IsEditable()) {
			SetText(moduleObject->GetModuleText()); m_bInitialized = true;
		}
		else {
			SetReadOnly(false);
			SetText(moduleObject->GetModuleText()); m_bInitialized = true;
			SetReadOnly(true);
		}

		m_precompileModule->Load(moduleObject->GetModuleText());

		try
		{
			m_precompileModule->PrepareLexem();
		}
		catch (...)
		{
		}
	}

	EmptyUndoBuffer();
	return m_document != NULL;
}

bool CAutocomplectionCtrl::SaveModule()
{
	if (m_document)
	{
		CMetaModuleObject *m_moduleObject = wxStaticCast(m_document->GetMetaObject(), CMetaModuleObject);

		wxASSERT(m_moduleObject);

		if (m_moduleObject) {
			m_moduleObject->SetModuleText(GetText());
		}
	}

	return m_document != NULL;
}

int CAutocomplectionCtrl::GetRealPosition()
{
	wxString sCode = GetTextRange(0, GetCurrentPos());
	return sCode.Length();
}

int CAutocomplectionCtrl::GetRealPositionFromPoint(wxPoint pt)
{
	int currentPos = PositionFromPoint(pt);
	wxString sCode = GetTextRange(0, currentPos);
	return sCode.Length();
}

CAutocomplectionCtrl::~CAutocomplectionCtrl()
{
	if (m_precompileModule) delete m_precompileModule;

	//Events: 
	Disconnect(wxEVT_STC_MARGINCLICK, wxStyledTextEventHandler(CAutocomplectionCtrl::OnMarginClick), NULL, this);
	Disconnect(wxEVT_STC_STYLENEEDED, wxStyledTextEventHandler(CAutocomplectionCtrl::OnStyleNeeded), NULL, this);
	Disconnect(wxEVT_STC_MODIFIED, wxStyledTextEventHandler(CAutocomplectionCtrl::OnTextChange), NULL, this);

	Disconnect(wxEVT_KEY_DOWN, wxKeyEventHandler(CAutocomplectionCtrl::OnKeyDown), NULL, this);

	Disconnect(wxEVT_DEBUG_EVENT, wxDebugEventHandler(CAutocomplectionCtrl::OnDebugEvent), NULL, this);
	Disconnect(wxEVT_DEBUG_TOOLTIP_EVENT, wxDebugToolTipEventHandler(CAutocomplectionCtrl::OnDebugToolTipEvent), NULL, this);
	Disconnect(wxEVT_DEBUG_AUTOCOMPLETE_EVENT, wxDebugAutocompleteEventHandler(CAutocomplectionCtrl::OnDebugAutocompleteEvent), NULL, this);

	Disconnect(wxEVT_MOTION, wxMouseEventHandler(CAutocomplectionCtrl::OnMouseMove), NULL, this);

	//remove handler from debugger
	debugServer->RemoveHandler(this);
}
