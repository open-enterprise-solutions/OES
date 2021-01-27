////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxwidgets community
//	Description : main frame window
////////////////////////////////////////////////////////////////////////////

#include "mainFrame.h" 
#include "metadata/metadata.h"
#include "common/reportManager.h"

//********************************************************************************
//*                                    System                                    *
//********************************************************************************

void CMainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	this->Close();
}

void CMainFrame::OnCloseWindow(wxCloseEvent& event)
{
	bool allowClose = true;

	if (metadata->IsModified())
	{
		if (wxMessageBox("Configuration '" + metadata->GetMetadataName() + "' has been changed. Save?", wxT("Save project"), wxYES_NO | wxCENTRE | wxICON_QUESTION, this) == wxYES)
			allowClose = metadata->SaveMetadata();
	}

	if (allowClose)
	{
		if (metadata->CloseMetadata())
			allowClose = true;//reportManager->Clear(!event.CanVeto());
		else
			allowClose = false; //программа не разрешила закрывать		
	}

	// The user decided not to close finally, abort.
	if (allowClose) event.Skip();
	// Just skip the event, base class handler will destroy the window.
	else event.Veto();
}
