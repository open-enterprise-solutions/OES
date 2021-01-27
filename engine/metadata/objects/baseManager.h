#ifndef _MANAGER_BASE_H__
#define _MANAGER_BASE_H__

#include "compiler/value.h"

class CManagerBase : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CManagerBase);

public:

	CManagerBase();
	CManagerBase(IMetadata *metaData);

	virtual ~CManagerBase();

	virtual CMethods* GetPMethods() const { PrepareNames();  return m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������

	//attributes
	virtual CValue GetAttribute(CAttributeParameters &aParams);                   //�������� ��������

	//types 
	virtual wxString GetTypeString() const { return wxT("manager"); }
	virtual wxString GetString() const { return wxT("manager"); }

protected:

	//methos 
	CMethods *m_methods;

	//metadata 
	IMetadata *m_metaData;
};

#endif 