////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : frame object
////////////////////////////////////////////////////////////////////////////

#include "frame.h"
#include "appData.h"
#include "metadata/metadata.h"
#include "common/reportManager.h"
#include "metadata/moduleManager/moduleManager.h"
#include "compiler/systemObjects.h"
#include "forms/visualEditorView.h"

//*************************************************************************************************
//*                                    System attribute                                           *
//*************************************************************************************************

bool CValueFrame::CloseFrame()
{
	if (!m_valueFrameDocument) return false;

	CValue bCancel = false;

	wxString sEventNameBeforeClose = m_events["beforeClose"]->GetValue();

	if (!sEventNameBeforeClose.IsEmpty() && m_procUnit) {
		m_procUnit->CallFunction(sEventNameBeforeClose, bCancel);
	}

	if (bCancel.GetBoolean()) return false;

	wxString sEventNameOnClose = m_events["onClose"]->GetValue();

	if (!sEventNameOnClose.IsEmpty() && m_procUnit) {
		m_procUnit->CallFunction(sEventNameOnClose);
	}

	return true;
}

#include "metadata/objects/baseObject.h"

bool CValueFrame::InitializeModule(bool runModule)
{
	IMetadata *metaData = m_metaFormObject->GetMetadata();
	wxASSERT(metaData);
	IModuleManager *moduleManager = metaData->GetModuleManager();
	wxASSERT(moduleManager);

	IModuleInfo *sourceObjectValue = dynamic_cast<IModuleInfo *>(m_sourceObject);

	if (!m_compileModule)
	{
		m_compileModule = new CCompileModule(m_metaFormObject);
		m_compileModule->SetParent(sourceObjectValue ? sourceObjectValue->GetCompileModule() : moduleManager->GetCompileModule());
		m_compileModule->AddContextVariable(thisForm, this);
	}

	try
	{
		m_compileModule->Compile();
	}
	catch (const CTranslateError *err)
	{
		if (appData->IsEnterpriseMode())
			CSystemObjects::Raise(err->what());
		else
			CSystemObjects::Message(err->what());

		return false;
	};

	if (appData->IsEnterpriseMode())
	{
		if (!m_procUnit)
		{
			m_procUnit = new CProcUnit();
			m_procUnit->SetParent(sourceObjectValue ? sourceObjectValue->GetProcUnit() : moduleManager->GetProcUnit());
		}

		m_procUnit->Execute(m_compileModule->m_cByteCode, runModule);
	}

	return true;
}

#include "compiler/valueType.h"

CValue CValueFrame::CreateControl(const CValueType *classControl, const CValue &vControl)
{
	if (appData->IsDesignerMode())
		return CValue();

	if (!CValue::IsRegisterObject(classControl->GetString(), eObjectType::eObjectType_object_control))
		CSystemObjects::Raise("Ñlass does not belong to control!");

	//get parent obj
	IControlElement *m_parentControl = NULL;

	if (!vControl.IsEmpty())
		m_parentControl = value_cast<IControlElement *>(vControl);
	else
		m_parentControl = this;

	// ademas, el objeto se insertara a continuacion del objeto seleccionado
	IControlElement *m_newControl = visualEditorDatabase->CreateObject(classControl->GetString(), m_parentControl);

	CVisualEditorContextForm::ResolveNameConflict(m_newControl);

	if (!CTranslateError::IsSimpleMode())
	{
		m_newControl->SetParent(m_parentControl);
		m_parentControl->AddChild(m_newControl);

		if (m_valueFrameDocument)
		{
			CVisualView *m_visualView = m_valueFrameDocument->GetVisualView();
			m_visualView->CreateControl(m_newControl);

			//fix size in parent window 
			wxWindow *m_wndParent = m_visualView->GetParent();
			if (m_wndParent) m_wndParent->Layout();
		}

		m_newControl->IncrRef();
	}

	//return value 
	if (m_newControl->GetClassName() == wxT("sizerItem"))
		return m_newControl->GetChild(0);

	return m_newControl;
}

void CValueFrame::RemoveControl(const CValue &vControl)
{
	if (appData->IsDesignerMode())
		return;

	//get parent obj
	IControlElement *m_currentControl = value_cast<IControlElement *>(vControl);

	wxASSERT(m_currentControl);

	if (!CTranslateError::IsSimpleMode())
	{
		if (m_valueFrameDocument)
		{
			CVisualView *m_visualView = m_valueFrameDocument->GetVisualView();
			m_visualView->RemoveControl(m_currentControl);

			//fix size in parent window 
			wxWindow *m_wndParent = m_visualView->GetParent();
			if (m_wndParent) m_wndParent->Layout();
		}
	}

	IControlElement *m_parentControl = m_currentControl->GetParent();

	if (m_parentControl->GetClassName() == wxT("sizerItem"))
	{
		IControlElement *m_parentOwner = m_parentControl->GetParent();

		if (m_parentOwner)
			m_parentOwner->RemoveChild(m_currentControl);

		m_parentControl->SetParent(NULL);
		m_parentControl->DecrRef();
	}

	m_parentControl->RemoveChild(m_currentControl);
	m_currentControl->SetParent(NULL);
	m_currentControl->DecrRef();
}

//*************************************************************************************************
//*                                              Events                                           *
//*************************************************************************************************

void CValueFrame::ShowForm(bool demonstration)
{
	if (CTranslateError::IsSimpleMode())
		return;

	if (m_valueFrameDocument)
	{
		ActivateForm();
		return;
	}
	else if (!demonstration)
	{
		CValue::IncrRef();
	}

	if (appData->IsEnterpriseMode())
	{
		CValue bCancel = false;

		wxString sEventNameBeforeOpen = m_events["beforeOpen"]->GetValue();

		if (!sEventNameBeforeOpen.IsEmpty() && m_procUnit) {
			m_procUnit->CallFunction(sEventNameBeforeOpen, bCancel);
		}

		if (bCancel.GetBoolean()) return;

		wxString sEventNameOnOpen = m_events["onOpen"]->GetValue();

		if (!sEventNameOnOpen.IsEmpty() && m_procUnit) {
			m_procUnit->CallFunction(sEventNameOnOpen);
		}
	}

	if (demonstration || appData->IsEnterpriseMode()) ShowDocumentForm(demonstration);
}

void CValueFrame::ActivateForm()
{
	if (m_valueFrameDocument) m_valueFrameDocument->Activate();
}

void CValueFrame::UpdateForm()
{
	if (CTranslateError::IsSimpleMode()) return;

	if (m_valueFrameDocument)
	{
		CVisualView *m_visualView = m_valueFrameDocument->GetVisualView();
		if (m_visualView) m_visualView->UpdateFrame();
	}
}

bool CValueFrame::CloseForm()
{
	if (CTranslateError::IsSimpleMode()) return false;

	if (appData->IsEnterpriseMode())
	{
		if (!CloseFrame()) return false;
	}

	if (m_valueFrameDocument) { m_valueFrameDocument->DeleteAllViews(); m_valueFrameDocument = NULL; }
	return true;
}
