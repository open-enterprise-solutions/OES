////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : catalog object - methods
////////////////////////////////////////////////////////////////////////////

#include "catalog.h"
#include "compiler/methods.h"
#include "appData.h"

enum
{
	enIsNew = 0,

	enCopy,
	enFill,
	enWrite,
	enDelete,

	enGetForm,
	enGetMetadata
};

//****************************************************************************
//*                              Support methods                             *
//****************************************************************************

CMethods* CObjectCatalogValue::GetPMethods() const
{
	CObjectCatalogValue::PrepareNames();
	return m_methods;
};

void CObjectCatalogValue::PrepareNames() const
{
	std::vector<SEng> aMethods, aAttributes;

	aMethods = {
	{"isNew", "isNew()"},

	{"copy", "copy()"},
	{"fill", "fill(object)"},
	{"write", "write()"},
	{"delete", "delete()"},

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

	{
		//add ref  
		SEng attribute;
		attribute.sName = wxT("reference");
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

CValue CObjectCatalogValue::Method(CMethodParameters &aParams)
{
	switch (aParams.GetMethodIndex())
	{
	case enIsNew: return m_bNewObject;
	case enCopy: return CopyObject();
	case enFill: FillObject(aParams[0]); break;
	case enWrite: WriteObject(); break;
	case enDelete: DeleteObject(); break;
	case enGetForm: return GetFrameValue();
	case enGetMetadata: return m_metaObject;
	}

	return IModuleInfo::ExecuteMethod(aParams);
}