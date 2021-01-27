#ifndef _VALUEQUERY_H__
#define _VALUEQUERY_H__

#include "value.h"

//Поддержка массивов
class CValueGrid : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueGrid);

public:

	CValueGrid();
	virtual ~CValueGrid();

	virtual wxString GetTypeString()const { return wxT("table.document"); }
	virtual wxString GetString()const { return wxT("table.document"); }

	static CMethods m_methods;

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //установка атрибута
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //значение атрибута

	virtual CMethods* GetPMethods() const { return &m_methods; }; //получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                         //этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       //вызов метода

protected:

	bool ShowGrid(const wxString &sTitle);
};

#endif