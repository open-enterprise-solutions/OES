#include "notebook.h"
#include "forms/visualEditor.h"  

//***********************************************************************************
//*                           IMPLEMENT_DYNAMIC_CLASS                               *
//***********************************************************************************

wxIMPLEMENT_DYNAMIC_CLASS(CValueNotebookPage, IControlElement);

//***********************************************************************************
//*                              CValueNotebookPage                                 *
//***********************************************************************************

CValueNotebookPage::CValueNotebookPage() : IControlElement(),
m_name("page"),
m_label("NewPage"), m_visible(true),
m_orient(wxVERTICAL)
{
	m_category = new PropertyCategory("Page");

	m_category->AddProperty("name");
	m_category->AddProperty("label");
	m_category->AddProperty("visible");
	m_category->AddProperty("icon");

	//properties 
	m_properties["name"] = new Property("name", PropertyType::PT_WXNAME, this);
	m_properties["label"] = new Property("label", PropertyType::PT_WXSTRING, this);
	m_properties["visible"] = new Property("visible", PropertyType::PT_BOOL, this);
	m_properties["icon"] = new Property("icon", PropertyType::PT_BITMAP, this);

	PropertyCategory *m_category_sizer = new PropertyCategory("Sizer");
	m_category_sizer->AddProperty("orient");
	m_properties["orient"] = new PropertyOption("orient", GetOrient(), this);
	m_category->AddCategory(m_category_sizer);
}

wxObject* CValueNotebookPage::Create(wxObject* parent, IVisualHost *visualHost)
{
	return new CPageWindow((wxWindow*)parent, wxID_ANY);
}

void CValueNotebookPage::OnCreated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	CPageWindow *m_newPage = dynamic_cast<CPageWindow *>(wxobject);
	wxASSERT(m_newPage);
	wxAuiNotebook *m_auiNotebookWnd = dynamic_cast<wxAuiNotebook *>(wxparent);

	if (m_auiNotebookWnd && m_visible) { m_auiNotebookWnd->AddPage(m_newPage, m_label, false, m_bitmap); m_newPage->SetOrientation(m_orient); }
	if (visualHost->IsDesignerHost()) m_newPage->PushEventHandler(new CDesignerWindow::HighlightPaintHandler(m_newPage));
}

void CValueNotebookPage::OnUpdated(wxObject* wxobject, wxWindow* wxparent, IVisualHost *visualHost)
{
	IControlElement *m_parentControl = GetParent(); int idx = wxNOT_FOUND;

	for (unsigned int i = 0; i < m_parentControl->GetChildCount(); i++)
	{
		CValueNotebookPage *child = dynamic_cast<CValueNotebookPage *>(m_parentControl->GetChild(i));
		wxASSERT(child);
		if (m_obj_id == child->m_obj_id) { idx = i; break; }
	}

	CPageWindow *m_pageNotebook = dynamic_cast<CPageWindow *>(wxobject);
	wxASSERT(m_pageNotebook);
	wxAuiNotebook *m_auiNotebookWnd = dynamic_cast<wxAuiNotebook *>(wxparent);
	wxASSERT(m_auiNotebookWnd);

	int idxPage = wxNOT_FOUND;
	for (unsigned int pageIndex = 0; pageIndex < m_auiNotebookWnd->GetPageCount(); pageIndex++) { if (m_auiNotebookWnd->GetPage(pageIndex) == m_pageNotebook) { idxPage = pageIndex; break; } }
	if (idxPage != wxNOT_FOUND) m_auiNotebookWnd->RemovePage(idxPage);

	if (m_visible)
	{
		m_auiNotebookWnd->InsertPage(idx, m_pageNotebook, m_label, true, m_bitmap);

		m_auiNotebookWnd->SetPageText(idx, m_label);
		m_auiNotebookWnd->SetPageBitmap(idx, m_bitmap);

		m_pageNotebook->SetOrientation(m_orient);
	}
}

void CValueNotebookPage::OnSelected(wxObject* wxobject)
{
	IControlElement *m_parentCtrl = GetParent();
	wxASSERT(m_parentCtrl);
	wxAuiNotebook *m_auiNotebookWnd = dynamic_cast<wxAuiNotebook *>(m_parentCtrl->GetWxObject());
	wxASSERT(m_auiNotebookWnd);
	int idxPage = wxNOT_FOUND;
	for (unsigned int pageIndex = 0; pageIndex < m_auiNotebookWnd->GetPageCount(); pageIndex++) { if (m_auiNotebookWnd->GetPage(pageIndex) == wxobject) { idxPage = pageIndex; break; } }
	if (idxPage != m_auiNotebookWnd->GetSelection()) m_auiNotebookWnd->SetSelection(idxPage);
}

void CValueNotebookPage::Cleanup(wxObject* wxobject, IVisualHost *visualHost)
{
	CPageWindow *m_newPage = dynamic_cast<CPageWindow *>(wxobject);
	wxASSERT(m_newPage);
	wxAuiNotebook *m_auiNotebookWnd = dynamic_cast<wxAuiNotebook *>(visualHost->GetWxObject(GetParent()));
	if (m_auiNotebookWnd)
	{
		int page = wxNOT_FOUND;
		for (unsigned int idx = 0; idx < m_auiNotebookWnd->GetPageCount(); idx++) { if (m_auiNotebookWnd->GetPage(idx) == wxobject) { page = idx; break; } }
		if (page != wxNOT_FOUND) { m_auiNotebookWnd->RemovePage(page); m_newPage->Hide(); }
	}

	if (visualHost->IsDesignerHost()) m_newPage->PopEventHandler(true);
}

//***********************************************************************************
//*                              Read & save property                               *
//***********************************************************************************

void CValueNotebookPage::ReadProperty()
{
	m_properties["name"]->SetValue(m_name);
	m_properties["label"]->SetValue(m_label);
	m_properties["visible"]->SetValue(m_visible);
	//m_properties["icon"]->SetValue(m_bitmap);
	m_properties["orient"]->SetValue(m_orient);
}

void CValueNotebookPage::SaveProperty()
{
	m_name = m_properties["name"]->GetValueAsString();
	m_label = m_properties["label"]->GetValueAsString();
	m_visible = m_properties["visible"]->GetValueAsInteger();
	m_bitmap = m_properties["icon"]->GetValueAsBitmap();
	m_orient = m_properties["orient"]->GetValueAsInteger();
}