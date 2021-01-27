#ifndef _CONSTANTS_MANAGER_H__
#define _CONSTANTS_MANAGER_H__

#include "constants.h"

class CManagerConstants : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CManagerDocument);

protected:

	CConstantObject *m_metaConst;

public:

	CManagerConstants() {}
	CManagerConstants(IMetaObject *metaConst);

	//methos 
	static CMethods m_methods;

	virtual CMethods* GetPMethods() const { PrepareNames();  return &m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	virtual CValue Method(CMethodParameters &aParams);//����� ������

	//types 
	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	virtual ~CManagerConstants();
};


#endif // !_CONSTANTS_MANAGER_H__
