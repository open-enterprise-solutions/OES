#ifndef _AUTOCOMPLETE_COMPILE_H__
#define _AUTOCOMPLETE_COMPILE_H__

#include "common/moduleInfo.h"
#include "compiler/compilemodule.h"

class CPrecompileModule;
struct CPrecompileFunction;

struct SParamValue
{
	wxString sName;//��� ���������� 
	wxString sType;//��� ���������� � ����. ������� (� ������ ����� ���������)

	CValue vObject;
};

struct CPrecompileVariable
{
	bool bExport;
	bool bContext;
	bool bTempVar;

	int nNumber;

	wxString sName;//��� ����������
	wxString sType;//��� ��������
	wxString sRealName;

	CValue vContext;
	CValue vObject;

	CPrecompileVariable() : bExport(false), bContext(false), bTempVar(false), nNumber(0) {};
	CPrecompileVariable(wxString csVarName) : sName(csVarName), bExport(false), bContext(false), bTempVar(false), nNumber(0) {};
};

struct CPrecompileContext
{
	CPrecompileModule *pModule;
	void SetModule(CPrecompileModule *pSetModule) { pModule = pSetModule; };

	CPrecompileContext *pParent;//������������ ��������
	CPrecompileContext *pStopParent;//������ ����������� ������� �����������
	CPrecompileContext *pContinueParent;//������ ����������� ������� �����������
	bool bStaticVariable;		//��� ���������� ���������

	//����������
	std::map <wxString, CPrecompileVariable> cVariables;

	SParamValue GetVariable(const wxString &sName, bool bFindInParent = true, bool bCheckError = false, CValue Value = CValue());
	SParamValue AddVariable(const wxString &sName, const wxString &sType = wxEmptyString, bool bExport = false, bool bTempVar = false, CValue Value = CValue());

	bool FindVariable(const wxString &sName, CValue &vContext = CValue(), bool bContext = false);
	bool FindFunction(const wxString &sName, CValue &vContext = CValue(), bool bContext = false);

	void RemoveVariable(const wxString &sName);

	int nTempVar;//����� ������� ��������� ����������
	int nFindLocalInParent;//������� ������ ���������� � �������� (�� ���� �������), � ��������� ������� � ��������� ������ ������ ���������� ����������)

	//������� � ���������
	std::map<wxString, CPrecompileFunction *> cFunctions;//������ ������������� ����������� �������
	int nReturn;//����� ��������� ��������� RETURN : RETURN_NONE,RETURN_PROCEDURE,RETURN_FUNCTION
	wxString sCurFuncName;//��� ������� ������������� ������� (��� ��������� �������� ������ ����������� �������)

	CPrecompileContext(CPrecompileContext *hSetParent = NULL)
	{
		pParent = hSetParent;

		nReturn = 0;
		nFindLocalInParent = 1;
		pModule = NULL;

		pStopParent = NULL;
		pContinueParent = NULL;

		if (hSetParent)
		{
			pStopParent = hSetParent->pStopParent;
			pContinueParent = hSetParent->pContinueParent;
		}

		nTempVar = 0;
		bStaticVariable = false;

	};

	~CPrecompileContext();
};

//����������� �������
struct CPrecompileFunction
{
	wxString sRealName;//��� �������
	wxString sName;//��� ������� � ������� ��������
	std::vector<SParamValue> aParamList;
	bool bExport;
	bool bContext;
	CPrecompileContext *m_pContext;//������� ����������
	int nVarCount;//����� ��������� ����������
	int nStart;//��������� ������� � ������� ����-�����
	int nFinish;//�������� ������� � ������� ����-�����

	CValue vContext;

	SParamValue RealRetValue;//��� �������� ���������� ��� �������� ������
	bool bSysFunction;
	wxString sType;		//��� (� ����. �������), ���� ��� �������������� �������

	//��� IntelliSense
	int	nNumberLine;	//����� ������ ��������� ������ (��� ����� ��������)
	wxString sShortDescription;//�������� � ���� ��� ������ ����� ��������� ����� �������(���������)
	wxString sLongDescription;//�������� � ���� ���� ������� (�.�.� ��� ������ �����) ���� ����������� �� ����������� ������� (���������)

	CPrecompileFunction(const wxString &sFuncName, CPrecompileContext *pSetContext = NULL)
	{
		sName = sFuncName;
		m_pContext = pSetContext;
		bExport = false;
		bContext = false;
		nVarCount = 0;
		nStart = 0;
		nFinish = 0;
		bSysFunction = false;
		nNumberLine = -1;
	};

	~CPrecompileFunction()
	{
		if (m_pContext)//������� ����������� �������� (� ������ ������� ���� ������ ����� � ��������� ����������)
			delete m_pContext;
	};
};

//*******************************************************************
//*                         �����: ���-����������                   *
//*******************************************************************
class CPrecompileModule : public CTranslateModule
{
	int m_nCurrentCompile;		//������� ��������� � ������� ������

	IModuleInfo *m_contextVariable;
	CMetaModuleObject *m_moduleObject;

	std::map<wxString, unsigned int> m_aHashConstList;

	CPrecompileContext	cContext;
	CPrecompileContext	*m_pContext;
	CPrecompileContext  *m_pCurrentContext;

	CValue vObject;

	unsigned int nLastPosition;

	wxString sLastExpression;
	wxString sLastKeyword;
	wxString sLastParentKeyword;

	bool m_bCalcValue;

	unsigned int m_nCurrentPos;

	friend class CAutocomplectionCtrl;

public:

	//�������� ������:
	void Clear();//����� ������ ��� ���������� ������������� �������
	void PrepareModuleData();

	CPrecompileModule(CMetaModuleObject *moduleObject);
	virtual ~CPrecompileModule();

	CValue GetComputeValue() { return vObject; }
	
	CPrecompileContext *GetContext()
	{
		cContext.SetModule(this);
		return &cContext;
	};

	CPrecompileContext *GetCurrentContext() const { return m_pCurrentContext; }

	bool Compile();

	bool PrepareLexem(); 
	void PatchLexem(unsigned int line, int offsetLine, unsigned int offsetString, unsigned int modFlags);

protected:

	bool CompileFunction();
	bool CompileDeclaration();

	bool CompileBlock();

	bool CompileNewObject();
	bool CompileGoto();
	bool CompileIf();
	bool CompileWhile();
	bool CompileFor();
	bool CompileForeach();

protected:

	bool CompileModule();

	CLexem PreviewGetLexem();
	CLexem GetLexem();
	CLexem GETLexem();
	void GETDelimeter(char c);

	bool IsNextDelimeter(char c);
	bool IsNextKeyWord(int nKey);
	void GETKeyWord(int nKey);
	wxString GETIdentifier(bool realName = false);
	CValue GETConstant();
	int GetConstString(const wxString &sMethod);

	int IsTypeVar(const wxString &sType = wxEmptyString);
	wxString GetTypeVar(const wxString &sType = wxEmptyString);

	SParamValue GetExpression(int nPriority = 0);

	SParamValue GetCurrentIdentifier(int &nIsSet);
	SParamValue GetCallFunction(const wxString &sName);

	void AddVariable(const wxString &sName, CValue Value);

	SParamValue GetVariable(const wxString &sName, bool bCheckError = false);
	SParamValue GetVariable();

	SParamValue FindConst(CValue &vData);
};

#endif 