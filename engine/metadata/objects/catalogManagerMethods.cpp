////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : catalog manager - methods
////////////////////////////////////////////////////////////////////////////

#include "catalogManager.h"
#include "compiler/methods.h"

enum
{
	eCreateElement = 0,
	eCreateGroup,

	eSelect,
	eFindByCode,
	eFindByName,

	eGetListForm,
	eGetSelectForm,

	eEmptyRef
};

#include "metadata/metadata.h"

void CManagerCatalog::PrepareNames() const
{
	IMetadata *metaData = m_metaObject->GetMetadata();
	wxASSERT(metaData);
	IModuleManager *moduleManager = metaData->GetModuleManager();
	wxASSERT(moduleManager);

	SEng aMethods[] =
	{
		{"createElement", "createElement()"},
		{"createGroup", "createGroup()"},

		{"select", "select()"},
		{"findByCode", "findByCode(string)"},
		{"findByName", "findByName(string)"},

		{"getListForm", "getListForm()"},
		{"getSelectForm", "getSelectForm()"},

		{"emptyRef", "emptyRef()"},
	};

	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods->PrepareMethods(aMethods, nCountM);

	CValue *pRefData = moduleManager->FindCommonModule(m_metaObject->GetModuleManager());

	if (pRefData) {

		//добавляем методы из контекста
		for (unsigned int idx = 0; idx < pRefData->GetNMethods(); idx++) {
			m_methods->AppendMethod(pRefData->GetMethodName(idx),
				pRefData->GetMethodDescription(idx),
				wxT("commonModule"));
		}
	}
}

#include "forms/elements/frame.h"

CValue CManagerCatalog::Method(CMethodParameters &aParams)
{
	CValue ret;

	switch (aParams.GetMethodIndex())
	{
	case eCreateElement:
	{
		return m_metaObject->CreateObjectValue();
	}
	case eCreateGroup:
	{
		return m_metaObject->CreateObjectValue();
	}
	case eSelect:
	{
		return m_metaObject->CreateObjectValue();
	}
	case eFindByCode:
	{
		return FindByCode(aParams[0]);
	}
	case eFindByName:
	{
		return FindByName(aParams[0]);
	}
	case eGetListForm:
	{
		CValueGuid *guidVal = aParams.GetParamCount() > 2 ? aParams[2].ConvertToType<CValueGuid *>() : NULL;

		return m_metaObject->GetListForm(aParams[0].GetString(),
			aParams.GetParamCount() > 1 ? aParams[1].ConvertToType<IOwnerInfo *>() : NULL,
			guidVal ? *guidVal : Guid::newGuid());
	}
	case eGetSelectForm:
	{
		CValueGuid *guidVal = aParams.GetParamCount() > 2 ? aParams[2].ConvertToType<CValueGuid *>() : NULL;

		return m_metaObject->GetSelectForm(aParams[0].GetString(),
			aParams.GetParamCount() > 1 ? aParams[1].ConvertToType<IOwnerInfo *>() : NULL,
			guidVal ? *guidVal : Guid::newGuid());
	}
	case eEmptyRef:
	{
		return EmptyRef();
	}
	}

	IMetadata *metaData = m_metaObject->GetMetadata();
	wxASSERT(metaData);
	IModuleManager *moduleManager = metaData->GetModuleManager();
	wxASSERT(moduleManager);

	CValue *pRefData = moduleManager->FindCommonModule(m_metaObject->GetModuleManager());

	if (pRefData) {
		return pRefData->Method(aParams);
	}

	return ret;
}