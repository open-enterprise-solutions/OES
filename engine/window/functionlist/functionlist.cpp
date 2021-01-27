////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : function list window
////////////////////////////////////////////////////////////////////////////

#include "functionlist.h"
#include "common/docInfo.h"

#include "window/autocomplete/autoComplectionParser.h"
#include "metadata/metaObjectsDefines.h"

wxBEGIN_EVENT_TABLE(CFunctionList, wxDialog)
wxEND_EVENT_TABLE()

wxImageList *GetImageList();

CFunctionList::CFunctionList(CDocument *moduleDoc, CAutocomplectionCtrl* parent)
	: wxDialog(parent, wxID_ANY, _("Procedures and functions")), m_docModule(moduleDoc), m_codeEditor(parent)
{
	m_OK = new wxButton(this, wxID_ANY, _("OK"));
	m_OK->Connect(wxEVT_BUTTON, wxCommandEventHandler(CFunctionList::OnButtonOk), NULL, this);
	m_Cancel = new wxButton(this, wxID_ANY, _("Cancel"));
	m_Cancel->Connect(wxEVT_BUTTON, wxCommandEventHandler(CFunctionList::OnButtonCancel), NULL, this);

	m_Sort = new wxCheckBox(this, wxID_ANY, _("Sort"));
	m_Sort->Connect(wxEVT_CHECKBOX, wxCommandEventHandler(CFunctionList::OnCheckBoxSort), NULL, this);

	wxBoxSizer *boxsizerList = new wxBoxSizer(wxHORIZONTAL);

	m_listProcedures = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_NO_HEADER);
	m_listProcedures->AppendColumn(_("Procedures and functions"), wxLIST_FORMAT_LEFT, GetSize().x - 10);

	m_listProcedures->Connect(wxEVT_LIST_ITEM_SELECTED, wxListEventHandler(CFunctionList::OnItemSelected), NULL, this);

	m_listProcedures->SetImageList(::GetImageList(), wxIMAGE_LIST_SMALL);

	CMetaModuleObject *metaModule = dynamic_cast<CMetaModuleObject *>(moduleDoc->GetMetaObject());
	wxASSERT(metaModule);

	CParserModule moduleParser;

	if (moduleParser.ParseModule(metaModule->GetModuleText()))
	{
		for (auto content : moduleParser.GetAllContent()) {

			if (content.eType == eContentType::eExportFunction ||
				content.eType == eContentType::eFunction ||
				content.eType == eContentType::eExportProcedure ||
				content.eType == eContentType::eProcedure)
			{
				int item_id = m_listProcedures->GetItemCount();

				wxListItem info;
				info.m_image = content.nImage;
				info.m_text = content.sName;
				info.m_mask = wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE;
				info.m_itemId = item_id;
				info.m_col = 0;
				info.m_data = content.nLine;

				long item = m_listProcedures->InsertItem(info);
				m_listProcedures->SetItemData(item, content.nLine);
			}
		}
	}

	wxBoxSizer *boxsizerButton = new wxBoxSizer(wxVERTICAL);
	boxsizerButton->Add(m_OK, 0, wxEXPAND);
	boxsizerButton->Add(m_Cancel, 0, wxEXPAND);
	boxsizerButton->AddSpacer(10);
	boxsizerButton->Add(m_Sort, 0, wxEXPAND);

	boxsizerList->Add(m_listProcedures, 1, wxEXPAND);
	boxsizerList->Add(boxsizerButton, 0, wxEXPAND);

	SetSizer(boxsizerList);
}

void CFunctionList::OnButtonOk(wxCommandEvent &event)
{
	long lSelectedItem = m_listProcedures->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	m_codeEditor->Raise();
	m_codeEditor->SetFocus();

	m_codeEditor->SetSTCFocus(true);

	if (lSelectedItem != wxNOT_FOUND) {
		wxUIntPtr line = m_listProcedures->GetItemData(lSelectedItem);
		m_codeEditor->GotoLine(line);
	}

	EndModal(0); event.Skip();
}

void CFunctionList::OnButtonCancel(wxCommandEvent &event)
{
	m_codeEditor->Raise();
	m_codeEditor->SetFocus();

	m_codeEditor->SetSTCFocus(true);

	EndModal(1); event.Skip();
}

int wxCALLBACK
CompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
	sortInfo_t *sortInfo = (sortInfo_t *)sortData;
	wxListCtrl *listCtrl = sortInfo->m_listCtrl;

	long index1 = listCtrl->FindItem(-1, item1); // gets index of the first item
	long index2 = listCtrl->FindItem(-1, item2); // gets index of the second item

	if (sortInfo->m_sortOrder) {
	
		wxString string1 = listCtrl->GetItemText(index1);
		wxString string2 = listCtrl->GetItemText(index2);

		if (string1.Cmp(string2) < 0)
		{
			return -1;
		}
		else if (string1.Cmp(string2) > 0)
		{
			return 1;
		}

	}
	else {

		wxIntPtr line1 = listCtrl->GetItemData(index1);
		wxIntPtr line2 = listCtrl->GetItemData(index2);

		if (line1 < line2)
			return -1;
		if (line1 > line2)
			return 1;
	}

	return 0;
}

void CFunctionList::OnCheckBoxSort(wxCommandEvent &event)
{
	sortInfo.m_listCtrl = m_listProcedures;
	sortInfo.m_sortOrder = m_Sort->GetValue();

	m_listProcedures->SortItems(CompareFunction, (wxIntPtr)&sortInfo);
	event.Skip();
}

void CFunctionList::OnItemSelected(wxListEvent &event)
{
	/*long lSelectedItem = m_listProcedures->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if (lSelectedItem != wxNOT_FOUND) {
		wxUIntPtr line = m_listProcedures->GetItemData(lSelectedItem);
		m_codeEditor->GotoLine(line);
	}

	m_codeEditor->SetFocus(); 
	EndModal(0);*/ event.Skip(); 
}
