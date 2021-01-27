////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : report manager 
////////////////////////////////////////////////////////////////////////////

#include <wx/docview.h>
#include <wx/cmdproc.h>
#include <wx/config.h>

#if wxUSE_PRINTING_ARCHITECTURE
#include <wx/paper.h>
#endif // wxUSE_PRINTING_ARCHITECTURE

#include "appData.h"
#include "cmdProc.h"
#include "reportManager.h"

#include "common/templates/text.h"
#include "common/templates/module.h"
#include "common/templates/formDesigner.h"
#include "common/templates/template.h"

//files
#include "common/templates/dataProcessorFile.h"
#include "common/templates/metaFile.h"

#include "utils/stringutils.h"

void CReportManager::Destroy()
{
	CReportManager *const m_docManager = CReportManager::GetDocumentManager();

	if (m_docManager)
	{
#if wxUSE_CONFIG
		m_docManager->FileHistorySave(*wxConfig::Get());
#endif // wxUSE_CONFIG
		delete m_docManager;
	}
}

//****************************************************************
//*                           CReportManager                     *
//****************************************************************

#include "metadata/metaObjects/metaObject.h"

CReportManager::CReportManager()
	: wxDocManager()
{
	AddDocTemplate("Text", "*.txt;*.text", "", "txt;text", "Text Doc", "Text View", CLASSINFO(CTextEditDocument), CLASSINFO(CTextEditView), wxTEMPLATE_VISIBLE, "Text", "Text", 317);
	AddDocTemplate("Grid", "*.grd", "", "grd", "Grid Doc", "Grid View", CLASSINFO(CGridEditDocument), CLASSINFO(CGridEditView), wxTEMPLATE_VISIBLE, "Template", "Template", 318);

	if (appData->IsDesignerMode())
	{
		AddDocTemplate("Data processor", "*.dpr", "", "dpr", "Data processor Doc", "Data processor View", CLASSINFO(CDataProcessorEditDocument), CLASSINFO(CDataProcessorEditView), wxTEMPLATE_VISIBLE, "DataProcessor", "DataProcessor", 320);
		AddDocTemplate("Metadata", "*.mtd", "", "mtd", "Metadata Doc", "Metadata View", CLASSINFO(CMetataEditDocument), CLASSINFO(CMetadataView), wxTEMPLATE_ONLY_OPEN, "Metadata", "Metadata", 320);

		//common objects 
		AddDocTemplate(g_metaCommonModuleCLSID, CLASSINFO(CModuleEditDocument), CLASSINFO(CModuleView), 601);
		AddDocTemplate(g_metaCommonFormCLSID, CLASSINFO(CFormEditDocument), CLASSINFO(CFormEditView), 294);
		AddDocTemplate(g_metaCommonTemplateCLSID, CLASSINFO(CGridEditDocument), CLASSINFO(CGridEditView), 318);

		//advanced object
		AddDocTemplate(g_metaModuleCLSID, CLASSINFO(CModuleEditDocument), CLASSINFO(CModuleView), 601);
		AddDocTemplate(g_metaFormCLSID, CLASSINFO(CFormEditDocument), CLASSINFO(CFormEditView), 294);
		AddDocTemplate(g_metaTemplateCLSID, CLASSINFO(CGridEditDocument), CLASSINFO(CGridEditView), 318);
	}
	else
	{
		AddDocTemplate("Data processor", "*.dpr", "", "dpr", "Data processor Doc", "Data processor View", CLASSINFO(CDataProcessorDocument), CLASSINFO(CDataProcessorView), wxTEMPLATE_ONLY_OPEN, "DataProcessor", "DataProcessor", 320);
	}

#if wxUSE_PRINTING_ARCHITECTURE
	// initialize print data and setup
	wxPrintData m_printData;

	wxPrintPaperType *paper = wxThePrintPaperDatabase->FindPaperType(wxPAPER_A4);

	m_printData.SetPaperId(paper->GetId());
	m_printData.SetPaperSize(paper->GetSize());
	m_printData.SetOrientation(wxPORTRAIT);

	// copy over initial paper size from print record
	m_pageSetupDialogData.SetPrintData(m_printData);

#endif // wxUSE_PRINTING_ARCHITECTURE
}

extern wxImageList *GetImageList();

wxDocument *CReportManager::CreateDocument(const wxString & pathOrig, long flags)
{
	// this ought to be const but SelectDocumentType/Path() are not
	// const-correct and can't be changed as, being virtual, this risks
	// breaking user code overriding them
	wxDocTemplateVector templates;  //(GetVisibleTemplates(m_templates));

	for (auto docTempl : m_aTemplates)
	{
		if (!docTempl.m_docTemplate->IsVisible())
			continue;

		templates.push_back(docTempl.m_docTemplate);
	}

	const size_t numTemplates = templates.size();

	if (!numTemplates)
	{
		// no templates can be used, can't create document
		return NULL;
	}

	// normally user should select the template to use but wxDOC_SILENT flag we
	// choose one ourselves
	wxString path = pathOrig;   // may be modified below
	wxDocTemplate *temp;
	if (flags & wxDOC_SILENT)
	{
		wxASSERT_MSG(!path.empty(),
			"using empty path with wxDOC_SILENT doesn't make sense");

		temp = FindTemplateForPath(path);
		if (!temp)
		{
			wxLogWarning(_("The format of file '%s' couldn't be determined."),
				path);
		}
	}
	else // not silent, ask the user
	{
		// for the new file we need just the template, for an existing one we
		// need the template and the path, unless it's already specified
		if ((flags & wxDOC_NEW) || !path.empty())
			temp = SelectDocumentType(&templates[0], numTemplates);
		else
			temp = SelectDocumentPath(&templates[0], numTemplates, path, flags);
	}

	if (!temp)
		return NULL;

	// check whether the document with this path is already opened
	if (!path.empty())
	{
		wxDocument * const doc = FindDocumentByPath(path);

		if (doc)
		{
			// file already open, just activate it and return
			doc->Activate();
			return doc;
		}
	}

	// no, we need to create a new document


	// if we've reached the max number of docs, close the first one.
	if ((int)GetDocuments().GetCount() >= m_maxDocsOpen)
	{
		if (!CloseDocument((wxDocument *)GetDocuments().GetFirst()->GetData()))
		{
			// can't open the new document if closing the old one failed
			return NULL;
		}
	}

	// do create and initialize the new document finally
	wxDocument * const docNew = temp->CreateDocument(path, flags);
	if (!docNew)
		return NULL;

	docNew->SetDocumentName(temp->GetDocumentName());

	wxTRY
	{
		// call the appropriate function depending on whether we're creating a
		// new file or opening an existing one
		if (!(flags & wxDOC_NEW ? docNew->OnNewDocument()
								 : docNew->OnOpenDocument(path)))
		{
			docNew->DeleteAllViews();
			return NULL;
		}
	}
	wxCATCH_ALL(docNew->DeleteAllViews(); throw; )

		// add the successfully opened file to MRU, but only if we're going to be
		// able to reopen it successfully later which requires the template for
		// this document to be retrievable from the file extension
		if (!(flags & wxDOC_NEW) && temp->FileMatchesTemplate(path))
			AddFileToHistory(path);

	// at least under Mac (where views are top level windows) it seems to be
	// necessary to manually activate the new document to bring it to the
	// forefront -- and it shouldn't hurt doing this under the other platforms
	docNew->Activate();

	return docNew;
}

wxDocTemplate *CReportManager::SelectDocumentPath(wxDocTemplate **templates, int noTemplates, wxString &path, long flags, bool save)
{
#ifdef wxHAS_MULTIPLE_FILEDLG_FILTERS
	wxString descrBuf;

	for (int i = 0; i < noTemplates; i++)
	{
		if (templates[i]->GetFlags() == wxTEMPLATE_VISIBLE
			|| templates[i]->GetFlags() == wxTEMPLATE_ONLY_OPEN)
		{
			if (!descrBuf.empty())
				descrBuf << wxT(";");

			descrBuf << templates[i]->GetFileFilter();
		}
	}

	descrBuf = _("OES files (") + descrBuf + wxT(") |");

	for (int i = 0; i < noTemplates; i++)
	{
		if (templates[i]->IsVisible())
		{
			descrBuf << templates[i]->GetFileFilter() << wxT(";");
		}
	}

	for (int i = 0; i < noTemplates; i++)
	{
		if (templates[i]->GetFlags() == wxTEMPLATE_VISIBLE
			|| templates[i]->GetFlags() == wxTEMPLATE_ONLY_OPEN)
		{
			// add a '|' to separate this filter from the previous one
			if (!descrBuf.empty())
				descrBuf << wxT('|');

			descrBuf << templates[i]->GetDescription()
				<< wxT(" (") << templates[i]->GetFileFilter() << wxT(") |")
				<< templates[i]->GetFileFilter();
		}
	}
#else
	wxString descrBuf = wxT("*.*");
	wxUnusedVar(noTemplates);
#endif

	int FilterIndex = -1;

	wxString pathTmp = wxFileSelectorEx(_("Open File"),
		GetLastDirectory(),
		wxEmptyString,
		&FilterIndex,
		descrBuf,
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	wxDocTemplate *theTemplate = NULL;
	if (!pathTmp.empty())
	{
		if (!wxFileExists(pathTmp))
		{
			wxString msgTitle;
			if (!wxTheApp->GetAppDisplayName().empty())
				msgTitle = wxTheApp->GetAppDisplayName();
			else
				msgTitle = wxString(_("File error"));

			wxMessageBox(_("Sorry, could not open this file."),
				msgTitle,
				wxOK | wxICON_EXCLAMATION | wxCENTRE);

			path.clear();
			return NULL;
		}

		SetLastDirectory(wxPathOnly(pathTmp));

		path = pathTmp;

		// first choose the template using the extension, if this fails (i.e.
		// wxFileSelectorEx() didn't fill it), then use the path
		if (FilterIndex != -1)
		{
			theTemplate = templates[FilterIndex];
			if (theTemplate)
			{
				// But don't use this template if it doesn't match the path as
				// can happen if the user specified the extension explicitly
				// but didn't bother changing the filter.
				if (!theTemplate->FileMatchesTemplate(path))
					theTemplate = NULL;
			}
		}

		if (!theTemplate)
			theTemplate = FindTemplateForPath(path);
		if (!theTemplate)
		{
			// Since we do not add files with non-default extensions to the
			// file history this can only happen if the application changes the
			// allowed templates in runtime.
			wxMessageBox(_("Sorry, the format for this file is unknown."),
				_("Open File"),
				wxOK | wxICON_EXCLAMATION | wxCENTRE);
		}
	}
	else
	{
		path.clear();
	}

	return theTemplate;
	}

#include <wx/scopedarray.h>

wxDocTemplate *CReportManager::SelectDocumentType(wxDocTemplate **templates, int noTemplates, bool sort)
{
	wxArrayString strings;
	wxScopedArray<wxDocTemplate *> data(noTemplates);
	int i;
	int n = 0;

	for (i = 0; i < noTemplates; i++)
	{
		if (templates[i]->GetFlags() == wxTEMPLATE_VISIBLE)
		{
			int j;
			bool want = true;

			for (j = 0; j < n; j++)
			{
				//filter out NOT unique documents + view combinations
				if (templates[i]->GetDocumentName() == data[j]->GetDocumentName() &&
					templates[i]->GetViewName() == data[j]->GetViewName()
					)
					want = false;
			}

			if (want)
			{
				strings.Add(templates[i]->GetDescription());

				data[n] = templates[i];
				n++;
			}
		}
	}  // for

	if (sort)
	{
		strings.Sort(); // ascending sort
		// Yes, this will be slow, but template lists
		// are typically short.
		int j;
		n = strings.Count();
		for (i = 0; i < n; i++)
		{
			for (j = 0; j < noTemplates; j++)
			{
				if (strings[i] == templates[j]->GetDescription())
					data[i] = templates[j];
			}
		}
	}

	wxDocTemplate *theTemplate;

	switch (n)
	{
	case 0:
		// no visible templates, hence nothing to choose from
		theTemplate = NULL;
		break;

	case 1:
		// don't propose the user to choose if he has no choice
		theTemplate = data[0];
		break;

	default:
		// propose the user to choose one of several
		theTemplate = (wxDocTemplate *)wxGetSingleChoiceData
		(
			_("Select a document template"),
			_("Templates"),
			strings,
			(void **)data.get()
		);
	}

	return theTemplate;
}

void CReportManager::AddDocTemplate(const wxString &descr, const wxString &filter, const wxString &dir, const wxString &ext, const wxString &docTypeName, const wxString &viewTypeName, wxClassInfo *docClassInfo, wxClassInfo *viewClassInfo, long flags, const wxString& sName, const wxString& sDescription, int nImage)
{
	CReportElement data;
	data.m_className = sName;
	data.m_classDescription = sDescription;
	data.m_nImage = nImage;
	data.m_docTemplate = new wxDocTemplate(this, descr, filter, dir, ext, docTypeName, viewTypeName, docClassInfo, viewClassInfo, flags);

	m_aTemplates.push_back(data);
}

void CReportManager::AddDocTemplate(const CLASS_ID &id, wxClassInfo *docClassInfo, wxClassInfo *viewClassInfo, int image)
{
	CReportElement data;
	data.m_clsid = id;
	data.m_nImage = image;
	data.m_docTemplate = new wxDocTemplate(this, wxEmptyString, wxEmptyString, wxEmptyString, wxEmptyString, wxEmptyString, wxEmptyString, docClassInfo, viewClassInfo, wxTEMPLATE_INVISIBLE);

	m_aTemplates.push_back(data);
}

#include "metadata/metaObjects/metaObject.h"

CDocument* CReportManager::OpenFormMDI(IMetaObject *metaObject, long flags)
{
	return reportManager->OpenForm(metaObject, NULL, flags);
}

CDocument* CReportManager::OpenFormMDI(IMetaObject *metaObject, CDocument *docParent, long flags)
{
	return reportManager->OpenForm(metaObject, docParent, flags);
}

CDocument* CReportManager::OpenForm(IMetaObject *metaObject, CDocument *docParent, long flags)
{
	for (auto currTemplate : m_aTemplates)
	{
		if (currTemplate.m_clsid == metaObject->GetClsid())
		{
			wxDocTemplate *m_docTemplate = currTemplate.m_docTemplate;
			wxASSERT(m_docTemplate);
			wxClassInfo *m_docClassInfo = m_docTemplate->GetDocClassInfo();
			wxASSERT(m_docClassInfo);

			CDocument *m_newDocument = wxStaticCast(m_docClassInfo->CreateObject(), CDocument);
			wxASSERT(m_newDocument);

			if (docParent) {
				m_newDocument->SetDocParent(docParent);
			}

			try
			{
				m_newDocument->SetTitle(metaObject->GetModuleName());
				m_newDocument->SetFilename(metaObject->GetDocPath());
				m_newDocument->SetDocumentTemplate(m_docTemplate);
				m_newDocument->SetMetaObject(metaObject);

				//if doc has parent - special delete!
				if (!docParent) {
					wxDocManager::AddDocument(m_newDocument);
				}

				if (currTemplate.m_nImage > 0) m_newDocument->SetIcon(GetImageList()->GetIcon(currTemplate.m_nImage));

				if (m_newDocument->OnCreate(metaObject->GetModuleName(), flags | wxDOC_NEW))
				{
					m_newDocument->SetCommandProcessor(m_newDocument->CreateCommandProcessor());

					m_newDocument->Modify(false);
					m_newDocument->UpdateAllViews();
					m_newDocument->Activate();

					return m_newDocument;
				}

				// The document may be already destroyed, this happens if its view
				// creation fails as then the view being created is destroyed
				// triggering the destruction of the document as this first view is
				// also the last one. However if OnCreate() fails for any reason other
				// than view creation failure, the document is still alive and we need
				// to clean it up ourselves to avoid having a zombie document.
				if (CReportManager::GetDocuments().Member(m_newDocument))
					m_newDocument->DeleteAllViews();

				return NULL;
			}
			catch (...)
			{
				if (CReportManager::GetDocuments().Member(m_newDocument))
				{
					m_newDocument->DeleteAllViews();
				}
			}

			return NULL;
		}
	}

	return NULL;
}

bool CReportManager::CloseDocument(wxDocument* doc, bool force)
{
	auto m_documentViews = doc->GetViews();

	// first check if all views agree to be closed
	const wxList::iterator end = m_documentViews.end();
	for (wxList::iterator i = m_documentViews.begin(); i != end; ++i)
	{
		wxView *view = (wxView *)*i;
		if (!view->Close(false)) {
			return false;
		}
		else
		{
			view->GetFrame()->Destroy();
			view->SetFrame(NULL);
		}
	}

	// as we delete elements we iterate over, don't use the usual "from
	// begin to end" loop
	for (;;)
	{
		wxView *view = (wxView *)*m_documentViews.begin();

		bool isLastOne = m_documentViews.size() == 1;

		// this always deletes the node implicitly and if this is the last
		// view also deletes this object itself (also implicitly, great),
		// so we can't test for m_documentViews.empty() after calling this!
		delete view;

		if (isLastOne)
			break;
	}

	wxASSERT(!m_docs.Member(doc));

	return true;
}

bool CReportManager::CloseDocuments(bool force)
{
	wxList::compatibility_iterator node = m_docs.GetFirst();

	while (node)
	{
		wxDocument *doc = (wxDocument *)node->GetData();
		wxList::compatibility_iterator next = node->GetNext();

		if (!CloseDocument(doc, force))
			return false;

		// This assumes that documents are not connected in
		// any way, i.e. deleting one document does NOT
		// delete another.
		node = next;
	}

	return true;
}

bool CReportManager::Clear(bool force)
{
	if (!CloseDocuments(force))
		return false;

	m_currentView = NULL;

	wxList::compatibility_iterator node = m_templates.GetFirst();

	while (node)
	{
		wxDocTemplate *templ = (wxDocTemplate*)node->GetData();
		wxList::compatibility_iterator next = node->GetNext();
		delete templ;
		node = next;
	}

	return true;
}

CReportManager::~CReportManager()
{
}