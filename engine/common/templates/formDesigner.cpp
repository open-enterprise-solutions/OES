#include "formDesigner.h"
#include "compiler/systemObjects.h"
#include "common/objectbase.h"
#include "utils/typeconv.h"
#include "appData.h"
#include "window/mainFrame.h"
#include "forms/visualEditor.h"
#include "metadata/metaObjects/metaObject.h"
#include "metadata/metadata.h"
#include "window/lineInput.h"
#include "window/property/objinspect.h"
#include "window/functionlist/functionlist.h"

wxIMPLEMENT_DYNAMIC_CLASS(CFormEditView, CView);

// ----------------------------------------------------------------------------
// CTextEditView implementation
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(CFormEditView, CView)
EVT_MENU(wxID_COPY, CFormEditView::OnCopy)
EVT_MENU(wxID_PASTE, CFormEditView::OnPaste)
EVT_MENU(wxID_SELECTALL, CFormEditView::OnSelectAll)
EVT_FRAME_EVENT_HANDLER_MODIFIED(CFormEditView::OnEventHandlerModified)
wxEND_EVENT_TABLE()

#define wxID_RUN_FORM 15001

static wxWindowID nextId = wxID_HIGHEST + 3000;

wxMenu *CreateMenuForm()
{
	// and its menu bar
	wxMenu *menuForm = new wxMenu();
	menuForm->Append(wxID_RUN_FORM, _("Run form"));
	return menuForm;
}

extern wxImageList *GetImageList();

#include "window/theme/luna_auitabart.h"

bool CFormEditView::OnCreate(CDocument *doc, long flags)
{
	m_notebook = new wxAuiNotebook(m_viewFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_BOTTOM | wxAUI_NB_TAB_FIXED_WIDTH);
	m_notebook->SetArtProvider(new CLunaTabArt());
	m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &CFormEditView::OnPageChanged, this);

	InitializeFormDesigner(flags);
	InitializeCodeView(flags);

	m_notebook->AddPage(m_visualEditor, wxT("Designer"), false, GetImageList()->GetBitmap(240));
	m_notebook->AddPage(m_code, wxT("Code"), false, GetImageList()->GetBitmap(256));

	if (mainFrame->GetMenuBar()->FindMenu(wxT("Forms")) == wxNOT_FOUND)
	{
		mainFrame->Connect(wxEVT_MENU, wxCommandEventHandler(CFormEditView::OnMenuClick), NULL, this);
		mainFrame->GetMenuBar()->Insert(2, CreateMenuForm(), wxT("Forms"));
	}

	m_visualEditor->ActivateObject();
	return CView::OnCreate(doc, flags);
}

enum
{
	wxID_ADD_COMMENTS = wxID_HIGHEST + 10000,
	wxID_REMOVE_COMMENTS,

	wxID_SYSNTAX_CONTROL,

	wxID_GOTOLINE,
	wxID_PROCEDURES_FUNCTIONS

};

#include "forms/visualEditor.h"

void CFormEditView::OnCreateToolbar(wxAuiToolBar *m_toolbar)
{
	if (IsEditorActivate())
	{
		if (!m_toolbar->GetToolCount())
		{
			m_toolbar->AddTool(wxID_ADD_COMMENTS, _("Add comments"), GetImageList()->GetBitmap(622), _("Add"), wxItemKind::wxITEM_NORMAL);
			m_toolbar->EnableTool(wxID_ADD_COMMENTS, m_code->IsEditable());
			m_toolbar->AddTool(wxID_REMOVE_COMMENTS, _("Remove comments"), GetImageList()->GetBitmap(623), _("Remove"), wxItemKind::wxITEM_NORMAL);
			m_toolbar->EnableTool(wxID_REMOVE_COMMENTS, m_code->IsEditable());
			m_toolbar->AddSeparator();
			m_toolbar->AddTool(wxID_SYSNTAX_CONTROL, _("Syntax control"), GetImageList()->GetBitmap(624), _("Syntax"), wxItemKind::wxITEM_NORMAL);
			m_toolbar->EnableTool(wxID_SYSNTAX_CONTROL, m_code->IsEditable());
			m_toolbar->AddSeparator();
			m_toolbar->AddTool(wxID_GOTOLINE, _("Goto line"), GetImageList()->GetBitmap(626), _("Goto"), wxItemKind::wxITEM_NORMAL);
			//m_toolbar->EnableTool(wxID_GOTOLINE, m_code->IsEditable());
			m_toolbar->AddTool(wxID_PROCEDURES_FUNCTIONS, _("Procedures and functions"), GetImageList()->GetBitmap(627), _("Procedures and functions"), wxItemKind::wxITEM_NORMAL);
			//m_toolbar->EnableTool(wxID_PROCEDURES_FUNCTIONS, m_code->IsEditable());
		}
	}
	else
	{
		for (auto controlClass : CValue::GetAvailableObjects(eObjectType::eObjectType_object_control))
		{
			IControlValueAbstract *objectSingle = dynamic_cast<IControlValueAbstract *>(CValue::GetAvailableObject(controlClass));
			wxASSERT(objectSingle);
			wxBitmap m_bitmap = objectSingle->GetControlImage();
			if (m_bitmap.IsOk()) {
				m_toolbar->AddTool(nextId++, controlClass, m_bitmap, controlClass);
				m_toolbar->EnableTool(nextId - 1, m_visualEditor->IsEditable());
			}
			//else m_toolbar->AddTool(nextId++, controlClass, wxArtProvider::GetBitmap(wxART_NEW, wxART_OTHER, wxSize(16, 16)), controlClass);
		}
	}

	m_toolbar->Bind(wxEVT_MENU, &CFormEditView::OnMenuClicked, this);
}

void CFormEditView::OnRemoveToolbar(wxAuiToolBar *m_toolbar)
{
	if (IsEditorActivate())
	{
		m_toolbar->Unbind(wxEVT_MENU, &CFormEditView::OnMenuClicked, this);
	}
}

#define FORM_POS 2

void CFormEditView::OnActivateView(bool activate, wxView *activeView, wxView *deactiveView)
{
	if (activate)
	{
		if (IsDesignerActivate())
		{
			if (mainFrame->GetMenuBar()->FindMenu(wxT("Forms")) == wxNOT_FOUND)
			{
				mainFrame->Connect(wxEVT_MENU, wxCommandEventHandler(CFormEditView::OnMenuClick), NULL, this);
				mainFrame->GetMenuBar()->Insert(2, CreateMenuForm(), wxT("Forms"));
			}

			m_visualEditor->ActivateObject();
		}
		else
		{
			if (mainFrame->GetMenuBar()->FindMenu(wxT("Forms")) != wxNOT_FOUND)
			{
				mainFrame->GetMenuBar()->Remove(2);
				mainFrame->Disconnect(wxEVT_MENU, wxCommandEventHandler(CFormEditView::OnMenuClick), NULL, this);
			}
		}
	}
	else
	{
		if (mainFrame->GetMenuBar()->FindMenu(wxT("Forms")) != wxNOT_FOUND)
		{
			mainFrame->GetMenuBar()->Remove(2);
			mainFrame->Disconnect(wxEVT_MENU, wxCommandEventHandler(CFormEditView::OnMenuClick), NULL, this);
		}
	}

	CView::OnActivateView(activate, activeView, deactiveView);
}

void CFormEditView::OnUpdate(wxView *sender, wxObject *hint)
{
	m_code->SetEditorSettings(mainFrame->GetEditorSettings());
	m_code->SetFontColorSettings(mainFrame->GetFontColorSettings());

	m_code->UpdateBreakpoints();
}

void CFormEditView::OnDraw(wxDC *WXUNUSED(dc))
{
	// nothing to do here, wxTextCtrl draws itself
}

bool CFormEditView::OnClose(bool deleteWindow)
{
	Activate(false);

	if (deleteWindow)
	{
		GetFrame()->Destroy();
		SetFrame(NULL);
	}

	m_visualEditor->DeactivateObject();
	return CView::OnClose(deleteWindow);
}

void CFormEditView::OnCopy(wxCommandEvent& WXUNUSED(event))
{
	if (m_notebook->GetSelection() == 0)
	{
		m_visualEditor->CopyObject(m_visualEditor->GetSelectedObject());
	}
	else if (m_notebook->GetSelection() == 1)
	{
		m_code->Copy();
	}
}

void CFormEditView::OnPaste(wxCommandEvent& WXUNUSED(event))
{
	if (m_notebook->GetSelection() == 0)
	{
		m_visualEditor->PasteObject(m_visualEditor->GetSelectedObject());
	}
	else if (m_notebook->GetSelection() == 1)
	{
		m_code->Paste();
	}
}

void CFormEditView::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
	if (m_notebook->GetSelection() == 0)
	{
		//m_visualEditor->PasteObject(m_visualEditor->GetSelectedObject());
	}
	else if (m_notebook->GetSelection() == 1)
	{
		m_code->SelectAll();
	}
}

#include "window/autocomplete/autoComplectionPrintOut.h"
#include "forms/printout/formPrintOut.h"

wxPrintout *CFormEditView::OnCreatePrintout()
{
	if (IsEditorActivate())
		return new CAutocomplectionPrint(m_code, this->GetViewName());

	return new CFormPrintout(m_visualEditor->GetVisualEditor());
}

void CFormEditView::OnEventHandlerModified(wxFrameEventHandlerEvent &event)
{
	Event *evtHandler = event.GetFrameEventHandler();

	m_code->AppendText("\t\n");
	m_code->AppendText("procedure " + evtHandler->GetValue() + "() \t\n");
	m_code->AppendText("endProcedure \t\n");
	m_code->GotoLine(m_code->GetLineCount() - 2);

	m_notebook->SetSelection(1);
}

void CFormEditView::OnMenuClick(wxCommandEvent &event)
{
	if (event.GetId() == wxID_RUN_FORM)
		m_visualEditor->RunForm();

	event.Skip();
}

void CFormEditView::OnPageChanged(wxAuiNotebookEvent &event)
{
	int selection = event.GetSelection();

	if (selection == 0)
	{
		objectInspector->SelectObject(m_visualEditor->GetSelectedObject(), m_visualEditor->GetEventHandler());
	}
	else if (selection == 1)
	{
		CDocument *metaDoc = GetDocument();

		if (metaDoc)
		{
			IMetaObject *moduleObject = metaDoc->GetMetaObject();
			if (moduleObject) {
				objectInspector->SelectObject(moduleObject);
			}
		}

		m_code->SetSTCFocus(true); 
	}

	Activate(true); event.Skip();
}

void CFormEditView::OnMenuClicked(wxCommandEvent& event)
{
	CDocument *m_docView = dynamic_cast<CDocument *>(m_viewDocument);

	if (IsEditorActivate())
	{
		if (event.GetId() == wxID_ADD_COMMENTS)
		{
			int nStartLine, nEndLine;
			m_code->GetSelection(&nStartLine, &nEndLine);

			for (int line = m_code->LineFromPosition(nStartLine); line <= m_code->LineFromPosition(nEndLine); line++)
			{
				m_code->InsertText(m_code->PositionFromLine(line), "//");
			}
		}
		else if (event.GetId() == wxID_REMOVE_COMMENTS)
		{
			int nStartLine, nEndLine;
			m_code->GetSelection(&nStartLine, &nEndLine);

			for (int line = m_code->LineFromPosition(nStartLine); line <= m_code->LineFromPosition(nEndLine); line++)
			{
				int startPos = m_code->PositionFromLine(line);
				wxString sLine = m_code->GetLineRaw(line);
				for (unsigned int i = 0; i < sLine.length(); i++)
				{
					if (sLine[i] == '/'
						&& (i + 1 < sLine.length() && sLine[i + 1] == '/'))
					{
						m_code->Replace(startPos + i, startPos + i + 2, wxEmptyString); break;
					}
				}
			}
		}
		else if (event.GetId() == wxID_SYSNTAX_CONTROL)
		{
			IMetaObject *metaObject = m_docView->GetMetaObject();
			wxASSERT(metaObject);
			IMetadata *metaData = metaObject->GetMetadata();
			wxASSERT(metaData);
			IModuleManager *moduleManager = metaData->GetModuleManager();
			wxASSERT(moduleManager);

			IModuleInfo *m_dataRef = NULL;

			if (moduleManager->FindCompileModule(metaObject, m_dataRef))
			{
				CCompileModule *m_compileModule = m_dataRef->GetCompileModule();
				try
				{
					if (m_compileModule->Compile()) CSystemObjects::Message(_("No syntax errors detected!"));
					else { wxASSERT("CCompileModule::Compile return false"); }
				}
				catch (const CTranslateError *err)
				{
					CSystemObjects::Message(err->what());
				}
			}
		}
		else if (event.GetId() == wxID_GOTOLINE)
		{
			CLineInput *m_lineInput = new CLineInput(m_code);
			int ret = m_lineInput->ShowModal();
			if (ret != wxNOT_FOUND)
			{
				m_code->SetFocus();
				m_code->GotoLine(ret - 1);
			}
		}
		else if (event.GetId() == wxID_PROCEDURES_FUNCTIONS)
		{
			CFunctionList *m_funcList = new CFunctionList(GetDocument(), m_code);
			int ret = m_funcList->ShowModal();
		}
	}
	else
	{
		wxAuiToolBar *m_tv = mainFrame->GetAdditionalToolbar();

		wxString name = m_tv->GetToolShortHelp(event.GetId());
		m_visualEditor->CreateObject(name);
	}

	//event.Skip();
}

void CFormEditView::InitializeFormDesigner(long flags)
{
	m_visualEditor = new CVisualEditorContextForm(GetDocument(), this, m_notebook);
	m_visualEditor->SetReadOnly(flags == wxDOC_READONLY);
}

void CFormEditView::InitializeCodeView(long flags)
{
	m_code = new CAutocomplectionCtrl(GetDocument(), m_notebook, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);

	m_code->SetEditorSettings(mainFrame->GetEditorSettings());
	m_code->SetFontColorSettings(mainFrame->GetFontColorSettings());

	m_code->SetReadOnly(flags == wxDOC_READONLY);
	m_code->SetSTCFocus(true);
}

bool CFormEditView::LoadForm()
{
	return m_visualEditor->LoadForm() && m_code->LoadModule();
}

bool CFormEditView::SaveForm()
{
	return m_visualEditor->SaveForm() && m_code->SaveModule();
}