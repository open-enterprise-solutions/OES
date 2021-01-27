#ifndef _COMPILEMODULE_H__
#define _COMPILEMODULE_H__

#include "translateModule.h"

typedef std::vector<CValue> CDefValueList;
typedef std::vector<bool> CDefBoolList;
typedef std::vector<int> CDefIntList;

struct SParam
{
	wxLongLong_t m_nArray;
	wxLongLong_t m_nIndex;

	wxString m_sName;//��� ���������� 
	wxString m_sType;//��� ���������� � ����. ������� (� ������ ����� ���������)

	SParam() : m_nArray(0), m_nIndex(0) {}
};

struct CVariable
{
	bool m_bExport;
	bool m_bContext;
	bool m_bTempVar;

	unsigned int m_nNumber;

	wxString m_sName;       //��� ����������
	wxString m_sType;       //��� �������
	wxString m_sContextVar; //��� ����������� ����������
	wxString m_sRealName;   //��� ���������� ��������

	CVariable() : m_bExport(false), m_bContext(false), m_bTempVar(false), m_nNumber(0) {};
	CVariable(const wxString &sVariableName) : m_sName(sVariableName), m_bExport(false), m_bContext(false), m_bTempVar(false), m_nNumber(0) {};
};

struct CFunction;

//�������� ������ ���� ���������
struct CByte
{
	short m_nOper;                //��� ����������
	unsigned int m_nNumberString;	//����� ��������� ������ (��� ������ ������)
	unsigned int m_nNumberLine;	//����� ������ ��������� ������ (��� ����� ��������)

	//��������� ��� ����������:
	SParam m_param1;
	SParam m_param2;
	SParam m_param3;
	SParam m_param4; // - ������������ ��� �����������

	wxString m_sModuleName; //��� ������ (�.�. �������� include ����������� �� ������ �������)
	wxString m_sDocPath; // ���������� ���� � ��������� 
	wxString m_sFileName; // ���� � ����� (���� ������� ���������) 

	CByte() : m_param1(), m_param2(), m_param3(), m_param4(),
		m_nOper(0), m_nNumberString(0), m_nNumberLine(0) {}
};

class CCompileModule;

//����� ����-����
struct CByteCode
{
	//��������:
	bool m_bCompile;		//������� �������� ����������

	std::vector <CByte>	m_aCodeList;//����������� ��� ������
	std::vector <CValue> m_aConstList;//������ �������� ������

	std::map<wxString, unsigned int> m_aVarList; //������ ���������� ������
	std::map<wxString, unsigned int> m_aFuncList; //������ ������� ������

	std::map<wxString, unsigned int> m_aExportVarList; //������ ���������� ���������� ������
	std::map<wxString, unsigned int> m_aExportFuncList; //������ ���������� ������� ������

	unsigned int m_nVarCount;		//���������� ��������� ���������� � ������
	unsigned int m_nStartModule;	//������ ������� ������ ������

	wxString m_sModuleName;//��� ������������ ������, �������� ����������� ����-���
	wxString m_sDocPath; //���������� ���� � �����
	wxString m_sFileName; //���� � �������� ����� (���� �������� ���������)

	wxString m_sModule;	//��� ������������ ������

	CByteCode *m_pParent;	//������������ ����-��� (��� ��������)
	CCompileModule *m_pModule;

	//������ ������� � ����������� ����������
	std::vector<CValue *> m_aExternValues;

	CByteCode() { Reset(); };

	void SetModule(CCompileModule *pSetModule) { m_pModule = pSetModule; };

	void Reset()
	{
		m_nStartModule = 0;
		m_nVarCount = 0;

		m_sModuleName = wxEmptyString;
		m_sModule = wxEmptyString;

		m_bCompile = false;

		m_pParent = NULL;
		m_pModule = NULL;

		m_aCodeList.clear();
		m_aConstList.clear();

		m_aVarList.clear();
		m_aFuncList.clear();
		m_aExportVarList.clear();
		m_aExportFuncList.clear();

		m_aExternValues.clear();
	}
};

struct SLabel
{
	wxString m_sName;
	int		m_nLine;
	int		m_nError;
};

class CCompileModule;

//*******************************************************************
//*                �����: �������� ����������                       *
//*******************************************************************
struct CCompileContext
{
	CCompileModule *m_compileModule;
	CCompileContext *m_parentContext;//������������ ��������

	CFunction *m_functionContext;

	//����������
	std::map <wxString, CVariable> m_cVariables;

	int m_nTempVar;//����� ������� ��������� ����������
	int m_nFindLocalInParent;//������� ������ ���������� � �������� (�� ���� �������), � ��������� ������� � ��������� ������ ������ ���������� ����������)

	//������� � ���������
	std::map<wxString, CFunction *>	m_cFunctions;//������ ������������� ����������� �������

	short m_nReturn;//����� ��������� ��������� RETURN : RETURN_NONE,RETURN_PROCEDURE,RETURN_FUNCTION
	wxString m_sCurFuncName;//��� ������� ������������� ������� (��� ��������� �������� ������ ����������� �������)

	//�����
	//��������� ��������
	unsigned short m_nDoNumber;//����� ���������� �����

	std::map<unsigned short, CDefIntList *> aContinueList;//������ ���������� Continue
	std::map<unsigned short, CDefIntList *> aBreakList;//������ ���������� Break

	//��������� ������� �������� ��� ������ Continue � Break
	void StartDoList();
	void FinishDoList(CByteCode	&cByteCode, int nGotoContinue, int nGotoBreak);

	//�����
	std::map<wxString, unsigned int> m_cLabelsDef;	//����������
	std::vector <SLabel> m_cLabels;	//������ ������������� ��������� �� �����

	void DoLabels();

	void SetModule(CCompileModule *module) { m_compileModule = module; }
	void SetFunction(CFunction *function) { m_functionContext = function; }

	SParam AddVariable(const wxString &sName, const wxString &sType = wxEmptyString, bool bExport = false, bool bContext = false, bool bTempVar = false);
	SParam GetVariable(const wxString &sName, bool bFindInParent = true, bool bCheckError = false, bool bContext = false, bool bTempVar = false);

	bool FindVariable(const wxString &sName, wxString &sContextVariable = wxString(), bool bContext = false);
	bool FindFunction(const wxString &sName, wxString &sContextVariable = wxString(), bool bContext = false);

	CCompileContext(CCompileContext *hSetParent = NULL) : m_parentContext(hSetParent),
		m_nDoNumber(0), m_nReturn(0), m_nTempVar(0), m_nFindLocalInParent(1),
		m_compileModule(NULL), m_functionContext(NULL)
	{
	};

	~CCompileContext();
};

struct CParamVariable
{
	bool m_bByRef;

	wxString m_sName;//��� ����������
	wxString m_sType;//��� ��������

	SParam 	m_vData;//�������� �� ���������

	CParamVariable() : m_bByRef(false) {
		m_vData.m_nArray = -1;
		m_vData.m_nIndex = -1;
	};
};

//����������� �������
struct CFunction
{
	bool m_bExport;
	bool m_bContext;

	wxString m_sRealName; //��� �������
	wxString m_sName; //��� ������� � ������� ��������
	wxString m_sType;	 //��� (� ����. �������), ���� ��� �������������� �������

	CCompileContext *m_pContext;//������� ����������

	unsigned int m_nVarCount;//����� ��������� ����������
	unsigned int m_nStart;//��������� ������� � ������� ����-�����
	unsigned int m_nFinish;//�������� ������� � ������� ����-�����

	SParam m_realRetValue;//��� �������� ���������� ��� �������� ������

	//��� IntelliSense
	unsigned int m_nNumberLine;	//����� ������ ��������� ������ (��� ����� ��������)

	wxString m_sShortDescription;//�������� � ���� ��� ������ ����� ��������� ����� �������(���������)
	wxString m_sLongDescription;//�������� � ���� ���� ������� (�.�.� ��� ������ �����) ���� ����������� �� ����������� ������� (���������)

	wxString m_sContextVar; //��� ���������� ����������

	std::vector<CParamVariable> m_aParamList;

	CFunction(const wxString &sFuncName, CCompileContext *pSetContext = NULL) : m_sName(sFuncName), m_pContext(pSetContext),
		m_bExport(false), m_bContext(false), m_nVarCount(0), m_nStart(0), m_nFinish(0), m_nNumberLine(0)
	{
		m_realRetValue.m_nArray = 0;
		m_realRetValue.m_nIndex = 0;
	};

	~CFunction()
	{
		if (m_pContext) 
			delete m_pContext; //������� ����������� �������� (� ������ ������� ���� ������ ����� � ��������� ����������)
	};
};

struct CCallFunction
{
	wxString m_sName;//��� ���������� �������
	wxString m_sRealName;//��� ���������� �������

	SParam m_sRetValue;//���������� ���� ������ ������������ ��������� ���������� �������
	SParam m_sContextVal;//��������� �� ���������� ��������

	unsigned int m_nAddLine;//��������� � ������� ����-�����, ��� ���������� ����� (��� ������, ����� ���� �����, �� ������� ��� �� ���������)
	unsigned int m_nError;//��� ������ ��������� ��� �������

	unsigned int m_nNumberString;	//����� ��������� ������ (��� ������ ������)
	unsigned int m_nNumberLine;	//����� ������ ��������� ������ (��� ����� ��������)

	wxString m_sModuleName;//��� ������ (�.�. �������� include ����������� �� ������ �������)

	std::vector<SParam> m_aParamList;//������ ������������ ���������� (������ ���������, ���� �������� �� ������, �� �.�. (-1,-1))
};

class CMetaModuleObject;

//*******************************************************************
//*                         �����: ����������                       *
//*******************************************************************

class CCompileModule : public CTranslateModule
{
	friend class CProcUnit;

private:

	bool Recompile(); //�������������� �������� ������ �� ����-�������

public:

	CCompileModule();
	CCompileModule(CMetaModuleObject *moduleObject, bool commonModule = false);

	virtual ~CCompileModule();

	//�������� ������:
	void Reset();//����� ������ ��� ���������� ������������� �������

	void PrepareModuleData();

	void AddVariable(const wxString &sName, const CValue &value);//��������� ������� ����������
	void AddVariable(const wxString &sName, CValue *pValue);//��������� ������� ����������

	void AddContextVariable(const wxString &sName, const CValue &value);
	void AddContextVariable(const wxString &sName, CValue *pValue);

	void RemoveVariable(const wxString &sName);

	void SetParent(CCompileModule *pSetParent);//��������� ������������� ������ � ������������ ����. �����������
	CCompileModule *GetParent() const { return m_pParent; }

	bool Compile(); //���������� ������ �� ����-�������

	//��������:
	CCompileModule *m_pParent;//������������ ������ (�.�. �� ��������� � �������� ��������� ���� ����������� ������)

	bool m_bCommonModule; // true - only functions and export functions 

	//������� �������� ����������, ������� � �����
	CCompileContext	m_cContext;
	CCompileContext	*m_pContext;

	CByteCode m_cByteCode;        //�������� ������ ����-����� ��� ���������� ����������� �������

	int m_nCurrentCompile;		//������� ��������� � ������� ������

	bool m_bExpressionOnly;		//������ ���������� ��������� (��� ����� ������� �������)
	bool m_bNeedRecompile;

	//������������ ������� ����������
	std::map<wxString, CValue *> m_aExternValues;
	//������������ ����������� ����������
	std::map<wxString, CValue *> m_aContextValues;

protected:

	CMetaModuleObject *m_moduleObject;

	std::map<wxString, unsigned int> m_aHashConstList;
	std::vector <CCallFunction*> m_apCallFunctions;	//������ ������������� ������� �������� � �������

	int m_nLastNumberLine;

public:

	static void InitializeCompileModule();

	CCompileContext *GetContext()
	{
		m_cContext.SetModule(this);
		return &m_cContext;
	};

	CMetaModuleObject *GetModuleObject() { return m_moduleObject; }

	//������ ������ ������ ��� ����������:
	void SetError(int nErr, const wxString &sError = wxEmptyString);
	void SetError(int nErr, char c);

	SParam GetExpression(int nPriority = 0);

protected:

	CLexem PreviewGetLexem();
	CLexem GetLexem();
	CLexem GETLexem();
	void GETDelimeter(char c);

	bool IsDelimeter(char c);
	bool IsKeyWord(int nKey);

	bool IsNextDelimeter(char c);
	bool IsNextKeyWord(int nKey);

	void GETKeyWord(int nKey);

	wxString GETIdentifier(bool realName = false);
	CValue GETConstant();

	void AddLineInfo(CByte &code);

	bool CompileModule();

	bool CompileFunction();
	bool CompileDeclaration();

	bool CompileBlock();

	bool CompileNewObject();
	bool CompileGoto();
	bool CompileIf();
	bool CompileWhile();
	bool CompileFor();
	bool CompileForeach();

	SParam GetCallFunction(const wxString &sName);
	SParam GetCurrentIdentifier(int &nIsSet);

	SParam FindConst(CValue &vData);

	bool AddCallFunction(CCallFunction* pRealCall);
	CFunction *GetFunction(const wxString &sName, int *pNumber = NULL);

	bool IsTypeVar(const wxString &sVariable = wxEmptyString);
	wxString GetTypeVar(const wxString &sVariable = wxEmptyString);
	void AddTypeSet(const SParam &sVariable);

	int GetConstString(const wxString &sMethod);

protected:

	virtual SParam AddVariable(const wxString &sName, const wxString &sType = wxEmptyString, bool bExport = false, bool bContext = false, bool bTempVar = false);

	virtual SParam GetVariable();
	virtual SParam GetVariable(const wxString &sName, bool bCheckError = false, bool bLoadFromContext = false);
};

#endif 