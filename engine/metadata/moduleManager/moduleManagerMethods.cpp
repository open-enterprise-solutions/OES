////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : module manager - methods
////////////////////////////////////////////////////////////////////////////

#include "moduleManager.h"
#include "compiler/methods.h"
#include "appData.h"

void IModuleManager::PrepareNames() const
{
	std::vector<SEng> aMethods, aAttributes;

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

CValue IModuleManager::Method(CMethodParameters &aParams)
{
	return IModuleInfo::ExecuteMethod(aParams);
}

void IModuleManager::SetAttribute(CAttributeParameters &aParams, CValue &cVal)        //установка атрибута
{
	if (m_procUnit)
		m_procUnit->SetAttribute(aParams, cVal);
}

CValue IModuleManager::GetAttribute(CAttributeParameters &aParams)                   //значение атрибута
{
	if (m_procUnit)
		return m_procUnit->GetAttribute(aParams);
	return CValue();
}

int IModuleManager::FindAttribute(const wxString &sName) const
{
	if (m_procUnit) {
		return m_procUnit->FindAttribute(sName);
	}

	return CValue::FindAttribute(sName);
}

//****************************************************************************
//*                      CExternalModuleManager                              *
//****************************************************************************

void CExternalModuleManager::PrepareNames() const
{
	std::vector<SEng> aMethods, aAttributes;

	if (m_objectData)
	{
		CMethods *dataMethods = m_objectData->GetPMethods();
		wxASSERT(dataMethods);

		for (unsigned int idx = 0; idx < dataMethods->GetNMethods(); idx++) {
			aMethods.emplace_back(dataMethods->GetMethodName(idx),
				dataMethods->GetMethodDescription(idx),
				dataMethods->GetMethodAlias(idx),
				dataMethods->GetMethodPosition(idx));
		}

		for (unsigned int idx = 0; idx < dataMethods->GetNAttributes(); idx++) {
			aAttributes.emplace_back(dataMethods->GetAttributeName(idx),
				wxEmptyString,
				dataMethods->GetAttributeAlias(idx),
				dataMethods->GetAttributePosition(idx));
		}
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

CValue CExternalModuleManager::Method(CMethodParameters &aParams)
{
	if (m_objectData &&
		m_objectData->FindMethod(aParams.GetMethodName()) != wxNOT_FOUND) {
		return m_objectData->Method(aParams);
	}

	return IModuleInfo::ExecuteMethod(aParams);
}

void CExternalModuleManager::SetAttribute(CAttributeParameters &aParams, CValue &cVal)        //установка атрибута
{
	if (m_objectData &&
		m_objectData->FindAttribute(aParams.GetAttributeName()) != wxNOT_FOUND) {
		m_objectData->SetAttribute(aParams, cVal);
	}

	if (m_procUnit) {
		m_procUnit->SetAttribute(aParams, cVal);
	}
}

CValue CExternalModuleManager::GetAttribute(CAttributeParameters &aParams)                   //значение атрибута
{
	if (m_objectData &&
		m_objectData->FindAttribute(aParams.GetAttributeName()) != wxNOT_FOUND) {
		return m_objectData->GetAttribute(aParams);
	}

	if (m_procUnit) {
		return m_procUnit->GetAttribute(aParams);
	}

	return CValue();
}

int CExternalModuleManager::FindAttribute(const wxString &sName) const
{
	if (m_objectData &&
		m_objectData->FindAttribute(sName) != wxNOT_FOUND) {
		return m_objectData->FindAttribute(sName);
	}

	if (m_procUnit) {
		return m_procUnit->FindAttribute(sName);
	}

	return CValue::FindAttribute(sName);
}