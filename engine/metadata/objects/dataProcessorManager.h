#ifndef _MANAGER_DATAPROCESSOR_H__
#define _MANAGER_DATAPROCESSOR_H__

#include "dataProcessor.h"
#include "common/managerInfo.h"

class CManagerDataProcessor : public CValue, 
	public IMetaManagerInfo
{
	wxDECLARE_DYNAMIC_CLASS(CManagerDataProcessor);

private:

	CMetaObjectDataProcessorValue *m_metaObject;

public:

	virtual CMetaCommonModuleObject *GetModuleManager();

	CManagerDataProcessor() : m_methods(NULL) {}
	CManagerDataProcessor(IMetaObject *metaObject);

	virtual CMethods* GetPMethods() const { PrepareNames(); return m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);//вызов метода

	//types 
	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	virtual ~CManagerDataProcessor();

protected:

	//methos 
	CMethods *m_methods;
};

#endif 