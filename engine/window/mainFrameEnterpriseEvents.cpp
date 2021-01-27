////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxwidgets community
//	Description : main frame window
////////////////////////////////////////////////////////////////////////////

#include "mainFrameEnterprise.h"
#include "metadata/metadata.h"
#include "metadata/metaObjectsDefines.h"
#include "metadata/objects/baseObject.h"
#include "forms/elements/frame.h"

#include <wx/dialog.h>
#include <wx/treectrl.h>

class CDialogOperations : public wxDialog
{
	wxTreeCtrl* m_treeCtrlElements;
	std::map< wxTreeItemId, IMetaObject *> m_aItems;

public:

	CDialogOperations(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = "All operations", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(480, 334), long style = wxDEFAULT_DIALOG_STYLE)
		: wxDialog(parent, id, title, pos, size, style)
	{
		this->SetSizeHints(wxDefaultSize, wxDefaultSize);

		wxBoxSizer* bSizer = new wxBoxSizer(wxVERTICAL);

		//m_buttonOpen = new wxButton(this, wxID_ANY, wxT("Open"), wxDefaultPosition, wxDefaultSize, 0);
		//bSizer->Add(m_buttonOpen, 0, wxALL | wxEXPAND, 5);

		m_treeCtrlElements = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_SINGLE);
		bSizer->Add(m_treeCtrlElements, 1, wxALL | wxEXPAND, 5);

		// Connect Events
		m_treeCtrlElements->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(CDialogOperations::m_treeCtrlElementsOnLeftDClick), NULL, this);

		this->SetSizer(bSizer);
		this->Layout();

		this->Centre(wxBOTH);
	}

	void BuildOperations()
	{
		wxTreeItemId root = m_treeCtrlElements->AddRoot(_("objects"));
		wxTreeItemId catalogs = m_treeCtrlElements->AppendItem(root, _("catalogs"));

		for (auto catalog : metadata->GetMetaObjects(g_metaCatalogCLSID))
		{
			wxTreeItemId hCatalog = m_treeCtrlElements->AppendItem(catalogs, catalog->GetName());
			m_aItems[hCatalog] = catalog;
		}

		wxTreeItemId documents = m_treeCtrlElements->AppendItem(root, _("documents"));

		for (auto document : metadata->GetMetaObjects(g_metaDocumentCLSID))
		{
			wxTreeItemId hDocument = m_treeCtrlElements->AppendItem(documents, document->GetName());
			m_aItems[hDocument] = document;
		}

		wxTreeItemId dataProcessors = m_treeCtrlElements->AppendItem(root, _("dataProcessors"));

		for (auto dataProcessor : metadata->GetMetaObjects(g_metaDataProcessorCLSID))
		{
			wxTreeItemId hDataProcessor = m_treeCtrlElements->AppendItem(dataProcessors, dataProcessor->GetName());
			m_aItems[hDataProcessor] = dataProcessor;
		}

		m_treeCtrlElements->ExpandAll();
	}

	virtual void m_treeCtrlElementsOnLeftDClick(wxMouseEvent& event)
	{
		wxTreeItemId selItem = m_treeCtrlElements->GetSelection();

		if (m_aItems.find(selItem) != m_aItems.end())
		{
			IMetaObjectValue *metaObj = wxStaticCast(m_aItems.at(selItem), IMetaObjectValue);

			if (metaObj->IsRefObject())
			{
				IMetaObjectRefValue *metaRefObj = wxStaticCast(metaObj, IMetaObjectRefValue);

				wxASSERT(metaRefObj);
				CValueFrame *valueFrame = metaRefObj->GetListForm();
				valueFrame->ShowForm(); Close();
			}
			else
			{
				wxASSERT(metaObj);
				CValueFrame *valueFrame = metaObj->GetObjectForm();
				valueFrame->ShowForm(); Close();
			}
		}

		event.Skip();
	}

	virtual ~CDialogOperations()
	{
		m_treeCtrlElements->Disconnect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(CDialogOperations::m_treeCtrlElementsOnLeftDClick), NULL, this);
	}
};

void CMainFrameEnterprise::OnClickAllOperaions(wxCommandEvent &event)
{
	CDialogOperations *m_dialog = new CDialogOperations(this, wxID_ANY);
	m_dialog->BuildOperations();
	m_dialog->Show();

	event.Skip();
}