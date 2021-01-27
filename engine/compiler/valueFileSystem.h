#ifndef _VALUEFILESYSYEM_H__
#define _VALUEFILESYSYEM_H__

#include "value.h"

class CValueFileSystem : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueFileSystem);
public:
	CValueFileSystem();
	virtual ~CValueFileSystem();
	int nTimer;
	HANDLE hFindFile;
	std::map<wxString, void *> aListHandle;


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

	void CloseAllHandle();
};

#endif