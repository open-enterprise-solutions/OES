#include "toolbar.h"
#include "forms/visualEditor.h"
#include "metadata/objects/baseObject.h"

//**********************************************************************************
//*                              Events                                            *
//**********************************************************************************

void CValueToolbar::OnTool(wxCommandEvent &event)
{
	IControlElement *control = FindControlByID(event.GetId());

	if (control)
	{	
		int index = control->GetPropertyAsInteger("action");

		IControlElement *m_sourceElement = m_actionSource != wxNOT_FOUND ? FindControlByID(m_actionSource) : NULL;
		if (m_sourceElement && index != wxNOT_FOUND) m_sourceElement->ExecuteAction(index);
		if (m_visualHostContext) m_visualHostContext->SelectObject(control);
	}

	event.Skip();
}

void CValueToolbar::OnRightDown(wxMouseEvent &event)
{
	event.Skip();
}