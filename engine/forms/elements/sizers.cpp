#include "sizers.h"

wxIMPLEMENT_ABSTRACT_CLASS(CValueSizer, IControlElement)

//*******************************************************************
//*                            Control                              *
//*******************************************************************

void CValueSizer::UpdateSizer(wxSizer* sizer)
{
	if (!sizer) return;

	if (m_minimum_size != wxDefaultSize) sizer->SetMinSize(m_minimum_size);
	if (m_minimum_size != wxDefaultSize) sizer->Layout();
}

//*******************************************************************
//*                             Property                            *
//*******************************************************************

void CValueSizer::ReadProperty()
{
	IControlElement::ReadProperty();

	m_properties["minimum_size"]->SetValue(m_minimum_size);

	//if we have sizerItem then call him readpropery 
	IControlElement *m_sizeritem = GetParent();
	if (m_sizeritem && m_sizeritem->GetClassName() == wxT("sizerItem"))
	{
		m_sizeritem->ReadProperty();
	}
}

void CValueSizer::SaveProperty()
{
	IControlElement::SaveProperty();

	m_minimum_size = m_properties["minimum_size"]->GetValueAsSize();

	//if we have sizerItem then call him savepropery 
	IControlElement *m_sizeritem = GetParent();
	if (m_sizeritem && m_sizeritem->GetClassName() == wxT("sizerItem"))
	{
		m_sizeritem->SaveProperty();
	}
}