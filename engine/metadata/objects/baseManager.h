#ifndef _MANAGER_BASE_H__
#define _MANAGER_BASE_H__

#include "compiler/value.h"

class CManagerBase : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CManagerBase);

public:

	CManagerBase();
	CManagerBase(IMetadata *metaData);

	virtual ~CManagerBase();

	virtual CMethods* GetPMethods() const { PrepareNames();  return m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов

	//attributes
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута

	//types 
	virtual wxString GetTypeString() const { return wxT("manager"); }
	virtual wxString GetString() const { return wxT("manager"); }

protected:

	//methos 
	CMethods *m_methods;

	//metadata 
	IMetadata *m_metaData;
};

#endif 