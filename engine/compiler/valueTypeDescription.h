#ifndef _TYPE_DESCRIPTION_H__
#define _TYPE_DESCRIPTION_H__

#include "value.h"

class CValueTypeDescription : public CValue
{
	wxDECLARE_DYNAMIC_CLASS_NO_COPY(CValueTypeDescription);

private:

	static CMethods m_methods;
	std::vector<unsigned long> m_aTypes;

public:

	//эти методы нужно переопределить в ваших агрегатных объектах:
	virtual CMethods* GetPMethods() const { return &m_methods; };//получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;//этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);//вызов метода

public:

	CValueTypeDescription();
	CValueTypeDescription(class CValueType *valueType);
	virtual ~CValueTypeDescription();

	virtual bool Init() { return false; }
	virtual bool Init(CValue **aParams);

	virtual wxString GetTypeString() const { return wxT("typeDescription"); }
	virtual wxString GetString() const { return wxT("typeDescription"); }

public:

	std::vector<unsigned long> GetLongTypes() { return m_aTypes; }

public:

	bool ContainsType(CValue &cType);
	CValue AdjustValue(CValue &cVal);
	CValue Types();
};

#endif 