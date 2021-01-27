#if !defined(_VALUE_TEXT_H__)
#define _VALUE_TEXT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "value.h"
#include "common/templates/module.h"

class CValueText : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueText);
public:
	CValueText();
	virtual ~CValueText();

	enum
	{
		enLinesCount = 0,
		enGetLine,
		enOpen,
		enTemplate,
		enFixTemplate,
		enInsertLine,
		enAddLine,
		enReplaceLine,
		enDeleteLine,
		enReadOnly,
		enShow,
		enClear,
		enCodePage,
		enWrite,
		enGetText,
	};

	wxString sTitle;
	wxArrayString aText;
	CModuleDocument* pDoc;
	BOOL bReadOnly;
	int nCodePage;
	int nTemplate;
	int nFixTemplate;
	wxString GetStr();

	//РАБОТА КАК АГРЕГАТНОГО ОБЪЕКТА
	wxString sObjectName;
	static CMethods m_methods;
	//эти методы нужно переопределить в ваших агрегатных объектах:
	CMethods* GetPMethods() const { return &m_methods; };//получить ссылку на класс помощник разбора имен атрибутов и методов
	void PrepareNames() const;//этот метод автоматически вызывается для инициализации имен атрибутов и методов
	CValue Method(CMethodParameters &aParams);//вызов метода

	wxString GetString()const { return sObjectName; };
	bool Init(wxString StrVid, CValue Param) { sObjectName = StrVid; };
	wxString GetTypeString()const { return sObjectName; };
};

#endif // !defined(_VALUE_TEXT_H__)
