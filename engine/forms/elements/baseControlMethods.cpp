////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : base control - methods
////////////////////////////////////////////////////////////////////////////

#include "baseControl.h"
#include "compiler/methods.h"

//****************************************************************************
//*                              Support methods                             *
//****************************************************************************

void IControlElement::PrepareNames() const
{
	std::vector<SEng> aAttributes;

	for (auto property : m_properties)
	{
		SEng attributes;
		attributes.sName = property.first;
		attributes.sAlias = wxT("attribute");
		aAttributes.push_back(attributes);
	}

	//if we have sizerItem then call him  
	IControlElement *m_sizeritem = GetParent();
	if (m_sizeritem && m_sizeritem->GetClassName() == wxT("sizerItem"))
	{
		for (unsigned int i = 0; i < m_sizeritem->GetPropertyCount(); i++)
		{
			Property *property = m_sizeritem->GetProperty(i);

			SEng attributes;
			attributes.sName = property->GetName();
			attributes.sAlias = wxT("sizerItem");
			attributes.iName = i; 
			aAttributes.push_back(attributes);
		}
	}

	m_methods->PrepareAttributes(aAttributes.data(), aAttributes.size());
}

CValue IControlElement::Method(CMethodParameters &aParams)
{
	return CValue();
}