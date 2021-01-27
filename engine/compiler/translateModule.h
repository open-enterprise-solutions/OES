#ifndef _TRANSLATEMODULE_H__
#define _TRANSLATEMODULE_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <map>
#include <vector>

#include "value.h"
#include "functions.h"

//������ �������� ����
struct aKeyWordsDef {
	//char *Rus;
	char *Eng;
	char *sShortDescription;
};

extern struct aKeyWordsDef aKeyWords[];

enum
{
	LEXEM_ADD = 0,
	LEXEM_ADDDEF,
	LEXEM_IGNORE,
};

//�������� �������:
enum
{
	RETURN_NONE = 0,//��� �������� (��� ������)
	RETURN_PROCEDURE,//������� �� ���������
	RETURN_FUNCTION,//������� �� �������
};

//�������� ���������� (�������� � �����. ��������� � ������� nArray ����-����)
enum
{
	DEF_VAR_SKIP = -1,//����������� ��������
	DEF_VAR_DEFAULT = -2,//�������� �� ���������
	DEF_VAR_TEMP = -3,//������� ��������� ��������� ����������
	DEF_VAR_NORET = -7,//������� (���������) �� ���������� ��������
	DEF_VAR_CONST = 1000,//�������� ��������

};

//�����������

//�������� ������ ��������� �� ��������� ����
struct CLexem
{
	//��� �������:
	short       m_nType;

	//���������� �������:
	short	    m_nData;		//����� ���������� �����(KEYWORD) ��� ������ �����������(DELIMITER)
	CValue	    m_vData;		//��������, ���� ��� ��������� ��� �������� ��� ��������������
	wxString	m_sData;		//��� ��� �������������� (����������, ������� � ��.)

	//�������������� ����������: 
	wxString m_sModuleName;//��� ������ (�.�. �������� include ����������� �� ������ �������)
	unsigned int m_nNumberLine;	  //����� ������ ��������� ������ (��� ����� ��������)
	unsigned int m_nNumberString;	  //����� ��������� ������ (��� ������ ������)

	//�����������: 
	CLexem() : m_nType(0), m_nData(0), m_nNumberString(0), m_nNumberLine(0) {}
};

typedef std::vector<CLexem> CLexemList;

//����� ��� �������� ����������� ������������
class CDefList
{
public:

	CDefList() : pParent(NULL) {};
	~CDefList();

	std::map<wxString, CLexemList *> DefList;//�������� ������� ������
	CDefList *pParent;

	void SetParent(CDefList *p);
	void RemoveDef(const wxString &sName);
	bool HasDef(const wxString &sName);
	CLexemList *GetDef(const wxString &sName);
	void SetDef(const wxString &sName, CLexemList*);
	void SetDef(const wxString &sName, wxString sValue);
};

/************************************************
CTranslateModule-���� �������� ��������� ����
����� ����� - ��������� Load() � TranslateModule().
������ ��������� ��������� ������������� ���������� � ��������
������ ������������ ����, ������ ��������� ��������� ����������
(������� ����). � �������� ���������� � ��������� ������ �����������
������ "������" ����-���� � ���������� cByteCode.
*************************************************/

class CTranslateModule
{
	static CDefList glDefList;

public:

	CTranslateModule(const wxString &moduleName = wxEmptyString);
	virtual ~CTranslateModule();

	bool HasDef(const wxString &sName)
	{
		if (m_aDefList) {
			return m_aDefList->HasDef(sName);
		}
		return false;
	};

	//������:
	void Load(const wxString &sCode);
	void OnSetParent(CTranslateModule *pSetParent);

	void Clear();
	bool PrepareLexem();
	void SetError(int nErr, int nCurPos, const wxString &sError = wxEmptyString);

public:

	inline void SkipSpaces();

	bool IsByte(char c);
	char GetByte();

	bool IsWord();
	wxString GetWord(bool bOrigin = false, bool bGetPoint = false, wxString *psOrig = NULL);

	bool IsNumber();
	wxString GetNumber();

	bool IsString();
	wxString GetString();

	bool IsDate();
	wxString GetDate();

	bool IsEnd();

#if defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(_WIN64)
	static long long IsKeyWord(const wxString &sKeyWord);
#else
	static int IsKeyWord(const wxString &sKeyWord);
#endif

	wxString GetStrToEndLine();
	void PrepareFromCurrent(int nMode, const wxString &sName = wxEmptyString);
	wxString GetModuleName() { return m_sModuleName; }

	unsigned int GetCurrentPos() { return m_nCurPos; }
	unsigned int GetCurrentLine() { return m_nCurLine; }

public:

	static std::map<wxString, void *> m_aHashKeyWords;
	static void LoadKeyWords();

protected:

	//������ � ���������� ��� �������� ������
	//��������� "��������":
	CDefList *m_aDefList;
	bool m_bAutoDeleteDefList;
	int m_nModePreparing;

	//��������:
	wxString m_sModuleName;//��� �������������� ������ (��� ������ ���������� ��� �������)
	unsigned int m_nSizeText;//������ ��������� ������

	//�������� �����:
	wxString m_sBuffer;
	wxString m_sBUFFER;//����� - �������� ��� � ������� ��������

	unsigned int m_nCurPos;//������� ������� ��������������� ������
	unsigned int m_nCurLine;

	//������������� ������ � ���������:
	std::vector<CLexem> m_aLexemList;
};

#endif