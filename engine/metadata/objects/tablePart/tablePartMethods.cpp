////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : tabular sections - methods
////////////////////////////////////////////////////////////////////////////

#include "tablePart.h"
#include "compiler/methods.h"
#include "metadata/objects/baseObject.h"
#include "utils/stringutils.h"

CMethods CValueTabularSection::m_methods;
CMethods CValueTabularRefSection::m_methods;

enum
{
	enAdd = 0,
	enClone,
	enCount,
	enClear,
	enLoad,
	enUnload,
	enGetMetadata,
};

//****************************************************************************
//*                              Support methods                             *
//****************************************************************************

CMethods* CValueTabularSection::GetPMethods() const
{
	PrepareNames();
	return &m_methods;
};

CMethods* CValueTabularRefSection::GetPMethods() const
{
	PrepareNames();
	return &m_methods;
};

void CValueTabularSection::PrepareNames() const
{
	SEng aMethods[] =
	{
		{"add","add() - ƒобавление строки"},

		{"clone","clone() - копирование таблицы"},
		{"count","count() - количество строк"},
		{"clear","clear() - ќчистка таблицы"},
		{"load", "load()"},
		{"unload", "unload()"},
		{"getMetadata", "getMetadata()"},
	};

	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods.PrepareMethods(aMethods, nCountM);
}

void CValueTabularRefSection::PrepareNames() const
{
	SEng aMethods[] =
	{
		{"add","add() - ƒобавление строки"},

		{"clone","clone() - копирование таблицы"},
		{"count","count() - количество строк"},
		{"clear","clear() - ќчистка таблицы"},
		{"load", "load()"},
		{"unload", "unload()"},
		{"getMetadata", "getMetadata()"},
	};

	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods.PrepareMethods(aMethods, nCountM);
}

CValue CValueTabularSection::Method(CMethodParameters &aParams)
{
	CValue Ret;

	switch (aParams.GetMethodIndex())
	{
	case enAdd:
	{
		CDataTable tableData;
		for (auto attribute : m_metaTableObj->GetObjectAttributes())
		{
			switch (attribute->GetTypeObject())
			{
			case eValueTypes::TYPE_BOOLEAN: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_BOOLEAN; break;
			case eValueTypes::TYPE_NUMBER: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_NUMBER; break;
			case eValueTypes::TYPE_DATE: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_DATE; break;
			case eValueTypes::TYPE_STRING: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_REFFER; break;
			default: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = new CValueReference(attribute->GetTypeObject()); break;
			}
			
			tableData.aDataTypes[StringUtils::MakeUpper(attribute->GetName())] = attribute->GetTypeObject();
		}

		if (!CTranslateError::IsSimpleMode())
			m_aObjectValues.push_back(tableData);

		return new CValueTabularSectionReturnLine(this, m_aObjectValues.size() - 1);
	}
	
	case enClone: return NULL;//new CValueTabularSection(*this);
	case enCount: return (unsigned int)m_aObjectValues.size();
	case enClear: m_aObjectValues.clear(); break;

	case enLoad: break;
	case enUnload: break;

	case enGetMetadata: return m_metaTableObj;
	}

	return Ret;
}

CValue CValueTabularRefSection::Method(CMethodParameters &aParams)
{
	CValue Ret;

	switch (aParams.GetMethodIndex())
	{
	case enAdd:
	{
		CDataTable tableData;
		for (auto attribute : m_metaTableObj->GetObjectAttributes())
		{
			switch (attribute->GetTypeObject())
			{
			case eValueTypes::TYPE_BOOLEAN: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_BOOLEAN; break;
			case eValueTypes::TYPE_NUMBER: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_NUMBER; break;
			case eValueTypes::TYPE_DATE: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_DATE; break;
			case eValueTypes::TYPE_STRING: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = eValueTypes::TYPE_REFFER; break;
			default: tableData.aDataValues[StringUtils::MakeUpper(attribute->GetName())] = new CValueReference(attribute->GetTypeObject()); break;
			}

			tableData.aDataTypes[StringUtils::MakeUpper(attribute->GetName())] = attribute->GetTypeObject();
		}

		if (!CTranslateError::IsSimpleMode())
			m_aObjectValues.push_back(tableData);

		return new CValueTabularSectionReturnLine(this, m_aObjectValues.size() - 1);
	}

	case enClone: return NULL;//new CValueTabularSection(*this);
	case enCount: return (unsigned int)m_aObjectValues.size();
	case enClear: m_aObjectValues.clear(); break;

	case enLoad: break;
	case enUnload: break;

	case enGetMetadata: return m_metaTableObj;
	}

	return Ret;
}