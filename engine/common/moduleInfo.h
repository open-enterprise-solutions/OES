#ifndef _MODULEINFO_H__
#define _MODULEINFO_H__

#include "compiler/procUnit.h"

class IModuleInfo
{
public:

	IModuleInfo();
	IModuleInfo(CCompileModule *compileModule);

	virtual ~IModuleInfo();

	CMetaModuleObject *GetMetaObject() const { return GetCompileModule() ? GetCompileModule()->GetModuleObject() : NULL; }

	virtual CCompileModule *GetCompileModule() const { return m_compileModule; }
	virtual CProcUnit *GetProcUnit() const { return m_procUnit; }

	//גûחמג לועמהא
	CValue ExecuteMethod(CMethodParameters &aParams);

protected:

	CCompileModule *m_compileModule;
	CProcUnit *m_procUnit;
};

#endif 