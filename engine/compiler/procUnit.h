#ifndef _PROCUNIT_H__
#define _PROCUNIT_H__

#include "compileModule.h"

class CRunContext
{
public:

	CProcUnit *m_procUnit;
	CCompileContext *m_compileContext;

	unsigned int m_nStart;
	unsigned int m_nCurLine; //������� ����������� ������ ����-����

	unsigned int m_nVarCount;
	unsigned int m_nParamCount;

	CValue m_cLocVars[MAX_STATIC_VAR];
	CValue *m_pLocVars;
	CValue *m_cRefLocVars[MAX_STATIC_VAR];
	CValue **m_pRefLocVars;

	std::map<wxString, CProcUnit *> m_aEvalString;

public:

	CRunContext(int nLocal = wxNOT_FOUND);

	CByteCode *GetByteCode();

	void SetLocalCount(int nLocal);
	unsigned int GetLocalCount();

	void SetProcUnit(CProcUnit *procUnit) { m_procUnit = procUnit; }
	CProcUnit *GetProcUnit() { return m_procUnit; }

	~CRunContext();
};

class CRunContextSmall
{
public:

	CRunContextSmall(int nLocal = wxNOT_FOUND) {
		m_nVarCount = 0;
		m_nParamCount = 0;

		if (nLocal >= 0) {
			SetLocalCount(nLocal);
		}
	};

	~CRunContextSmall() {
		if (m_nVarCount > MAX_STATIC_VAR)
		{
			delete[]m_pLocVars;
			delete[]m_pRefLocVars;
		}
	}

	unsigned int m_nStart;
	unsigned int m_nParamCount;

	CValue m_cLocVars[MAX_STATIC_VAR];
	CValue *m_pLocVars;
	CValue *m_cRefLocVars[MAX_STATIC_VAR];
	CValue **m_pRefLocVars;

private:

	unsigned int m_nVarCount;

public:

	void SetLocalCount(int nLocal)
	{
		m_nVarCount = nLocal;

		if (m_nVarCount > MAX_STATIC_VAR)
		{
			m_pLocVars = new CValue[m_nVarCount];
			m_pRefLocVars = new CValue*[m_nVarCount];
		}
		else
		{
			m_pLocVars = m_cLocVars;
			m_pRefLocVars = m_cRefLocVars;
		}

		for (unsigned int i = 0; i < m_nVarCount; i++)
		{
			m_pRefLocVars[i] = &m_pLocVars[i];
		}
	};

	unsigned int GetLocalCount() { return m_nVarCount; }
};

class CProcUnit
{
	//��������:
	int m_nAutoDeleteParent;	//������� �������� ������������� ������

	CByteCode *m_pByteCode;
	CRunContext m_cCurContext;
	CValue	***m_pppArrayList;//��������� �� ������� ���������� ���������� (0 - ��������� ����������,1-���������� �������� ������,2 � ���� - ���������� ������������ �������)
	CProcUnit **m_ppArrayCode;//��������� �� ������� ����������� ������� (0-������� ������,1 � ���� - ������������ ������)

	std::vector <CProcUnit *> m_aParent;

	friend class CRunContext;
	friend class CRunContextSmall;

public:

	//������������/�����������
	CProcUnit();
	virtual ~CProcUnit();

	//������
	void Clear();
	void SetParent(CProcUnit *pSetParent);
	CProcUnit *GetParent(unsigned int nLevel = 0);
	unsigned int GetParentCount();

	CByteCode *GetByteCode() { return m_pByteCode; }

	CValue Execute(CByteCode &ByteCode, bool bRunModule = true);
	CValue Execute(CRunContext *pContext, int nDelta); //nDelta=true - ������� ���������� ���������� ������, ������� ���� � ����� ������� � ��������

	static CValue Evaluate(const wxString &sCode, CRunContext *pRunContext = NULL, bool bCompileBlock = false, bool *bError = NULL);
	bool CompileExpression(const wxString &sCode, CRunContext *pRunContext, CCompileModule &cModule, bool bCompileBlock);

	//����� ������������ ������� ������������ ������
	int FindExportFunction(const wxString &sName);
	int FindFunction(const wxString &sName0, bool bError = false, int bExportOnly = 0);

	CValue CallFunction(const wxString &sName);
	CValue CallFunction(const wxString &sName, CValue &vParam1);
	CValue CallFunction(const wxString &sName, CValue &vParam1, CValue &vParam2);
	CValue CallFunction(const wxString &sName, CValue &vParam1, CValue &vParam2, CValue &vParam3);

	CValue CallFunction(const wxString &sName, CValue **ppParams, unsigned int nParamCount);

	CValue CallFunction(unsigned int lptr, CValue &vParam1);
	CValue CallFunction(unsigned int lptr, CValue &vParam1, CValue &vParam2);
	CValue CallFunction(unsigned int nAddr, CValue **ppParams, unsigned int nReceiveParamCount = MAX_STATIC_VAR);

	CValue CallFunction(CMethodParameters &aParams);

	void SetAttribute(const wxString &sName, CValue &cVal);
	void SetAttribute(CAttributeParameters &aParams, CValue &cVal);//��������� ��������

	CValue GetAttribute(const wxString &sName);
	CValue GetAttribute(CAttributeParameters &aParams);//�������� ��������

	int FindAttribute(const wxString &sName) const;

	//run module 
	static CProcUnit *GetCurrentRunModule();
	static void ClearCurrentRunModule();

	//run context
	static void AddRunContext(CRunContext *runContext);
	static unsigned int GetCountRunContext();
	static CRunContext *GetPrevRunContext();
	static CRunContext *GetCurrentRunContext();
	static CRunContext *GetRunContext(unsigned int idx);
	static void BackRunContext();

	static CByteCode *GetCurrentByteCode();

	static void Raise();
};

#endif 