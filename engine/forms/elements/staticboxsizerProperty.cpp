#include "sizers.h"

void CValueStaticBoxSizer::SetPropertyData(Property *property, const CValue &srcValue)
{
	IControlElement::SetPropertyData(property, srcValue);
}

#include "forms/special/enums/valueOrient.h"

CValue CValueStaticBoxSizer::GetPropertyData(Property *property)
{
	if (property->GetName() == wxT("orient"))
	{
		return new CValueEnumOrient(m_orient);
	}

	return IControlElement::GetPropertyData(property);
}