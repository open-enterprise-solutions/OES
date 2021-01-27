////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : metaobject methods
////////////////////////////////////////////////////////////////////////////

#include "metaObject.h"
#include "compiler/methods.h"

//****************************************************************************
//*                              Support methods                             *
//****************************************************************************

void IMetaObject::PrepareNames() const
{
	std::vector<SEng> aAttributes;

	for (auto property : m_properties)
	{
		SEng attributes;
		attributes.sName = property.first;
		aAttributes.push_back(attributes);
	}

	m_methods->PrepareAttributes(aAttributes.data(), aAttributes.size());
}

CValue IMetaObject::Method(CMethodParameters &aParams)
{
	return CValue();
}