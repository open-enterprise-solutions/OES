#ifndef _VALUEQUERY_H__
#define _VALUEQUERY_H__

#include "value.h"

//��������� ��������
class CValueGrid : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueGrid);

public:

	CValueGrid();
	virtual ~CValueGrid();

	virtual wxString GetTypeString()const { return wxT("table.document"); }
	virtual wxString GetString()const { return wxT("table.document"); }

	static CMethods m_methods;

	virtual void SetAttribute(CAttributeParameters &aParams, CValue &cVal);        //��������� ��������
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //�������� ��������

	virtual CMethods* GetPMethods() const { return &m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	virtual CValue Method(CMethodParameters &aParams);       //����� ������

protected:

	bool ShowGrid(const wxString &sTitle);
};

#endif