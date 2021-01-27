////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxFormBuilder
//	Description : visual editor 
////////////////////////////////////////////////////////////////////////////

#include "visualEditor.h"
#include "window/title.h"
#include "elements/frame.h"

wxIMPLEMENT_DYNAMIC_CLASS(CVisualEditorContextForm, wxPanel);

wxBEGIN_EVENT_TABLE(CVisualEditorContextForm, wxPanel)
EVT_PROJECT_LOADED(CVisualEditorContextForm::OnProjectLoaded)
EVT_PROJECT_SAVED(CVisualEditorContextForm::OnProjectSaved)
EVT_OBJECT_SELECTED(CVisualEditorContextForm::OnObjectSelected)
EVT_OBJECT_CREATED(CVisualEditorContextForm::OnObjectCreated)
EVT_OBJECT_REMOVED(CVisualEditorContextForm::OnObjectRemoved)
EVT_PROPERTY_MODIFIED(CVisualEditorContextForm::OnPropertyModified)
EVT_PROJECT_REFRESH(CVisualEditorContextForm::OnProjectRefresh)
EVT_FRAME_EVENT_HANDLER_MODIFIED(CVisualEditorContextForm::OnCodeGeneration)
wxEND_EVENT_TABLE()

CVisualEditorContextForm::CVisualEditorContextForm() :
	wxPanel(), m_bReadOnly(false)
{
}

CVisualEditorContextForm::CVisualEditorContextForm(CDocument *document, CView *view, wxWindow *parent, int id) :
	wxPanel(parent, id),
	m_document(document), m_view(view), m_cmdProc(document->GetCommandProcessor()), m_valueFrame(NULL),
	m_bReadOnly(false)
{
	CreateWideGui();
}

void CVisualEditorContextForm::CreateWideGui()
{
	wxWindow::Freeze();

	m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
	m_splitter->SetSashGravity(0.5);
	m_splitter->SetMinimumPaneSize(20); // Smalest size the

	wxBoxSizer *m_sizerMain = new wxBoxSizer(wxVERTICAL);
	m_sizerMain->Add(m_splitter, 1, wxEXPAND, 0);

	wxPanel *m_panelDesigner = new wxPanel(m_splitter, wxID_ANY);
	wxBoxSizer *designersizer = new wxBoxSizer(wxVERTICAL);

	wxASSERT(m_visualEditor);

	m_visualEditor = new CVisualEditor(this, m_panelDesigner);

	designersizer->Add(m_visualEditor, 1, wxEXPAND, 0);

	m_panelDesigner->SetSizer(designersizer);

	wxPanel *m_panelTree = new wxPanel(m_splitter, wxID_ANY);
	wxBoxSizer *treesizer = new wxBoxSizer(wxVERTICAL);

	m_objectTree = new CVisualEditorObjectTree(this, m_panelTree);
	m_objectTree->Create();

	treesizer->Add(m_objectTree, 1, wxEXPAND, 0);
	m_panelTree->SetSizer(treesizer);

	m_splitter->SplitVertically(m_panelDesigner, m_panelTree, -300);
	SetSizer(m_sizerMain);

	wxWindow::Thaw();
}

void CVisualEditorContextForm::ActivateObject()
{
	m_visualHostContext = this;
}

void CVisualEditorContextForm::DeactivateObject()
{
	m_visualHostContext = NULL;
}

#include "common/docInfo.h" 
#include "compiler/value.h"
#include "metadata/objects/baseObject.h"
#include "metadata/metadata.h"
#include "metadata/metaObjectsDefines.h"

bool CVisualEditorContextForm::LoadForm()
{
	if (!m_document)
		return false;

	IMetaFormObject *formMetaObject = dynamic_cast<IMetaFormObject *>(m_document->GetMetaObject());

	if (!formMetaObject)
		return false;

	IMetadata *metaData = formMetaObject->GetMetadata();
	wxASSERT(metaData);
	
	IModuleManager *moduleManager = metaData->GetModuleManager();
	wxASSERT(moduleManager);
	
	if (moduleManager->FindCompileModule(formMetaObject, m_valueFrame))
		m_valueFrame->IncrRef();
	else
		return false;

	ResolveNameConflict(m_valueFrame);
	NotifyProjectLoaded();
	SelectObject(m_valueFrame, true);

	// first create control 
	m_visualEditor->CreateVisualEditor();
	// then update control 
	m_visualEditor->UpdateVisualEditor();

	return true;
}

bool CVisualEditorContextForm::SaveForm()
{
	ticpp::Document doc;
	m_valueFrame->Serialize(&doc);

	// Declare a printer    
	TiXmlPrinter m_data;

	// attach it to the document you want to convert in to a std::string 
	doc.Accept(&m_data);

	IMetaFormObject *formMetaObject = dynamic_cast<IMetaFormObject *>(m_document->GetMetaObject());

	// Create a std::string and copy your document data in to the string    
	if (formMetaObject)
		formMetaObject->SetFormData(m_data.Str());

	m_document->Modify(false);
	NotifyProjectSaved();

	return true;
}

#include "visualEditorView.h"

void CVisualEditorContextForm::RunForm()
{
	m_valueFrame->ShowForm(true);
}

#include "window/mainFrame.h"
#include "window/property/objinspect.h"

CVisualEditorContextForm::~CVisualEditorContextForm()
{
	if (m_valueFrame)
	{
		//CVisualDocument *m_visualDocument = m_valueFrame->GetVisualDocument();
		//if (m_visualDocument) m_visualDocument->DeleteAllViews();
		//objectInspector->SelectObject(m_valueFrame->GetFormMetaObject());
		m_valueFrame->DecrRef();
	}

	m_visualEditor->Destroy();
	m_objectTree->Destroy();
	m_splitter->Destroy();
}

//********************************************************************
//*                           Events                                 *
//********************************************************************

void CVisualEditorContextForm::OnProjectLoaded(wxFrameEvent &event)
{
	// first create control 
	m_visualEditor->CreateVisualEditor();
	// then update control 
	m_visualEditor->UpdateVisualEditor();
}

void CVisualEditorContextForm::OnProjectSaved(wxFrameEvent &event)
{
}

void CVisualEditorContextForm::OnObjectSelected(wxFrameObjectEvent &event)
{
	/*// Get the IControlElement from the event
	IControlElement* obj = event.GetFrameObject();
	if (!obj) return;
	m_visualEditor->SetObjectSelect(obj);*/
}

void CVisualEditorContextForm::OnObjectCreated(wxFrameObjectEvent &event)
{
}

void CVisualEditorContextForm::OnObjectRemoved(wxFrameObjectEvent &event)
{
}

void CVisualEditorContextForm::OnPropertyModified(wxFramePropertyEvent &event)
{
	Property *p = event.GetFrameProperty();
	wxASSERT(p);
	ModifyProperty(p, event.GetValue());
}

void CVisualEditorContextForm::OnProjectRefresh(wxFrameEvent &event)
{
}

void CVisualEditorContextForm::OnCodeGeneration(wxFrameEventHandlerEvent &event)
{
	wxView* view = m_document->GetFirstView();
	view->ProcessEvent(event);

	ticpp::Document doc;
	m_valueFrame->Serialize(&doc);

	// Declare a printer    
	TiXmlPrinter m_data;
	// attach it to the document you want to convert in to a std::string 
	doc.Accept(&m_data);

	IMetaFormObject *formObject = dynamic_cast<IMetaFormObject *>(m_document->GetMetaObject());
	wxASSERT(formObject);

	// Create a std::string and copy your document data in to the string    
	if (formObject)
		formObject->SetFormData(m_data.Str());
	
}