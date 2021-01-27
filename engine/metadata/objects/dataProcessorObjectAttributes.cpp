////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : dataProcessor - attributes
////////////////////////////////////////////////////////////////////////////

#include "dataProcessor.h"
#include "compiler/methods.h"
#include "reference/reference.h"
#include "utils/stringutils.h"

//****************************************************************************
//*                              Override attribute                          *
//****************************************************************************
void CObjectDataProcessorValue::SetAttribute(CAttributeParameters &aParams, CValue &vObject)        //��������� ��������
{
	wxString sName = GetAttributeName(aParams.GetAttributeIndex()), sAlias = m_methods->GetAttributeAlias(aParams.GetAttributeIndex());

	if (sAlias == wxT("procUnit")) { if (m_procUnit) m_procUnit->SetAttribute(aParams.GetAttributeName(), vObject); }
	else if (sAlias == wxT("attribute"))
	{
		int typeObj = m_aObjectTypes[StringUtils::MakeUpper(sName)];
		switch (typeObj)
		{
		case eValueTypes::TYPE_BOOLEAN: m_aObjectValues[StringUtils::MakeUpper(sName)] = vObject.GetBoolean(); break;
		case eValueTypes::TYPE_NUMBER: m_aObjectValues[StringUtils::MakeUpper(sName)] = vObject.GetNumber(); break;
		case eValueTypes::TYPE_DATE: m_aObjectValues[StringUtils::MakeUpper(sName)] = vObject.GetDate(); break;
		case eValueTypes::TYPE_STRING: m_aObjectValues[StringUtils::MakeUpper(sName)] = vObject.GetString(); break;
		default:
		{
			CValueReference *m_reference = dynamic_cast<CValueReference *>(vObject.GetRef());
			if (m_reference && (typeObj == m_reference->GetTypeRef())) m_aObjectValues[StringUtils::MakeUpper(sName)] = m_reference;
		}
		}
	}
}

CValue CObjectDataProcessorValue::GetAttribute(CAttributeParameters &aParams)                   //�������� ��������
{
	wxString sName = GetAttributeName(aParams.GetAttributeIndex()), sAlias = m_methods->GetAttributeAlias(aParams.GetAttributeIndex());

	if (sAlias == wxT("procUnit")) { if (m_procUnit) return m_procUnit->GetAttribute(aParams.GetAttributeName()); }
	else if (sAlias == wxT("attribute") || sAlias == wxT("table")) { return m_aObjectValues[StringUtils::MakeUpper(sName)]; }
	else if (sAlias == wxT("system")) return this;
	
	return CValue();
}