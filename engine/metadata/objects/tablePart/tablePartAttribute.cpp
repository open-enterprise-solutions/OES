////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : tabular sections - attributes
////////////////////////////////////////////////////////////////////////////

#include "tablePart.h"
#include "compiler/methods.h"
#include "../reference/reference.h"
#include "utils/stringutils.h"

//****************************************************************************
//*                              Override attribute                          *
//****************************************************************************
void CValueTabularSection::SetAttribute(CAttributeParameters &aParams, CValue &vObject) {}

void CValueTabularRefSection::SetAttribute(CAttributeParameters &aParams, CValue &vObject) {}

CValue CValueTabularSection::GetAttribute(CAttributeParameters &aParams) { return CValue(); }

CValue CValueTabularRefSection::GetAttribute(CAttributeParameters &aParams) { return CValue(); }

//****************************************************************************
//*                              Override attribute                          *
//****************************************************************************

void IValueTabularSection::CValueTabularSectionReturnLine::SetAttribute(CAttributeParameters &aParams, CValue &vObject)
{
	unsigned int typeObj = m_ownerTable->m_aObjectValues[m_lineTable].aDataTypes[StringUtils::MakeUpper(aParams.GetAttributeName())];

	switch (typeObj)
	{
	case eValueTypes::TYPE_BOOLEAN: m_ownerTable->m_aObjectValues[m_lineTable].aDataValues[StringUtils::MakeUpper(aParams.GetAttributeName())] = vObject.GetBoolean(); break;
	case eValueTypes::TYPE_NUMBER: m_ownerTable->m_aObjectValues[m_lineTable].aDataValues[StringUtils::MakeUpper(aParams.GetAttributeName())] = vObject.GetNumber(); break;
	case eValueTypes::TYPE_DATE: m_ownerTable->m_aObjectValues[m_lineTable].aDataValues[StringUtils::MakeUpper(aParams.GetAttributeName())] = vObject.GetDate(); break;
	case eValueTypes::TYPE_STRING: m_ownerTable->m_aObjectValues[m_lineTable].aDataValues[StringUtils::MakeUpper(aParams.GetAttributeName())] = vObject.GetString(); break;
	default:
	{
		CValueReference *m_reference = dynamic_cast<CValueReference *>(vObject.GetRef());
		if (m_reference && (typeObj == m_reference->GetTypeRef())) m_ownerTable->m_aObjectValues[m_lineTable].aDataValues[StringUtils::MakeUpper(aParams.GetAttributeName())] = m_reference;
		//else m_ownerTable->m_aObjectValues[m_lineTable].aDataValues[StringUtils::MakeUpper(aParams.GetAttributeName())] = new CValueReference(m_ownerTable->m_metaObject);
	}
	}
}

CValue IValueTabularSection::CValueTabularSectionReturnLine::GetAttribute(CAttributeParameters &aParams)
{
	return m_ownerTable->m_aObjectValues[m_lineTable].aDataValues[StringUtils::MakeUpper(aParams.GetAttributeName())];
}
