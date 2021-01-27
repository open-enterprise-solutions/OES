////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : reference - attributes
////////////////////////////////////////////////////////////////////////////

#include "reference.h"
#include "compiler/methods.h"
#include "metadata/objects/tablePart/tablePart.h"
#include "utils/stringutils.h"

//****************************************************************************
//*                              Override attribute                          *
//****************************************************************************
void CValueReference::SetAttribute(CAttributeParameters &aParams, CValue &value) {}

CValue CValueReference::GetAttribute(CAttributeParameters &aParams)
{
	wxString sAlias = m_methods->GetAttributeAlias(aParams.GetAttributeIndex());

	if (IsEmpty())
	{
		if (sAlias != wxT("reference"))
		{
			unsigned int typeObj = m_aObjectTypes[StringUtils::MakeUpper(aParams.GetAttributeName())];

			if (typeObj != wxNOT_FOUND)
			{
				switch (typeObj)
				{
				case eValueTypes::TYPE_BOOLEAN: return eValueTypes::TYPE_BOOLEAN;
				case eValueTypes::TYPE_NUMBER: return eValueTypes::TYPE_NUMBER;
				case eValueTypes::TYPE_DATE: return eValueTypes::TYPE_DATE;
				case eValueTypes::TYPE_STRING: return eValueTypes::TYPE_STRING;
				default: return new CValueReference(typeObj);
				}
			}
			else return m_aObjectValues[StringUtils::MakeUpper(aParams.GetAttributeName())];
		}
		else
		{
			return new CValueReference(m_metaObject);
		}
	}

	if (sAlias == wxT("reference")) {
		return new CValueReference(m_metaObject, m_objGuid);
	}
	else {
		return m_aObjectValues[StringUtils::MakeUpper(aParams.GetAttributeName())];
	}
}