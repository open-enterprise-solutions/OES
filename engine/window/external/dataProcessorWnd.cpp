////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : dataprocessor window
////////////////////////////////////////////////////////////////////////////

#include "dataProcessorWnd.h"
#include "compiler/debugger/debugServer.h"

wxIMPLEMENT_DYNAMIC_CLASS(CDataProcessorTree, wxPanel);

extern wxImageList *GetImageList();

wxBEGIN_EVENT_TABLE(CDataProcessorTree, wxPanel)
wxEND_EVENT_TABLE()

CDataProcessorTree::CDataProcessorTree(CDocument *docParent, wxWindow* parent, wxWindowID id)
	: wxPanel(parent, id), m_docParent(docParent)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizerMain = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* bSizerHeader = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* bSizerCaption = new wxBoxSizer(wxVERTICAL);

	m_nameCaption = new wxStaticText(this, wxID_ANY, wxT("Name"), wxDefaultPosition, wxDefaultSize, 0);
	m_nameCaption->Wrap(-1);

	bSizerCaption->Add(m_nameCaption, 0, wxALL, 5);

	m_aliasCaption = new wxStaticText(this, wxID_ANY, wxT("Alias"), wxDefaultPosition, wxDefaultSize, 0);
	m_aliasCaption->Wrap(-1);

	bSizerCaption->Add(m_aliasCaption, 0, wxALL, 5);

	m_commentCaption = new wxStaticText(this, wxID_ANY, wxT("Comment"), wxDefaultPosition, wxDefaultSize, 0);
	m_commentCaption->Wrap(-1);

	bSizerCaption->Add(m_commentCaption, 0, wxALL, 5);

	m_defaultForm = new wxStaticText(this, wxID_ANY, wxT("Default form:"), wxDefaultPosition, wxDefaultSize, 0);
	m_defaultForm->Wrap(-1);

	bSizerCaption->Add(m_defaultForm, 0, wxALL, 5);
	bSizerHeader->Add(bSizerCaption, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerValue = new wxBoxSizer(wxVERTICAL);

	m_nameValue = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizerValue->Add(m_nameValue, 1, wxALL | wxEXPAND, 1);
	m_nameValue->Connect(wxEVT_TEXT, wxCommandEventHandler(CDataProcessorTree::OnEditCaptionName), NULL, this);

	m_aliasValue = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizerValue->Add(m_aliasValue, 1, wxALL | wxEXPAND, 1);
	m_aliasValue->Connect(wxEVT_TEXT, wxCommandEventHandler(CDataProcessorTree::OnEditCaptionAlias), NULL, this);

	m_commentValue = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizerValue->Add(m_commentValue, 1, wxALL | wxEXPAND, 1);
	m_commentValue->Connect(wxEVT_TEXT, wxCommandEventHandler(CDataProcessorTree::OnEditCaptionComment), NULL, this);

	m_defaultFormValue = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, NULL, 0);
	m_defaultFormValue->AppendString(_("<not selected>"));
	m_defaultFormValue->SetSelection(0);

	m_defaultFormValue->Connect(wxEVT_CHOICE, wxCommandEventHandler(CDataProcessorTree::OnChoiceDefForm), NULL, this);


	bSizerValue->Add(m_defaultFormValue, 1, wxALL | wxEXPAND, 1);
	bSizerHeader->Add(bSizerValue, 1, 0, 5);
	bSizerMain->Add(bSizerHeader, 0, wxEXPAND, 5);

	wxStaticBoxSizer* sbSizerTree = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("")), wxVERTICAL);

	m_metatreeToolbar = new wxAuiToolBar(sbSizerTree->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_TEXT);
	m_metatreeToolbar->AddTool(ID_METATREE_NEW, _("new"), wxGetImageBMPFromResource(IDB_EDIT_NEW), wxNullBitmap, wxItemKind::wxITEM_NORMAL, _("new item"), _("new item"), NULL);
	m_metatreeToolbar->AddTool(ID_METATREE_EDIT, _("edit"), wxGetImageBMPFromResource(IDB_EDIT), wxNullBitmap, wxItemKind::wxITEM_NORMAL, _("edit item"), _("edit item"), NULL);
	m_metatreeToolbar->AddTool(ID_METATREE_REMOVE, _("remove"), wxGetImageBMPFromResource(IDB_EDIT_CUT), wxNullBitmap, wxItemKind::wxITEM_NORMAL, _("remove item"), _("remove item"), NULL);
	m_metatreeToolbar->Realize();

	sbSizerTree->Add(m_metatreeToolbar, 0, wxALL | wxEXPAND, 0);

	m_metatreeWnd = new CDataProcessorTreeWnd(sbSizerTree->GetStaticBox(), this);

	//set image list
	m_metatreeWnd->SetImageList(::GetImageList());

	m_metatreeToolbar->Bind(wxEVT_MENU, &CDataProcessorTree::CDataProcessorTreeWnd::OnCreateItem, m_metatreeWnd, ID_METATREE_NEW);
	m_metatreeToolbar->Bind(wxEVT_MENU, &CDataProcessorTree::CDataProcessorTreeWnd::OnEditItem, m_metatreeWnd, ID_METATREE_EDIT);
	m_metatreeToolbar->Bind(wxEVT_MENU, &CDataProcessorTree::CDataProcessorTreeWnd::OnRemoveItem, m_metatreeWnd, ID_METATREE_REMOVE);

	sbSizerTree->Add(m_metatreeWnd, 1, wxALL | wxEXPAND, 0);

	bSizerMain->Add(sbSizerTree, 1, wxEXPAND, 5);

	m_buttonModule = new wxButton(this, wxID_ANY, _("open module"));
	m_buttonModule->Connect(wxEVT_BUTTON, wxCommandEventHandler(CDataProcessorTree::OnButtonModuleClicked), NULL, this);

	bSizerMain->Add(m_buttonModule, 0, wxALL);

	this->SetSizer(bSizerMain);
	this->Layout();

	this->Centre(wxBOTH);

	//Initialize tree
	InitTree();
}

CDataProcessorTree::~CDataProcessorTree()
{
	m_nameValue->Disconnect(wxEVT_TEXT, wxCommandEventHandler(CDataProcessorTree::OnEditCaptionName), NULL, this);
	m_aliasValue->Disconnect(wxEVT_TEXT, wxCommandEventHandler(CDataProcessorTree::OnEditCaptionAlias), NULL, this);
	m_commentValue->Disconnect(wxEVT_TEXT, wxCommandEventHandler(CDataProcessorTree::OnEditCaptionComment), NULL, this);

	m_metatreeToolbar->Unbind(wxEVT_MENU, &CDataProcessorTree::CDataProcessorTreeWnd::OnCreateItem, m_metatreeWnd, ID_METATREE_NEW);
	m_metatreeToolbar->Unbind(wxEVT_MENU, &CDataProcessorTree::CDataProcessorTreeWnd::OnEditItem, m_metatreeWnd, ID_METATREE_EDIT);
	m_metatreeToolbar->Unbind(wxEVT_MENU, &CDataProcessorTree::CDataProcessorTreeWnd::OnRemoveItem, m_metatreeWnd, ID_METATREE_REMOVE);

	m_buttonModule->Connect(wxEVT_BUTTON, wxCommandEventHandler(CDataProcessorTree::OnButtonModuleClicked), NULL, this);
}

void CDataProcessorTree::OnEditCaptionName(wxCommandEvent &event)
{
	wxString text = m_nameValue->GetValue();

	for (unsigned int i = 0; i < text.length(); i++)
	{
		if (!((text[i] == '_') ||
			(text[i] >= 'A' && text[i] <= 'Z') || (text[i] >= 'a' && text[i] <= 'z') ||
			(text[i] >= '�' && text[i] <= '�') || (text[i] >= '�' && text[i] <= '�') ||
			(text[i] >= '0' && text[i] <= '9')))
		{
			text = text.Left(i);
			wxMessageBox(wxT("You can enter only numbers, letters and the symbol \"_\""), wxT("Error entering value"));
			m_nameValue->SetValue(text);
			break;
		}
	}

	m_docParent->SetTitle(text);

	CMetaObjectDataProcessorValue *dataProcessor = m_metaData->GetDataProcessor();
	wxASSERT(dataProcessor);
	dataProcessor->SetName(text);

	m_docParent->OnChangeFilename(true);
	m_metaData->Modify(true);
}

void CDataProcessorTree::OnEditCaptionAlias(wxCommandEvent &event)
{
	CMetaObjectDataProcessorValue *dataProcessor = m_metaData->GetDataProcessor();
	wxASSERT(dataProcessor);
	dataProcessor->SetAlias(m_aliasValue->GetValue());

	m_metaData->Modify(true);
}

void CDataProcessorTree::OnEditCaptionComment(wxCommandEvent &event)
{
	CMetaObjectDataProcessorValue *dataProcessor = m_metaData->GetDataProcessor();
	wxASSERT(dataProcessor);
	dataProcessor->SetComment(m_commentValue->GetValue());

	m_metaData->Modify(true);
}

void CDataProcessorTree::OnChoiceDefForm(wxCommandEvent &event)
{
	CMetaObjectDataProcessorValue *dataProcessor = m_metaData->GetDataProcessor();
	wxASSERT(dataProcessor);

	meta_identifier_t meta_id = reinterpret_cast<meta_identifier_t>(event.GetClientData());
	if (meta_id > 0) {
		dataProcessor->m_defaultFormObject = meta_id;
	}
	else {
		dataProcessor->m_defaultFormObject = wxNOT_FOUND;
	}

	m_metaData->Modify(true);
}

void CDataProcessorTree::OnButtonModuleClicked(wxCommandEvent &event)
{
	CMetaObjectDataProcessorValue *dataProcessor = m_metaData->GetDataProcessor();
	wxASSERT(dataProcessor);
	OpenFormMDI(dataProcessor->GetModuleObject());
}

wxIMPLEMENT_DYNAMIC_CLASS(CDataProcessorTree::CDataProcessorTreeWnd, wxTreeCtrl);

//**********************************************************************************
//*                                  metatree window						       *
//**********************************************************************************

wxBEGIN_EVENT_TABLE(CDataProcessorTree::CDataProcessorTreeWnd, wxTreeCtrl)

EVT_PROPERTY_MODIFIED(CDataProcessorTree::CDataProcessorTreeWnd::OnPropertyModified)

EVT_LEFT_UP(CDataProcessorTree::CDataProcessorTreeWnd::OnLeftUp)
EVT_LEFT_DOWN(CDataProcessorTree::CDataProcessorTreeWnd::OnLeftDown)
EVT_LEFT_DCLICK(CDataProcessorTree::CDataProcessorTreeWnd::OnLeftDClick)
EVT_RIGHT_UP(CDataProcessorTree::CDataProcessorTreeWnd::OnRightUp)
EVT_RIGHT_DOWN(CDataProcessorTree::CDataProcessorTreeWnd::OnRightDown)
EVT_RIGHT_DCLICK(CDataProcessorTree::CDataProcessorTreeWnd::OnRightDClick)
EVT_MOTION(CDataProcessorTree::CDataProcessorTreeWnd::OnMouseMove)
EVT_KEY_UP(CDataProcessorTree::CDataProcessorTreeWnd::OnKeyUp)
EVT_KEY_DOWN(CDataProcessorTree::CDataProcessorTreeWnd::OnKeyDown)

EVT_TREE_ITEM_COLLAPSING(wxID_ANY, CDataProcessorTree::CDataProcessorTreeWnd::OnCollapsing)
EVT_TREE_ITEM_EXPANDING(wxID_ANY, CDataProcessorTree::CDataProcessorTreeWnd::OnExpanding)

EVT_MENU(ID_METATREE_NEW, CDataProcessorTree::CDataProcessorTreeWnd::OnCreateItem)
EVT_MENU(ID_METATREE_EDIT, CDataProcessorTree::CDataProcessorTreeWnd::OnEditItem)
EVT_MENU(ID_METATREE_REMOVE, CDataProcessorTree::CDataProcessorTreeWnd::OnRemoveItem)
EVT_MENU(ID_METATREE_PROPERTY, CDataProcessorTree::CDataProcessorTreeWnd::OnPropertyItem)

EVT_MENU(wxID_COPY, CDataProcessorTree::CDataProcessorTreeWnd::OnCopyItem)
EVT_MENU(wxID_PASTE, CDataProcessorTree::CDataProcessorTreeWnd::OnPasteItem)

EVT_DEBUG(CDataProcessorTree::CDataProcessorTreeWnd::OnDebugEvent)

wxEND_EVENT_TABLE()

CDataProcessorTree::CDataProcessorTreeWnd::CDataProcessorTreeWnd()
	: wxTreeCtrl(), m_ownerTree(NULL)
{
	//set double buffered
	SetDoubleBuffered(true);
}

CDataProcessorTree::CDataProcessorTreeWnd::CDataProcessorTreeWnd(wxWindow *parentWnd, CDataProcessorTree *ownerWnd)
	: wxTreeCtrl(parentWnd, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_HIDE_ROOT), m_ownerTree(ownerWnd)
{
	debugServer->AddHandler(this);

	//set double buffered
	SetDoubleBuffered(true);
}

CDataProcessorTree::CDataProcessorTreeWnd::~CDataProcessorTreeWnd()
{
	debugServer->RemoveHandler(this);
}