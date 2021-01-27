#ifndef _VALUEFONT_H__
#define _VALUEFONT_H__

#include "compiler/value.h"

//��������� ��������
class CValueFont : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueFont);

public:

	wxFont m_font;

public:

	CValueFont();
	CValueFont(const wxFont &font);

	virtual bool Init(CValue **aParams);

	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	//check is empty
	virtual inline bool IsEmpty() const override { return !m_font.IsOk(); }

	static CMethods m_methods;

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cValue);        //��������� ��������
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //�������� ��������

	virtual CMethods* GetPMethods() const { return &m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������

	operator wxFont() { return m_font; }

	virtual ~CValueFont();
};

#endif