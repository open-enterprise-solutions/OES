#include "formDesigner.h"
#include "forms/visualEditor.h"

CValue CFormDocument::GetCommonValueObject()
{
	CFormEditView *m_editview = GetFormDesigner();
	
	if (m_editview)
	{
		CVisualEditorContextForm *m_visualView = m_editview->GetDesignerContext();
		return m_visualView->GetValueFrame();
	}

	return CValue();
}