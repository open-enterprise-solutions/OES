#ifndef _MANAGER_CATALOG_H__
#define _MANAGER_CATALOG_H__

#include "catalog.h"
#include "common/managerInfo.h"

class CManagerCatalog : public CValue,
	public IMetaManagerInfo
{
	wxDECLARE_DYNAMIC_CLASS(CManagerCatalog);

private:

	CMetaObjectCatalogValue *m_metaObject;

public:

	virtual CMetaCommonModuleObject *GetModuleManager();

	CManagerCatalog() : m_methods(NULL) {}
	CManagerCatalog(IMetaObject *metaObject);

	CValue FindByCode(CValue sCode);
	CValue FindByName(CValue sName);

	CValue EmptyRef();

	virtual CMethods* GetPMethods() const { PrepareNames();  return m_methods; }; //�������� ������ �� ����� �������� ������� ���� ��������� � �������
	virtual void PrepareNames() const;                         //���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	virtual CValue Method(CMethodParameters &aParams);//����� ������

	//types 
	virtual wxString GetTypeString() const;
	virtual wxString GetString() const;

	virtual ~CManagerCatalog();

protected:

	//methos 
	CMethods *m_methods;
};

#endif 