////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : value type description (collection types)
////////////////////////////////////////////////////////////////////////////

#include "valueTypeDescription.h"
#include "valueType.h"
#include "valueArray.h"
#include "methods.h"

wxIMPLEMENT_DYNAMIC_CLASS(CValueTypeDescription, CValue);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMethods CValueTypeDescription::m_methods;

CValueTypeDescription::CValueTypeDescription() : CValue(eValueTypes::TYPE_VALUE, true) {}

CValueTypeDescription::CValueTypeDescription(CValueType *valueType) : CValue(eValueTypes::TYPE_VALUE, true) { m_aTypes.push_back(valueType->GetOwnerType()); }

CValueTypeDescription::~CValueTypeDescription() {}

enum
{
	enContainsType = 0,
	enAdjustValue,
	enTypes
};

void CValueTypeDescription::PrepareNames() const
{
	SEng aMethods[] = {
		{"containsType","containsType(type)"},
		{"adjustValue","adjustValue(value)"},
		{"types","types()"},
	};
	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods.PrepareMethods(aMethods, nCountM);
}

CValue CValueTypeDescription::Method(CMethodParameters &aParams)
{
	switch (aParams.GetMethodIndex())
	{
	case enContainsType: return ContainsType(aParams[0]);
	case enAdjustValue: return AdjustValue(aParams[0]);
	case enTypes: return Types();
	}

	return CValue();
}

bool CValueTypeDescription::Init(CValue **aParams)
{
	if (aParams[0]->GetType() == eValueTypes::TYPE_STRING)
	{
		wxString classType = aParams[0]->GetString(); 

		if (CValue::IsRegisterObject(classType))
		{
			m_aTypes.push_back(CValue::GetIDObjectFromString(classType));
			return true;
		}
	}

	CValueArray *m_valArray = NULL;

	if (aParams[0]->ConvertToValue(m_valArray))
	{
		for (unsigned int i = 0; i < m_valArray->Count(); i++)
		{
			CValueType *m_valType = value_cast<CValueType *>(m_valArray->GetAt(i));
			wxASSERT(m_valType);
			m_aTypes.push_back(m_valType->GetOwnerType());
		}

		std::sort(m_aTypes.begin(), m_aTypes.end());

		return true;
	}

	CValueType *m_valType = NULL;

	if (aParams[0]->ConvertToValue(m_valType))
	{
		m_aTypes.push_back(m_valType->GetOwnerType());

		return true;
	}

	return false;
}

bool CValueTypeDescription::ContainsType(CValue &cType)
{
	CValueType *valueType = value_cast<CValueType *>(cType);
	wxASSERT(valueType);
	auto itFounded = std::find(m_aTypes.begin(), m_aTypes.begin(), valueType->GetOwnerType());
	return itFounded != m_aTypes.end();
}

CValue CValueTypeDescription::AdjustValue(CValue &cVal)
{
	auto foundedIt = std::find(m_aTypes.begin(), m_aTypes.end(), cVal.GetTypeID());

	if (foundedIt != m_aTypes.end())
		return cVal;

	for (auto type_id : m_aTypes)
	{
		if (type_id < eValueTypes::TYPE_REFFER)
		{
			switch (type_id)
			{
			case eValueTypes::TYPE_BOOLEAN: return cVal.GetBoolean();
			case eValueTypes::TYPE_NUMBER: return cVal.GetNumber();
			case eValueTypes::TYPE_STRING: return cVal.GetString();
			case eValueTypes::TYPE_DATE: return cVal.GetDate();
			default: return CValue(cVal.m_typeClass);
			}
		}
	}

	if (m_aTypes.size() == 1)
	{
		wxString className = CValue::GetNameObjectFromID(m_aTypes[0]);
		return CValue::CreateObject(className);
	}

	return CValue();
}

CValue CValueTypeDescription::Types()
{
	std::vector<CValue> allTypes;
	for (auto type_id : m_aTypes) { allTypes.push_back(new CValueType(type_id)); }
	return new CValueArray(allTypes);
}

//**********************************************************************
//*                       Runtime register                             *
//**********************************************************************

VALUE_REGISTER(CValueTypeDescription, "typeDescription", TEXT2CLSID("VL_TYDE"));