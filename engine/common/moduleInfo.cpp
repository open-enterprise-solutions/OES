////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko 
//	Description : module information for CValue 
////////////////////////////////////////////////////////////////////////////

#include "moduleInfo.h"
#include "compiler/definition.h"
#include "appData.h"

IModuleInfo::IModuleInfo() : m_compileModule(NULL), m_procUnit(NULL) {}

IModuleInfo::IModuleInfo(CCompileModule *compileModule) : m_compileModule(compileModule), m_procUnit(NULL) {}

IModuleInfo::~IModuleInfo()
{
	if (m_compileModule) delete m_compileModule;
	if (m_procUnit) delete m_procUnit;
}

CValue IModuleInfo::ExecuteMethod(CMethodParameters &aParams)
{
	if (m_procUnit)
	{
		if (m_procUnit->FindFunction(aParams.GetMethodName()) != wxNOT_FOUND) {
			return m_procUnit->CallFunction(aParams);
		}

		return new CValueNoRet(aParams.GetMethodName());
	}

	return CValue();
}