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

	//��� ������ ����� �������������� � ����� ���������� ��������:
	virtual CMethods* GetPMethods() const { return &m_methods; };//�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;//���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	virtual CValue Method(CMethodParameters &aParams);//����� ������

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