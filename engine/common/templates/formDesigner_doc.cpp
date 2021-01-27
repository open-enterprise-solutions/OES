#include "formDesigner.h"

#include "window/mainFrame.h"
#include "metadata/metaObjectsDefines.h" 

#include "formDesigner_cmd.h"

wxIMPLEMENT_CLASS(CFormDocument, CDocument);

CCommandProcessor *CFormDocument::CreateCommandProcessor()
{
	CVisualEditorContextForm* m_context = GetFormDesigner()->GetDesignerContext();
	CVisualDesignerCommandProcessor *m_commandprocessor = new CVisualDesignerCommandProcessor(this, m_context);
	m_context->SetCommandProcessor(m_commandprocessor);

	m_commandprocessor->SetEditMenu(mainFrame->GetDefaultMenu(wxID_EDIT));
	m_commandprocessor->Initialize();

	return m_commandprocessor;
}

bool CFormDocument::OnCreate(const wxString& path, long flags)
{
	if (!CDocument::OnCreate(path, flags))
		return false;

	return GetFormDesigner()->LoadForm();
}

bool CFormDocument::OnOpenDocument(const wxString& filename)
{
	// we're not modified by the user yet
	Modify(false);

	return CDocument::OnOpenDocument(filename);
}

#include "forms/visualEditor.h"

bool CFormDocument::OnSaveDocument(const wxString& filename)
{
	return GetFormDesigner()->SaveForm() &&
		CDocument::OnSaveDocument(filename);
}

#include "metadata/metadata.h"
#include "window/property/objinspect.h"

bool CFormDocument::OnCloseDocument()
{
	CVisualEditorContextForm *visualContext = GetFormDesigner()->GetDesignerContext();
	wxASSERT(visualContext);
	CValueFrame *valueFrame = visualContext->GetValueFrame();
	
	if (valueFrame)
	{
		if (!valueFrame->CloseForm()) {
			return false;
		}

		objectInspector->SelectObject(valueFrame->GetFormMetaObject());
	}
	else
	{
		objectInspector->ClearProperty(); 
	}

	return true;
}

bool CFormDocument::IsModified() const
{
	return CDocument::IsModified();
}

void CFormDocument::Modify(bool modified)
{
	CDocument::Modify(modified);
}

// ----------------------------------------------------------------------------
// CTextEditDocument implementation
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(CFormEditDocument, CDocument);

CFormEditView* CFormEditDocument::GetFormDesigner() const
{
	wxView* view = GetFirstView();
	return view ? wxDynamicCast(view, CFormEditView) : NULL;
}

void CFormEditDocument::SetCurrentLine(int lineBreakpoint, bool setBreakpoint)
{
	CFormEditView *m_designerForm = GetFormDesigner();
	wxASSERT(m_designerForm);
	m_designerForm->ActivateEditor();

	CAutocomplectionCtrl *autoComplete = m_designerForm->GetCodeCtrl();
	autoComplete->SetCurrentLine(lineBreakpoint, setBreakpoint);
}