////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxwidgets community
//	Description : main frame window
////////////////////////////////////////////////////////////////////////////

#include "mainFrameDesigner.h"
#include "appData.h"
#include "compiler/debugger/debugServer.h"
#include "settings/settingsdialog.h"
#include "settings/keybinderdialog.h" 
#include "settings/fontcolorsettingspanel.h"
#include "settings/editorsettingspanel.h"

//********************************************************************************
//*                                 Debug commands                               *
//********************************************************************************

#include <wx/stdpaths.h>

void CMainFrameDesigner::OnStartDebug(wxCommandEvent& WXUNUSED(event))
{
	if (debugServer->HasConnections()) { wxMessageBox(wxT("Debugger is already running!")); return; }
	wxExecute("engine -m enterprise -d -ib \"" + appData->GetApplicationPath() + "\"");
}

void CMainFrameDesigner::OnStartDebugWithoutDebug(wxCommandEvent& WXUNUSED(event))
{
	wxExecute("engine -m enterprise -ib \"" + appData->GetApplicationPath() + "\"");
}

#include "metadata/metadata.h"
#include "window/metatree/metatreeWnd.h"
#include "window/property/objinspect.h"

void CMainFrameDesigner::OnConfiguration(wxCommandEvent &event)
{
	if (wxID_CONFIGURATION_LOAD == event.GetId())
	{
		wxFileDialog openFileDialog(this, _("Open metaData file"), "", "",
			"metaData files (*.mtd)|*.mtd", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return;     // the user changed idea...

		// proceed loading the file chosen by the user;
		if (metadata->LoadFromFile(openFileDialog.GetPath()))
		{
			objectInspector->ClearProperty();

			if (!m_docManager->CloseDocuments())
				return;

			if (metatreeWnd->Load())
			{
				objectInspector->SelectObject(metadata->GetCommonMetaObject());

				if (metadata->IsModified())
				{
					if (wxMessageBox("Configuration '" + metadata->GetMetadataName() + "' has been changed. Do you want to save?", wxT("Save project"), wxYES_NO | wxCENTRE | wxICON_QUESTION, this) == wxYES)
						metadata->SaveMetadata();
				}
			}
		}
	}
	else if (wxID_CONFIGURATION_SAVE == event.GetId())
	{
		wxFileDialog saveFileDialog(this, _("Save metaData file"), "", "",
			"metaData files (*.mtd)|*.mtd", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (saveFileDialog.ShowModal() == wxID_CANCEL)
			return;     // the user changed idea...

		if (metadata->SaveToFile(saveFileDialog.GetPath()))
		{
			wxMessageBox(_("Successfully unloaded to: ") + saveFileDialog.GetPath());
		}
	}
}

void CMainFrameDesigner::OnRunDebugCommand(wxCommandEvent& event)
{
	int nRunLine = -1;

	switch (event.GetId())
	{
	case wxID_DEBUG_STEP_OVER: debugServer->StepOver(); break;
	case wxID_DEBUG_STEP_INTO: debugServer->StepInto(); break;
	case wxID_DEBUG_PAUSE: debugServer->Pause(); break;
	case wxID_DEBUG_STOP_DEBUGGING: debugServer->Stop(false); break;
	case wxID_DEBUG_STOP_PROGRAM: debugServer->Stop(true); break;
	case wxID_DEBUG_NEXT_POINT: debugServer->Continue();  break;
	case wxID_DEBUG_REMOVE_ALL_DEBUGPOINTS: debugServer->RemoveAllBreakPoints();  break;
	}
}

//********************************************************************************
//*                                    Tool                                      *
//********************************************************************************

void CMainFrameDesigner::OnToolsSettings(wxCommandEvent& event)
{
	SettingsDialog dialog(this);

	KeyBinderDialog* keyBinder = dialog.GetKeyBinderDialog();

	for (unsigned int i = 0; i < m_keyBinder.GetNumCommands(); ++i)
	{
		keyBinder->AddCommand(m_keyBinder.GetCommand(i));
	}

	FontColorSettingsPanel* fontColorSettings = dialog.GetFontColorSettingsPanel();
	fontColorSettings->SetSettings(m_fontColorSettings);

	EditorSettingsPanel* editorSettings = dialog.GetEditorSettingsPanel();
	editorSettings->SetSettings(m_editorSettings);

	if (dialog.ShowModal() == wxID_OK)
	{
		m_keyBinder.ClearCommands();

		for (unsigned int i = 0; i < keyBinder->GetNumCommands(); ++i)
		{
			m_keyBinder.AddCommand(keyBinder->GetCommand(i));
		}

		m_keyBinder.UpdateWindow(this);
		m_keyBinder.UpdateMenuBar(GetMenuBar());

		m_fontColorSettings = fontColorSettings->GetSettings();
		m_editorSettings = editorSettings->GetSettings();

		UpdateEditorOptions();
	}
}

#include "userListWnd.h"

void CMainFrameDesigner::OnUsers(wxCommandEvent& event)
{
	userListWnd *m_usersWnd = new userListWnd(this, wxID_ANY);
	m_usersWnd->Show();
}

//********************************************************************************
//*                                    Toolbar                                   *
//********************************************************************************

void CMainFrameDesigner::OnDebugEvent(wxDebugEvent &event)
{
	if (event.GetEventId() == EventId::EventId_SessionStart)
	{
		//m_menuDebug->Enable(wxID_DEBUG_EDIT_POINT, true);
		m_menuDebug->Enable(wxID_DEBUG_STEP_INTO, true);
		m_menuDebug->Enable(wxID_DEBUG_STEP_OVER, true);
		m_menuDebug->Enable(wxID_DEBUG_PAUSE, true);
		m_menuDebug->Enable(wxID_DEBUG_STOP_DEBUGGING, true);
		m_menuDebug->Enable(wxID_DEBUG_STOP_PROGRAM, true);
		m_menuDebug->Enable(wxID_DEBUG_NEXT_POINT, true);
	}
	else if (event.GetEventId() == EventId::EventId_SessionEnd)
	{
		//m_menuDebug->Enable(wxID_DEBUG_EDIT_POINT, false);
		m_menuDebug->Enable(wxID_DEBUG_STEP_INTO, false);
		m_menuDebug->Enable(wxID_DEBUG_STEP_OVER, false);
		m_menuDebug->Enable(wxID_DEBUG_PAUSE, false);
		m_menuDebug->Enable(wxID_DEBUG_STOP_DEBUGGING, false);
		m_menuDebug->Enable(wxID_DEBUG_STOP_PROGRAM, false);
		m_menuDebug->Enable(wxID_DEBUG_NEXT_POINT, false);
	}
}

#include "metadata/metadata.h"

void CMainFrameDesigner::OnToolbarClicked(wxEvent &event)
{
	if (event.GetId() == 5)
	{
		int m_answer = wxMessageBox("Do you want to save a project '" + metadata->GetMetadataName() + "'?", wxT("Save project"), wxYES_NO | wxCENTRE | wxICON_QUESTION, this);

		if (m_answer == wxYES)
		{
			bool m_bCanSave = true;

			for (auto m_document : m_docManager->GetDocumentsVector())
			{
				if (m_bCanSave) m_bCanSave = m_document->OnSaveModified();
				else break;
			}

			if (m_bCanSave && !appData->SaveProject()) {
				wxMessageBox("Failed to save metadata!", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR, this);
			}
		}
	}

	event.Skip();
}