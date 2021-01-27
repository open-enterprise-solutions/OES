////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : dataProcessor - methods
////////////////////////////////////////////////////////////////////////////

#include "dataProcessorManager.h"
#include "compiler/methods.h"

enum
{
	eCreate = 0,
	eGetForm
};

#include "metadata/metadata.h"

void CManagerDataProcessor::PrepareNames() const
{
	IMetadata *metaData = m_metaObject->GetMetadata();
	wxASSERT(metaData);
	IModuleManager *moduleManager = metaData->GetModuleManager();
	wxASSERT(moduleManager);

	SEng aMethods[] =
	{
		{"create", "create()"},
		{"getForm", "getForm(string)"},
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

CValue CManagerDataProcessor::Method(CMethodParameters &aParams)
{
	CValue ret;

	switch (aParams.GetMethodIndex())
	{
	case eCreate:
	{
		return m_metaObject->CreateObjectValue();
	}
	case eGetForm:
	{
		CValueGuid *guidVal = aParams.GetParamCount() > 2 ? aParams[2].ConvertToType<CValueGuid *>() : NULL;

		return m_metaObject->GetObjectForm(aParams[0].GetString(),
			aParams.GetParamCount() > 1 ? aParams[1].ConvertToType<IOwnerInfo *>() : NULL,
			guidVal ? *guidVal : Guid::newGuid());
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