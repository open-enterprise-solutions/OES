#ifndef _ENUMFACTORY_H__
#define _ENUMFACTORY_H__

#include "value.h"

#define enumFactory         	(CEnumFactory::Get())
#define enumFactoryDestroy()  	(CEnumFactory::Destroy())

//realization factory pattern 
class CEnumFactory : public CValue
{
	static CEnumFactory *s_instance;

private:

	CEnumFactory();

public:

	~CEnumFactory();

	static CEnumFactory* Get();
	static void Destroy();

	//��� ������ ����� �������������� � ����� ���������� ��������:
	virtual CMethods* GetPMethods() const { return m_methods; };//�������� ������ �� ����� �������� ������� ���� ��������� � �������

	//���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	virtual CValue Method(const wxString &sName, CValue **aParams) { return CValue(); }
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //�������� ��������

	virtual wxString GetString() const { return wxT("enumFactory"); };
	virtual wxString GetTypeString() const { return wxT("enumFactory"); };

public:

	void AppendEnumeration(const wxString &enumName, CValue *newEnum);
	CValue *GetEnumeration(const wxString &enumName) const;
	void RemoveEnumeration(const wxString &enumName);

protected:

	CMethods *m_methods;
};

#endif 