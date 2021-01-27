#include "widgets.h"

//*******************************************************************
//*                             Events                              *
//*******************************************************************

void CValueCheckbox::OnClickedCheckbox(wxCommandEvent& event)
{
	wxCheckBox *m_checkbox = wxDynamicCast(event.GetEventObject(), wxCheckBox);
	m_checked = m_checkbox->GetValue();

	event.Skip(ProcessEvent("OnCheckboxClicked"));
}