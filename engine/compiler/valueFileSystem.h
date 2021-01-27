#ifndef _VALUEFILESYSYEM_H__
#define _VALUEFILESYSYEM_H__

#include "value.h"

class CValueFileSystem : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueFileSystem);
public:
	CValueFileSystem();
	virtual ~CValueFileSystem();
	int nTimer;
	HANDLE hFindFile;
	std::map<wxString, void *> aListHandle;


	//������ ��� ����������� �������
	wxString sObjectName;
	static CMethods m_methods;
	//��� ������ ����� �������������� � ����� ���������� ��������:
	CMethods* GetPMethods() const { return &m_methods; };//�������� ������ �� ����� �������� ������� ���� ��������� � �������
	void PrepareNames() const;//���� ����� ������������� ���������� ��� ������������� ���� ��������� � �������
	CValue Method(CMethodParameters &aParams);//����� ������

	wxString GetString()const { return sObjectName; };
	bool Init(wxString StrVid, CValue Param) { sObjectName = StrVid; };
	wxString GetTypeString()const { return sObjectName; };

	void CloseAllHandle();
};

#endif