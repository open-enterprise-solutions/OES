#ifndef _VALUESIZE_H__
#define _VALUESIZE_H__

#include "compiler/value.h"

//��������� ��������
class CValueSize : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueSize);

public:

	wxSize m_size;

public:

	CValueSize();
	CValueSize(const wxSize &size);

	virtual bool Init(CValue **aParams);

	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	virtual inline bool IsEmpty() const override { return m_size == wxDefaultSize; }

	static CMethods m_methods;

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //��������� ��������
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //�������� ��������

	virtual CMethods* GetPMethods() const { return &m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������

	operator wxSize() { return m_size; }

	virtual ~CValueSize();

};

#endif