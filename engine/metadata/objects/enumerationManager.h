#ifndef _ENUMERATION_MANAGER_H__
#define _ENUMERATION_MANAGER_H__

#include "enumeration.h"
#include "common/managerInfo.h"

class CManagerEnumeration : public CValue,
	public IMetaManagerInfo
{
	wxDECLARE_DYNAMIC_CLASS(CManagerDocument);

private:

	CMetaObjectEnumerationValue *m_metaObject;

public:

	virtual CMetaCommonModuleObject *GetModuleManager();

	CManagerEnumeration() : m_methods(NULL) {}
	CManagerEnumeration(IMetaObject *metaObject);

	virtual CMethods* GetPMethods() const { PrepareNames(); return m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);//вызов метода

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута

	//types 
	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	virtual ~CManagerEnumeration();

protected:

	//methos 
	CMethods *m_methods;
};

#endif 