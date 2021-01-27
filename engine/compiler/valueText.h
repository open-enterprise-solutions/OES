#if !defined(_VALUE_TEXT_H__)
#define _VALUE_TEXT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "value.h"
#include "common/templates/module.h"

class CValueText : public CValue
{
	wxDECLARE_DYNAMIC_CLASS(CValueText);
public:
	CValueText();
	virtual ~CValueText();

	enum
	{
		enLinesCount = 0,
		enGetLine,
		enOpen,
		enTemplate,
		enFixTemplate,
		enInsertLine,
		enAddLine,
		enReplaceLine,
		enDeleteLine,
		enReadOnly,
		enShow,
		enClear,
		enCodePage,
		enWrite,
		enGetText,
	};

	wxString sTitle;
	wxArrayString aText;
	CModuleDocument* pDoc;
	BOOL bReadOnly;
	int nCodePage;
	int nTemplate;
	int nFixTemplate;
	wxString GetStr();

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
};

#endif // !defined(_VALUE_TEXT_H__)
