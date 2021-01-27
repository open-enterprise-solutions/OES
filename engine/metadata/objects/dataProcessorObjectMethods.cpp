////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : dataProcessor - methods
////////////////////////////////////////////////////////////////////////////

#include "dataProcessor.h"
#include "compiler/methods.h"
#include "appData.h"

enum
{
	enGetForm = 0,
	enGetMetadata
};

//****************************************************************************
//*                              Support methods                             *
//****************************************************************************

CMethods* CObjectDataProcessorValue::GetPMethods() const
{
	CObjectDataProcessorValue::PrepareNames();
	return m_methods;
};

void CObjectDataProcessorValue::PrepareNames() const
{
	std::vector<SEng> aMethods, aAttributes;

	aMethods = {

	{"getFormObject", "getFormObject()"},
	{"getMetadata", "getMetadata()"},
	};

	{
		//add self 
		SEng attribute;
		attribute.sName = thisObject;
		attribute.sAlias = wxT("system");
		aAttributes.push_back(attribute);
	}

	//fill custom attributes 
	for (auto attributes : m_metaObject->GetObjectAttributes())
	{
		SEng attribute;
		attribute.sName = attributes->GetName();
		attribute.iName = attributes->GetMetaID();
		attribute.sAlias = wxT("attribute");
		aAttributes.push_back(attribute);
	}

	//fill custom tables 
	for (auto tables : m_metaObject->GetObjectTables())
	{
		SEng table;
		table.sName = tables->GetName();
		table.iName = tables->GetMetaID();
		table.sAlias = wxT("table");
		aAttributes.push_back(table);
	}

	if (m_procUnit)
	{
		CByteCode *m_byteCode = m_procUnit->GetByteCode();

		for (auto exportFunction : m_byteCode->m_aExportFuncList)
		{
			SEng methods;
			methods.sName = exportFunction.first;
			methods.sAlias = wxT("procUnit");
			methods.iName = exportFunction.second;
			aMethods.push_back(methods);
		}

		for (auto exportVariable : m_byteCode->m_aExportVarList)
		{
			SEng attributes;
			attributes.sName = exportVariable.first;
			attributes.sAlias = wxT("procUnit");
			attributes.iName = exportVariable.second;
			aAttributes.push_back(attributes);
		}
	}

	m_methods->PrepareAttributes(aAttributes.data(), aAttributes.size());
	m_methods->PrepareMethods(aMethods.data(), aMethods.size());
}

#include "forms/elements/frame.h"

CValue CObjectDataProcessorValue::Method(CMethodParameters &aParams)
{
	switch (aParams.GetMethodIndex())
	{
	case enGetForm: return GetFrameValue();
	case enGetMetadata: return m_metaObject;
	}

	return IModuleInfo::ExecuteMethod(aParams);
}