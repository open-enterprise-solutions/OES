////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko 
//	Description : document/view implementation for metaObject 
////////////////////////////////////////////////////////////////////////////

#include "docInfo.h"
#include "appData.h"
#include "cmdProc.h"
#include "window/mainFrame.h"

wxIMPLEMENT_CLASS(CDocument, wxDocument);
wxIMPLEMENT_CLASS(CView, wxView);

//******************************************************************************
//*                            Document implementation                         *
//******************************************************************************

#include "metadata/metaObjectsDefines.h"

CView *CDocument::DoCreateView()
{
	wxClassInfo *m_viewClassInfo = m_documentTemplate->GetViewClassInfo();
	if (!m_viewClassInfo)
		return NULL;

	return static_cast<CView *>(m_viewClassInfo->CreateObject());
}

CCommandProcessor *CDocument::CreateCommandProcessor()
{
	CCommandProcessor *m_commandprocessor = new CCommandProcessor();
	m_commandprocessor->SetEditMenu(mainFrame->GetDefaultMenu(wxID_EDIT));
	m_commandprocessor->Initialize();
	return  m_commandprocessor;
}

CCommandProcessor *CDocument::GetCommandProcessor()
{
	return dynamic_cast<CCommandProcessor *>(m_commandProcessor);
}

IMetaObject *CDocument::GetMetaObject()
{
	return m_metaObject;
}

wxString CDocument::GetModuleName()
{
	return m_metaObject->GetFullName();
}

CDocument::CDocument(CDocument *docParent) :
	wxDocument(), m_metaObject(NULL)
{
	m_docParent = docParent;

	if (docParent)
		docParent->m_childDocs.push_back(this);
}

CDocument::~CDocument()
{
	if (m_docParent)
		m_docParent->m_childDocs.remove(this);
}

bool CDocument::OnCreate(const wxString& path, long flags)
{
	wxScopedPtr<CView> view(DoCreateView());
	if (!view)
		return false;

	view->SetDocument(this);

	CMainFrame::CreateChildFrame(view.get(), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);

	if (!view->OnCreate(this, flags))
		return false;

	view->ShowFrame();
	return view.release() != NULL;
}

bool CDocument::OnSaveModified()
{
	return wxDocument::OnSaveModified();
}

bool CDocument::OnSaveDocument(const wxString& filename)
{
	if (m_metaObject)
		return true;

	return wxDocument::OnSaveDocument(filename);
}

#include "reportManager.h"
#include "window/metatree/metatreeWnd.h"

bool CDocument::OnCloseDocument()
{
	if (!m_docParent) {
		reportManager->RemoveDocument(this);
	}

	if (appData->IsDesignerMode())
	{
		if (m_metaObject)
		{
			IMetadata *metaData = m_metaObject->GetMetadata();
			wxASSERT(metaData);
			IMetadataTree *metaTree = metaData->GetMetaTree();
			wxASSERT(metaTree);
			metaTree->OnCloseDocument(this);
		}
	}

	return wxDocument::OnCloseDocument();
}

void CDocument::Modify(bool modify)
{
	if (modify != m_documentModified)
	{
		m_documentModified = modify;

		// Allow views to append asterix to the title
		wxView* view = GetFirstView();
		if (view) view->OnChangeFilename();
	}

	if (m_documentModified)
	{
		if (m_metaObject 
			&& appData->IsDesignerMode())
		{
			IMetadata *metaData = m_metaObject->GetMetadata();
			wxASSERT(metaData);
			metaData->Modify(true);
		}
	}
}

bool CDocument::Save()
{
	if (AlreadySaved())
		return true;

	if (m_docParent)
		m_docParent->Save();

	if (m_documentFile.empty() || !m_savedYet)
		return SaveAs();

	return OnSaveDocument(m_documentFile);
}

bool CDocument::SaveAs()
{
	if (m_metaObject)
		return true;

	return wxDocument::SaveAs();
}

bool CDocument::Close()
{
	if (!OnSaveModified())
		return false;

	// When the parent document closes, its children must be closed as well as
	// they can't exist without the parent.

	// As usual, first check if all children can be closed.
	wxDList<CDocument>::const_iterator it = m_childDocs.begin();
	for (wxDList<CDocument>::const_iterator end = m_childDocs.end(); it != end; ++it)
	{
		if (!(*it)->OnSaveModified())
		{
			// Leave the parent document opened if a child can't close.
			return false;
		}
	}

	// Now that they all did, do close them: as m_childDocs is modified as
	// we iterate over it, don't use the usual for-style iteration here.
	while (!m_childDocs.empty())
	{
		CDocument *const childDoc = m_childDocs.front();

		// This will call OnSaveModified() once again but it shouldn't do
		// anything as the document was just saved or marked as not needing to
		// be saved by the call to OnSaveModified() that returned true above.
		if (!childDoc->Close())
		{
			wxFAIL_MSG("Closing the child document unexpectedly failed "
				"after its OnSaveModified() returned true");
		}

		// Delete the child document by deleting all its views.
		childDoc->DeleteAllViews();
	}

	return OnCloseDocument();
}
