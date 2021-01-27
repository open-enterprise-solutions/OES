////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : frame methods
////////////////////////////////////////////////////////////////////////////

#include "frame.h"
#include "compiler/methods.h"
#include "appData.h"

enum
{
	enShow = 0,
	enActivate,
	enUpdate,
	enClose,

	enModify,
	enIsModified,
	enIsShown,

	enGetGuid,
};

//****************************************************************************
//*                              Support methods                             *
//****************************************************************************

void CValueFrame::PrepareNames() const
{
	std::vector<SEng> aMethods, aAttributes;

	aMethods = {
	{"show", "show()"},
	{"activate", "activate()"},
	{"update", "update()"},
	{"close", "close()"},

	{"modify", "modify(boolean)"},
	{"isModified", "isModified()"},
	{"isShown", "isShown()"},

	{"getGuid", "getGuid()"}
	};

	//default element
	SEng attributesThisForm;
	attributesThisForm.sName = thisForm;
	attributesThisForm.iName = 1;
	attributesThisForm.sAlias = wxT("system");
	aAttributes.push_back(attributesThisForm);

	SEng attributesControls;
	attributesControls.sName = wxT("controls");
	attributesControls.iName = 2;
	attributesControls.sAlias = wxT("system");
	aAttributes.push_back(attributesControls);

	//from property 
	for (auto property : m_properties)
	{
		SEng attributes;
		attributes.sName = property.first;
		attributes.iName = GetPropertyIndex(property.first);
		attributes.sAlias = wxT("attribute");
		aAttributes.push_back(attributes);
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

#include "compiler/valueGuid.h"
#include "compiler/valueType.h"

CValue CValueFrame::Method(CMethodParameters &aParams)
{
	switch (aParams.GetMethodIndex())
	{
	case enShow: ShowForm(); break;
	case enActivate: ActivateForm(); break;
	case enUpdate:  UpdateForm(); break;
	case enClose: return CloseForm();
	case enModify:  Modify(aParams[0].GetBoolean()); break;
	case enIsModified: return IsModified();
	case enIsShown:  return IsShown();

	case enGetGuid: return new CValueGuid(m_guidForm);
	}

	return IModuleInfo::ExecuteMethod(aParams);
}