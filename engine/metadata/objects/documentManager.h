#ifndef _MANAGER_DOCUMENT_H__
#define _MANAGER_DOCUMENT_H__

#include "document.h"
#include "common/managerInfo.h"

class CManagerDocument : public CValue,
	public IMetaManagerInfo
{
	wxDECLARE_DYNAMIC_CLASS(CManagerDocument);

private:

	CMetaObjectDocumentValue *m_metaObject;

public:

	virtual CMetaCommonModuleObject *GetModuleManager();

	CManagerDocument() : m_methods(NULL) {}
	CManagerDocument(IMetaObject *metaObject);

	CValue FindByNumber(CValue sCode);
	CValue EmptyRef();

	virtual CMethods* GetPMethods() const { PrepareNames(); return m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	virtual CValue Method(CMethodParameters &aParams);//����� ������

	//types 
	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	virtual ~CManagerDocument();

protected:

	//methos 
	CMethods *m_methods;
};

#endif 