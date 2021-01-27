#ifndef _REPORTMANAGER_H__
#define _REPORTMANAGER_H__

#include "docInfo.h"

#define reportManager CReportManager::GetDocumentManager()
#define reportManagerDestroy() CReportManager::Destroy()

#include "compiler/compiler.h"

// Document template flags
enum
{
	wxTEMPLATE_ONLY_OPEN = wxTEMPLATE_INVISIBLE + 1
};

class CReportManager : public wxDocManager
{
	struct CReportElement
	{
		CLASS_ID m_clsid;

		wxString m_className;
		wxString m_classDescription;

		int		m_nImage;

		wxDocTemplate *m_docTemplate;

		CReportElement() : m_nImage(0) {};
	};

	std::vector <CReportElement> m_aTemplates;

private:

	CDocument* OpenForm(IMetaObject *metaObject, CDocument *docParent, long flags);

public:

	CReportManager();

	void AddDocTemplate(const wxString& descr, const wxString& filter, const wxString& dir, const wxString& ext, const wxString& docTypeName, const wxString& viewTypeName, wxClassInfo *docClassInfo, wxClassInfo *viewClassInfo, long flags, const wxString& sName, const wxString& sDescription, int nImage);
	void AddDocTemplate(const CLASS_ID &id, wxClassInfo *docClassInfo, wxClassInfo *viewClassInfo, int image);

	static CDocument* OpenFormMDI(IMetaObject *metaObject, long flags = wxDOC_NEW);
	static CDocument* OpenFormMDI(IMetaObject *metaObject, CDocument *docParent, long flags = wxDOC_NEW);

	// Get the current document manager
	static CReportManager* GetDocumentManager() { return dynamic_cast<CReportManager *>(sm_docManager); }
	//destroy manager 
	static void Destroy();

	virtual wxDocument *CreateDocument(const wxString& pathOrig, long flags) override;

	virtual wxDocTemplate *SelectDocumentPath(wxDocTemplate **templates,
		int noTemplates, wxString& path, long flags, bool save = false) override;

	virtual wxDocTemplate *SelectDocumentType(wxDocTemplate **templates,
		int noTemplates, bool sort = false) override;

	bool CloseDocument(wxDocument* doc, bool force = false);
	bool CloseDocuments(bool force);
	bool Clear(bool force);

	virtual ~CReportManager();
};

#endif
