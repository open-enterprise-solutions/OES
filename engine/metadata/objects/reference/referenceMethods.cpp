////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : reference - methods
////////////////////////////////////////////////////////////////////////////

#include "reference.h"
#include "compiler/methods.h"
#include "metadata/objects/baseObject.h"
#include "metadata/metaObjects/tables/tables.h"

enum
{
	enIsEmpty = 0,
	enGetObject,
	enGetMetadata,
	enGetGuid
};

//****************************************************************************
//*                              Support methods                             *
//****************************************************************************

CMethods* CValueReference::GetPMethods() const
{
	PrepareNames();
	return m_methods;
};

void CValueReference::PrepareNames() const
{
	if (m_metaObject->isEnumeration()) 
		return;

	SEng aMethods[] =
	{
		{"isEmpty", "IsEmpty()"},
		{"getObject", "getObject()"},
		{"getMetadata", "getMetadata()"},
		{"getGuid", "getGuid()"},
	};

	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods->PrepareMethods(aMethods, nCountM);

	std::vector<SEng> aAttributes;

	//added reference 
	SEng attribute;
	attribute.sName = wxT("reference");
	attribute.iName = m_metaObject->GetMetaID();
	attribute.sAlias = wxT("reference");
	aAttributes.push_back(attribute);

	//fill custom attributes 
	for (auto attributes : m_metaObject->GetObjectAttributes())
	{
		SEng attribute;
		attribute.sName = attributes->GetName();
		attribute.iName = attributes->GetMetaID();
		attribute.sAlias = wxT("attribute");
		aAttributes.push_back(attribute);
	}

	//fill part tables attributes 
	for (auto table : m_metaObject->GetObjectTables())
	{
		SEng attribute;
		attribute.sName = table->GetName();
		attribute.iName = table->GetMetaID();
		attribute.sAlias = wxT("table");
		aAttributes.push_back(attribute);
	}

	m_methods->PrepareAttributes(aAttributes.data(), aAttributes.size());
}

CValue CValueReference::Method(CMethodParameters &aParams)
{
	CValue Ret;

	switch (aParams.GetMethodIndex())
	{
	case enIsEmpty: return IsEmptyRef();
	case enGetObject: return GetObject();
	case enGetMetadata: return GetMetadata();
	case enGetGuid: return new CValueGuid(m_objGuid);
	}

	return Ret;
}