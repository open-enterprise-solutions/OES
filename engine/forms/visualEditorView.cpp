////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxFormBuilder
//	Description : visual view  
////////////////////////////////////////////////////////////////////////////

#include "visualEditorView.h"
#include <wx/collpane.h>

void CVisualView::CreateFrame()
{
#if !defined(__WXGTK__ )
	Freeze();   // Prevent flickering on wx 2.8,
				// Causes problems on wx 2.9 in wxGTK (e.g. wxNoteBook objects)
#endif

	Enable(m_valueFrame ? m_valueFrame->m_enabled : false);

	if (m_valueFrame && m_valueFrame->IsShown())
	{
		// --- [1] Set the color of the form -------------------------------
		if (m_valueFrame->m_bg.IsOk())
		{
			SetBackgroundColour(m_valueFrame->m_bg);
		}
		else
		{
			SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
		}

		// --- [2] Title bar Setup
		if (m_document && !IsDemonstration())
		{
			m_document->SetFilename(m_valueFrame->m_title, true);
			m_document->SetTitle(m_valueFrame->m_title);
		}

		// --- [3] Default sizer Setup 
		m_mainBoxSizer = new wxBoxSizer(m_valueFrame->m_orient);
		SetSizer(m_mainBoxSizer);

		// --- [4] Create the components of the form -------------------------
		// Used to save valueFrame objects for later display
		IControlElement *menubar = NULL;
		wxWindow* statusbar = NULL;
		wxWindow* toolbar = NULL;

		for (unsigned int i = 0; i < m_valueFrame->GetChildCount(); i++)
		{
			IControlElement *child = m_valueFrame->GetChild(i);

			if (!menubar && (m_valueFrame->GetObjectTypeName() == wxT("menubar_form")))
			{
				// main form acts as a menubar
				menubar = m_valueFrame;
			}
			else if (child->GetObjectTypeName() == wxT("menubar"))
			{
				// Create the menubar later
				menubar = child;
			}
			else
			{
				// Recursively generate the ObjectTree
				try
				{
					// we have to put the content valueFrame panel as parentObject in order
					// to SetSizeHints be called.
					GenerateControl(child, this, GetFrameSizer());
				}
				catch (std::exception& ex)
				{
					wxLogError(ex.what());
				}
			}
		}

		Layout();

		m_mainBoxSizer->Fit(this);
		SetClientSize(GetBestSize());

		Refresh();
	}
	else
	{
		// There is no form to display
		Show(false);
		Refresh();
	}

#if !defined(__WXGTK__)
	Thaw();
#endif

	UpdateVirtualSize();
}

void CVisualView::UpdateFrame()
{
#if !defined(__WXGTK__ )
	Freeze();   // Prevent flickering on wx 2.8,
				// Causes problems on wx 2.9 in wxGTK (e.g. wxNoteBook objects)
#endif

	Enable(m_valueFrame->m_enabled);

	if (m_valueFrame &&
		m_valueFrame->IsShown())
	{
		// --- [1] Set the color of the form -------------------------------
		if (m_valueFrame->m_bg.IsOk())
		{
			SetBackgroundColour(m_valueFrame->m_bg);
		}
		else
		{
			SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
		}

		// --- [2] Title bar Setup
		if (m_document && !IsDemonstration())
		{
			m_document->SetFilename(m_valueFrame->m_title, true);
			m_document->SetTitle(m_valueFrame->m_title);
		}

		// --- [3] Default sizer Setup 
		m_mainBoxSizer->SetOrientation(m_valueFrame->m_orient);

		// --- [4] Create the components of the form -------------------------
		// Used to save valueFrame objects for later display
		IControlElement *menubar = NULL;
		wxWindow* statusbar = NULL;
		wxWindow* toolbar = NULL;

		for (unsigned int i = 0; i < m_valueFrame->GetChildCount(); i++)
		{
			IControlElement *child = m_valueFrame->GetChild(i);

			if (!menubar && (m_valueFrame->GetObjectTypeName() == wxT("menubar_form")))
			{
				// main form acts as a menubar
				menubar = m_valueFrame;
			}
			else if (child->GetObjectTypeName() == wxT("menubar"))
			{
				// Create the menubar later
				menubar = child;
			}
			else
			{
				// Recursively generate the ObjectTree
				try
				{
					// we have to put the content valueFrame panel as parentObject in order
					// to SetSizeHints be called.
					RefreshControl(child, this, GetFrameSizer());
				}
				catch (std::exception& ex)
				{
					wxLogError(ex.what());
				}
			}
		}

		Layout();

		m_mainBoxSizer->Fit(this);
		SetClientSize(GetBestSize());

		Refresh();
	}
	else
	{
		// There is no form to display
		Show(false);
		Refresh();
	}

#if !defined(__WXGTK__)
	Thaw();
#endif

	UpdateVirtualSize();
}

#include "common/reportManager.h"

CVisualView::~CVisualView()
{
	for (unsigned int i = 0; i < m_valueFrame->GetChildCount(); i++)
	{
		IControlElement *objChild = m_valueFrame->GetChild(i);
		DeleteRecursive(objChild, true);
	}
}