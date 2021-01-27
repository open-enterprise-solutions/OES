#ifndef _VALUECOLOUR_H__
#define _VALUECOLOUR_H__

#include "compiler/value.h"

//��������� ��������
class CValueColour : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueColour);

public:

	wxColour m_colour;

public:

	CValueColour();
	CValueColour(wxColour colour);
	
	virtual bool Init(CValue **aParams);

	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	//check is empty
	virtual inline bool IsEmpty() const override { return !m_colour.IsOk(); }

	static CMethods m_methods;

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //��������� ��������
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //�������� ��������

	virtual CMethods* GetPMethods() const { return &m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������

	operator wxColour() { return m_colour; }

	virtual ~CValueColour();
};

#endif