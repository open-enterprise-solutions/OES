////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, wxFormBuilder
//	Description : visual view events 
////////////////////////////////////////////////////////////////////////////

#include "visualEditorView.h"

void CVisualView::ShowForm()
{
	m_valueFrame->ShowForm();
}

void CVisualView::ActivateForm()
{
	m_valueFrame->ActivateForm();
}

void CVisualView::UpdateForm()
{
	m_valueFrame->UpdateForm();
}

bool CVisualView::CloseForm()
{
	return m_valueFrame->CloseForm();
}
