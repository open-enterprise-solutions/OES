#ifndef _VALUETYPES_H_
#define _VALUETYPES_H_

#include "compiler/value.h"

class CValueTypes : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueTypes);

private:

	std::vector<wxClassInfo *> aClassInfo; 

private:

	static CMethods m_methods;

public:

	CValueTypes();

	wxString GetTypeString()const;
	wxString GetString()const;

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //��������� ��������
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //�������� ��������
	virtual int  FindAttribute(const wxString &sName);

	virtual CMethods* GetPMethods() { return &m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames();                          //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	virtual CValue Method(CMethodParameters &aParams);       //����� ������

	virtual ~CValueTypes();
};

#endif 