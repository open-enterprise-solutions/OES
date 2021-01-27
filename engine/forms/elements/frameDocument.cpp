////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : frame docview
////////////////////////////////////////////////////////////////////////////

#include "frame.h"
#include "metadata/objects/baseObject.h"
#include "forms/visualEditorView.h"
#include "forms/printout/formPrintOut.h"
#include "window/mainFrame.h"
#include "window/mainFrameChild.h"

#define st_demonstration _("preview")

class CFormView : public CView
{
	CValueFrame *m_valueFrame;

public:

	CFormView(CValueFrame *valueFrame) : m_valueFrame(valueFrame) {}

	virtual wxPrintout *OnCreatePrintout() override
	{
		CVisualDocument *m_visualDocument = m_valueFrame->GetVisualDocument();
		return m_visualDocument ? new CFormPrintout(m_visualDocument->GetVisualView()) : NULL;
	}

	virtual bool OnClose(bool deleteWindow = true) override
	{
		if (!deleteWindow)
		{
			if (m_valueFrame->m_valueFrameDocument
				&& !m_valueFrame->CloseFrame())
				return false;
		}

		if (CMainFrame::Get())
			Activate(false);

		if (deleteWindow)
		{
			m_viewFrame->Destroy();
			m_viewFrame = NULL;
		}

		return m_viewDocument ? m_viewDocument->Close() : true;
	}

	CValueFrame *GetValueFrame() { return m_valueFrame; }
};

static std::map<const Guid, CVisualDocument *> s_aOpenedForms;

bool CVisualDocument::OnCloseDocument()
{
	CValueFrame *m_valueFrame = m_visualHost ? m_visualHost->GetValueFrame() : NULL;

	if (m_valueFrame)
	{
		m_valueFrame->m_formModified = false;
		m_valueFrame->m_valueFrameDocument = NULL;

		if (!m_visualHost->IsDemonstration())
		{
			m_valueFrame->m_formModified = false;
			m_valueFrame->m_valueFrameDocument = NULL;
			wxDELETE(m_visualHost);

			if (m_valueFrame->GetRefCount() > 1) {
				m_valueFrame->DecrRef();
			}
			else {
				wxTheApp->ScheduleForDestruction(m_valueFrame);
			}

			s_aOpenedForms.erase(m_guidForm);
			m_guidForm.reset();
		}
	}

	return CDocument::OnCloseDocument();
}

bool CVisualDocument::Save()
{
	CValueFrame *m_valueFrame = m_visualHost ? m_visualHost->GetValueFrame() : NULL;
	IDataObjectSource *m_sourceObject = m_valueFrame ? m_valueFrame->GetSourceObject() : NULL;

	if (m_sourceObject && m_sourceObject->SaveModify()) return CDocument::Save();
	else return false;
}

void CVisualDocument::Modify(bool modify)
{
	if (m_visualHost)
	{
		CValueFrame *m_valueFrame = m_visualHost->GetValueFrame();
		if (m_valueFrame) m_valueFrame->m_formModified = modify;
	}

	CDocument::Modify(modify);
}

CVisualDocument::~CVisualDocument()
{
	wxDELETE(m_visualHost);
}

void CValueFrame::SetMetaObject(IMetaFormObject *metaFormObject, IMetaObjectValue *metaObject)
{
	if (m_valueFrameDocument) { m_valueFrameDocument->Close(); m_valueFrameDocument = NULL; }
	m_metaFormObject = metaFormObject; m_metaObject = metaObject;
}

void CValueFrame::SetSourceObject(IDataObjectSource *sourceObject)
{
	if (sourceObject) { sourceObject->IncrRef(); }
	if (m_sourceObject) { m_sourceObject->DecrRef(); }

	m_sourceObject = sourceObject;
}

#include "common/reportManager.h"

bool CValueFrame::ShowDocumentForm(bool demonstration)
{
	if (m_valueFrameDocument) {
		ActivateForm(); return true;
	}

	if (!demonstration)
	{
		m_guidForm.reset();

		if (m_sourceObject)
		{
			std::map<const Guid, CVisualDocument *>::iterator foundedForm = s_aOpenedForms.find(m_sourceObject->GetGuid());

			if (foundedForm != s_aOpenedForms.end())
			{
				CVisualDocument *foundedVisualDocument = foundedForm->second;
				wxASSERT(foundedVisualDocument);
				foundedVisualDocument->Activate();
				return true;
			}

			m_guidForm = m_sourceObject->GetGuid();
		}
		else
		{
			m_guidForm = Guid::newGuid();
		}

		m_valueFrameDocument = new CVisualDocument(m_guidForm);
	}
	else
	{
		m_valueFrameDocument = new CVisualDocument();
	}

	reportManager->AddDocument(m_valueFrameDocument);

	m_valueFrameDocument->SetCommandProcessor(m_valueFrameDocument->CreateCommandProcessor());
	m_valueFrameDocument->SetMetaObject(m_metaFormObject);

	wxString m_valueFrameTitle = m_title;

	if (demonstration)
		m_valueFrameTitle = st_demonstration + wxT(": ") + m_title;

	if (m_valueFrameTitle.IsEmpty())
		m_valueFrameTitle = reportManager->MakeNewDocumentName();

	m_valueFrameDocument->SetTitle(m_valueFrameTitle);
	m_valueFrameDocument->SetFilename(m_guidForm.str());

	wxScopedPtr<CFormView> view(new CFormView(this));
	if (!view) return false;

	view->SetDocument(m_valueFrameDocument);

	// create a child valueFrame of appropriate class for the current mode
	CMainFrame::CreateChildFrame(view.get(), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);

	if (demonstration) m_visualHostContext = NULL;

	CVisualView *visualView = new CVisualView(m_valueFrameDocument, this, view->GetFrame(), demonstration);

	//set visual view
	m_valueFrameDocument->SetVisualView(visualView);
	m_valueFrameDocument->Modify(m_formModified);

	//create frame 
	visualView->CreateFrame();

	//set window if is not demonstation
	if (!visualView->IsDemonstration()) {
		s_aOpenedForms.insert_or_assign(m_guidForm, m_valueFrameDocument);
	}

	//update and show frame
	visualView->UpdateFrame(); view->ShowFrame();

	return view.release() != NULL;
}