////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, 2�-team
//	Description : compile module 
////////////////////////////////////////////////////////////////////////////

#include "compileModule.h"
#include "definition.h"
#include "systemObjects.h"
#include "metadata/metaObjects/metaModuleObject.h"
#include "utils/stringutils.h"

#pragma warning(push)
#pragma warning(disable : 4018)

//////////////////////////////////////////////////////////////////////
//                           Constants
//////////////////////////////////////////////////////////////////////

std::map<wxString, void *> aHelpDescription;//�������� �������� ���� � ��������� �������
std::map<wxString, void *> aHashKeywordList;

//������ ����������� �������������� ��������
static int aPriority[256];

//////////////////////////////////////////////////////////////////////
// CCompileContext CCompileContext CCompileContext CCompileContext  //
//////////////////////////////////////////////////////////////////////

/**
 * ��������� ����� ���������� � ������
 * ���������� ����������� ���������� � ���� SParam
 */
SParam CCompileContext::AddVariable(const wxString &m_sName, const wxString &m_sType, bool m_bExport, bool m_bContext, bool m_bTempVar)
{
	if (FindVariable(m_sName))//���� ���������� + ��������� ���������� = ������
		m_compileModule->SetError(ERROR_IDENTIFIER_DUPLICATE, m_sName);

	unsigned int nCountVar = m_cVariables.size();

	CVariable cCurrentVariable;
	cCurrentVariable.m_sName = StringUtils::MakeUpper(m_sName);
	cCurrentVariable.m_sRealName = m_sName;
	cCurrentVariable.m_bExport = m_bExport;
	cCurrentVariable.m_bContext = m_bContext;
	cCurrentVariable.m_bTempVar = m_bTempVar;

	cCurrentVariable.m_sType = m_sType;
	cCurrentVariable.m_nNumber = nCountVar;

	m_cVariables[StringUtils::MakeUpper(m_sName)] = cCurrentVariable;

	SParam cRet;
	cRet.m_sType = m_sType;
	cRet.m_nArray = 0;
	cRet.m_nIndex = nCountVar;

	return cRet;
}

/**
 * ������� ���������� ����� ���������� �� ���������� �����
 * ����� ����������� ����������, ������� � �������� ��������� �� ���� ������������
 * ���� ��������� ���������� ���, �� ��������� ����� ����������� ����������
 */
SParam CCompileContext::GetVariable(const wxString &m_sName, bool bFindInParent, bool bCheckError, bool m_bContext, bool m_bTempVar)
{
	int nCanUseLocalInParent = m_nFindLocalInParent;
	SParam Variable;

	if (!FindVariable(m_sName))
	{
		if (bFindInParent)//���� � ������������ ����������(�������)
		{
			int nParentNumber = 0;

			CCompileContext *pCurContext = m_parentContext;

			while (pCurContext)
			{
				nParentNumber++;

				if (nParentNumber > MAX_OBJECTS_LEVEL)
				{
					CSystemObjects::Message(pCurContext->m_compileModule->GetModuleName());
					if (nParentNumber > 2 * MAX_OBJECTS_LEVEL) CTranslateError::Error(_("Recursive call of modules!"));
				}

				if (pCurContext->FindVariable(m_sName))//�����
				{
					CVariable cCurrentVariable = pCurContext->m_cVariables[StringUtils::MakeUpper(m_sName)];
					//������� ��� ���������� ���������� ��� ��� (���� m_nFindLocalInParent=true, �� ����� ����� ��������� ���������� ��������)
					if (nCanUseLocalInParent > 0 || cCurrentVariable.m_bExport)
					{
						//���������� ����� ����������
						Variable.m_nArray = nParentNumber;
						Variable.m_nIndex = cCurrentVariable.m_nNumber;
						Variable.m_sType = cCurrentVariable.m_sType;
						return Variable;
					}
				}

				nCanUseLocalInParent--;
				pCurContext = pCurContext->m_parentContext;
			}
		}

		if (bCheckError) m_compileModule->SetError(ERROR_VAR_NOT_FOUND, m_sName); //������� ��������� �� ������

		//�� ���� ��� ���������� ���������� - ���������
		AddVariable(m_sName, wxEmptyString, m_bContext, m_bContext, m_bTempVar);
	}

	//���������� ����� � ��� ����������
	CVariable cCurrentVariable = m_cVariables[StringUtils::MakeUpper(m_sName)];

	Variable.m_nArray = 0;
	Variable.m_nIndex = cCurrentVariable.m_nNumber;
	Variable.m_sType = cCurrentVariable.m_sType;

	return Variable;
}

/**
 * ����� ���������� � ��� �������
 * ���������� 1 - ���� ���������� �������
 */
bool CCompileContext::FindVariable(const wxString &m_sName, wxString &sContextVariable, bool m_bContext)
{
	if (m_bContext)
	{
		auto itFounded = m_cVariables.find(StringUtils::MakeUpper(m_sName));
		if (itFounded != m_cVariables.end())
		{
			sContextVariable = StringUtils::MakeUpper(itFounded->second.m_sContextVar);
			return itFounded->second.m_bContext;
		}

		if (m_parentContext && m_parentContext->FindVariable(m_sName, sContextVariable, m_bContext))
			return true;

		sContextVariable = wxEmptyString;
		return false;
	}
	else
	{
		return m_cVariables.find(StringUtils::MakeUpper(m_sName)) != m_cVariables.end();
	}
}

/**
 * ����� ���������� � ��� �������
 * ���������� 1 - ���� ���������� �������
 */
bool CCompileContext::FindFunction(const wxString &m_sName, wxString &sContextVariable, bool m_bContext)
{
	if (m_bContext)
	{
		auto itFounded = m_cFunctions.find(StringUtils::MakeUpper(m_sName));
		if (itFounded != m_cFunctions.end() && itFounded->second)
		{
			sContextVariable = StringUtils::MakeUpper(itFounded->second->m_sContextVar);
			return itFounded->second->m_bContext;
		}

		if (m_parentContext && m_parentContext->FindFunction(m_sName, sContextVariable, m_bContext))
			return true;

		sContextVariable = wxEmptyString;
		return false;
	}
	else
	{
		return m_cFunctions.find(StringUtils::MakeUpper(m_sName)) != m_cFunctions.end();
	}
}

/**
 * ����������� ���������� GOTO � �������
 */
void CCompileContext::DoLabels()
{
	wxASSERT(m_compileModule);

	for (unsigned int i = 0; i < m_cLabels.size(); i++)
	{
		wxString m_sName = m_cLabels[i].m_sName;
		int m_nLine = m_cLabels[i].m_nLine;

		//���� ����� ����� � ������ ����������� �����
		unsigned int hLine = m_cLabelsDef[m_sName];

		if (!hLine)
		{
			m_compileModule->m_nCurrentCompile = m_cLabels[i].m_nError;
			m_compileModule->SetError(ERROR_LABEL_DEFINE, m_sName);//��������� ������������� ����������� �����
		}

		//���������� ����� ��������:
		m_compileModule->m_cByteCode.m_aCodeList[m_nLine].m_param1.m_nIndex = hLine + 1;
	}
};

/**
 * �������� ������ ��� ������ ����� ����-����, � ������� ����������� ������� Continue � Break
 */
void CCompileContext::StartDoList()
{
	//������� ������ ��� ������ Continue � Break (� ��� ����� �������� ������ ���� �����, ��� ����������� ��������������� �������)
	m_nDoNumber++;
	aContinueList[m_nDoNumber] = new CDefIntList();
	aBreakList[m_nDoNumber] = new CDefIntList();
};

/**
 * ��������� ������� �������� ��� ������ Continue � Break
 */
void CCompileContext::FinishDoList(CByteCode &m_cByteCode, int nGotoContinue, int nGotoBreak)
{
	CDefIntList *pListC = (CDefIntList *)aContinueList[m_nDoNumber];
	CDefIntList *pListB = (CDefIntList *)aBreakList[m_nDoNumber];
	if (pListC == 0 || pListB == 0)
	{
#ifdef _DEBUG 
		wxLogDebug("Error (FinishDoList) nGotoContinue=%d, nGotoBreak=%d\n", nGotoContinue, nGotoBreak);
		wxLogDebug("m_nDoNumber=%d\n", m_nDoNumber);
#endif 
		m_nDoNumber--;
		return;
	}

	for (unsigned int i = 0; i < pListC->size(); i++)
		m_cByteCode.m_aCodeList[*pListC[i].data()].m_param1.m_nIndex = nGotoContinue;

	for (unsigned int i = 0; i < pListB->size(); i++)
		m_cByteCode.m_aCodeList[*pListB[i].data()].m_param1.m_nIndex = nGotoBreak;

	aContinueList.erase(m_nDoNumber);
	aContinueList.erase(m_nDoNumber);

	delete pListC;
	delete pListB;

	m_nDoNumber--;
};

CCompileContext::~CCompileContext()
{
	for (auto it = m_cFunctions.begin(); it != m_cFunctions.end(); it++)
	{
		CFunction *pFunction = static_cast<CFunction *>(it->second);
		if (pFunction) delete pFunction;
	}
	
	m_cFunctions.clear();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction CCompileModule
//////////////////////////////////////////////////////////////////////

CCompileModule::CCompileModule() : CTranslateModule(_("module")),
m_pContext(GetContext()),
m_moduleObject(NULL), m_pParent(NULL),
m_bExpressionOnly(false), m_bNeedRecompile(false),
m_bCommonModule(false)
{
	m_cContext.m_nFindLocalInParent = 0; //� ������������ ���������� ��������� ���������� �� ����!
	InitializeCompileModule();
}

CCompileModule::CCompileModule(CMetaModuleObject *moduleObject, bool commonModule) : CTranslateModule(moduleObject->GetFullName()),
m_pContext(GetContext()),
m_moduleObject(moduleObject), m_pParent(NULL),
m_bExpressionOnly(false), m_bNeedRecompile(false),
m_bCommonModule(commonModule)
{
	m_cContext.m_nFindLocalInParent = 0; //� ������������ ���������� ��������� ���������� �� ����!
	InitializeCompileModule();
}

CCompileModule::~CCompileModule()
{
	Reset();

	m_aExternValues.clear();
	m_aContextValues.clear();
}

static bool m_initModule = false;

void CCompileModule::InitializeCompileModule()
{
	if (m_initModule) return;

	ZeroMemory(aPriority, sizeof(aPriority));

	aPriority['+'] = 10;
	aPriority['-'] = 10;
	aPriority['*'] = 30;
	aPriority['/'] = 30;
	aPriority['%'] = 30;
	aPriority['!'] = 50;

	aPriority[KEY_OR] = 1;
	aPriority[KEY_AND] = 2;

	aPriority['>'] = 3;
	aPriority['<'] = 3;
	aPriority['='] = 3;

	m_initModule = true;
}

void CCompileModule::Reset()
{
	m_pContext = NULL;

	m_cContext.m_nDoNumber = 0;
	m_cContext.m_nReturn = 0;
	m_cContext.m_nTempVar = 0;
	m_cContext.m_nFindLocalInParent = 1;

	m_cContext.aContinueList.clear();
	m_cContext.aBreakList.clear();

	m_cContext.m_cLabels.clear();
	m_cContext.m_cLabelsDef.clear();

	//clear functions & variables 
	for (auto function : m_cContext.m_cFunctions) {
		CFunction *pFunction = static_cast<CFunction *>(function.second);
		if (pFunction) delete pFunction;
	}

	m_cContext.m_cVariables.clear();
	m_cContext.m_cFunctions.clear();

	for (unsigned int i = 0; i < m_apCallFunctions.size(); i++) {
		delete m_apCallFunctions[i];
	}

	m_apCallFunctions.clear();
}

void CCompileModule::PrepareModuleData()
{
	for (auto externValue : m_aExternValues)
	{
		m_cContext.AddVariable(externValue.first, wxEmptyString, true);
		m_cByteCode.m_aExternValues.push_back(externValue.second);
	}

	for (auto contextValue : m_aContextValues)
	{
		m_cContext.AddVariable(contextValue.first, wxEmptyString, true);
		m_cByteCode.m_aExternValues.push_back(contextValue.second);
	}

	for (auto contextValue : m_aContextValues)
	{
		//��������� ���������� �� ���������
		for (unsigned int i = 0; i < contextValue.second->GetNAttributes(); i++)
		{
			wxString sAttributeName = contextValue.second->GetAttributeName(i);

			//���������� ����� � ��� ����������
			CVariable m_cVariables(sAttributeName);
			m_cVariables.m_sContextVar = contextValue.first;

			m_cVariables.m_bContext = true;
			m_cVariables.m_bExport = true;
			m_cVariables.m_nNumber = i;

			GetContext()->m_cVariables[StringUtils::MakeUpper(sAttributeName)] = m_cVariables;
		}

		//��������� ������ �� ���������
		for (unsigned int i = 0; i < contextValue.second->GetNMethods(); i++)
		{
			wxString sMethodName = contextValue.second->GetMethodName(i);

			//���������� ����� � ��� �������
			CFunction *pFunction = new CFunction(sMethodName);
			pFunction->m_nStart = i;
			pFunction->m_bContext = true;
			pFunction->m_bExport = true;

			pFunction->m_sContextVar = contextValue.first;

			//�������� �� ����������������
			GetContext()->m_cFunctions[StringUtils::MakeUpper(sMethodName)] = pFunction;
		}
	}
}

/**
 * SetError
 * ����������:
 * ��������� ������ ���������� � ������� ����������
 * ������������ ��������:
 * ����� �� ���������� ����������!
 */
void CCompileModule::SetError(int nErr, const wxString &sError)
{
	int m_nNumberString = 0;

	if (m_nCurrentCompile >= m_aLexemList.size()) m_nCurrentCompile = m_aLexemList.size() - 1;

	if (m_nCurrentCompile > 0)
	{
		if (m_nCurrentCompile < m_aLexemList.size()) m_nNumberString = m_aLexemList.at(m_nCurrentCompile - 1).m_nNumberString;
	}
	else
	{
		if (m_nCurrentCompile < m_aLexemList.size()) m_nNumberString = m_aLexemList.at(m_nCurrentCompile).m_nNumberString;
	}

	CTranslateModule::SetError(nErr, m_nNumberString, sError);
}

/**
 * ������ ������� �������
 */
void CCompileModule::SetError(int nErr, char c)
{
	SetError(nErr, wxString::Format(wxT("%c"), c));
}

//////////////////////////////////////////////////////////////////////
// Compiling
//////////////////////////////////////////////////////////////////////

/**
 *���������� � ���� ��� ���������� � ������� ������
 */
void CCompileModule::AddLineInfo(CByte &code)
{
	if (m_nCurrentCompile >= 0)
	{
		if (m_nCurrentCompile < m_aLexemList.size())
		{
			code.m_nNumberString = m_aLexemList[m_nCurrentCompile].m_nNumberString;
			code.m_nNumberLine = m_aLexemList[m_nCurrentCompile].m_nNumberLine;
		}
	}

	code.m_sModuleName = m_cByteCode.m_sModuleName;
	code.m_sDocPath = m_cByteCode.m_sDocPath;
	code.m_sFileName = m_cByteCode.m_sFileName;
}

/**
 * GetLexem
 * ����������:
 * �������� ��������� ������� �� ������ ���� ���� � �������� ������� ������� ������� �� 1
 * ������������ ��������:
 * 0 ��� ��������� �� �������
 */
CLexem CCompileModule::GetLexem()
{
	CLexem lex;
	if (m_nCurrentCompile + 1 < m_aLexemList.size())
	{
		lex = m_aLexemList[++m_nCurrentCompile];
	}
	return lex;
}

//�������� ��������� ������� �� ������ ���� ���� ��� ���������� �������� ������� �������
CLexem CCompileModule::PreviewGetLexem()
{
	CLexem lex;
	while (true)
	{
		lex = GetLexem();
		if (!(lex.m_nType == DELIMITER && lex.m_nData == ';')) break;
	}
	m_nCurrentCompile--;
	return lex;
}

/**
 * GETLexem
 * ����������:
 * �������� ��������� ������� �� ������ ���� ���� � �������� ������� ������� ������� �� 1
 * ������������ ��������:
 * ��� (� ������ ������� ��������� ����������)
 */
CLexem CCompileModule::GETLexem()
{
	CLexem lex = GetLexem();
	if (lex.m_nType == ERRORTYPE)
	{
		SetError(ERROR_CODE_DEFINE);
	}
	return lex;
}
/**
 * GETDelimeter
 * ����������:
 * �������� ��������� ������� ��� �������� �����������
 * ������������ ��������:
 * ��� (� ������ ������� ��������� ����������)
 */
void CCompileModule::GETDelimeter(char c)
{
	CLexem lex = GETLexem();
	if (!(lex.m_nType == DELIMITER && lex.m_nData == c))
	{
		SetError(ERROR_DELIMETER, c);
	}
}

/**
 * IsKeyWord
 * ����������:
 * ��������� �������� �� ������� ������� ����-���� �������� �������� ������
 * ������������ ��������:
 * true, false
 */
bool CCompileModule::IsKeyWord(int nKey)
{
	if (m_nCurrentCompile + 1 < m_aLexemList.size())
	{
		CLexem lex = m_aLexemList[m_nCurrentCompile];
		if (lex.m_nType == KEYWORD && lex.m_nData == nKey) return true;
	}
	return false;
}

/**
 * IsNextKeyWord
 * ����������:
 * ��������� �������� �� ��������� ������� ����-���� �������� �������� ������
 * ������������ ��������:
 * true,false
 */
bool CCompileModule::IsNextKeyWord(int nKey)
{
	if (m_nCurrentCompile + 1 < m_aLexemList.size())
	{
		CLexem lex = m_aLexemList[m_nCurrentCompile + 1];
		if (lex.m_nType == KEYWORD && lex.m_nData == nKey) return true;
	}
	return false;
}

/**
 * IsDelimeter
 * ����������:
 * ��������� �������� �� ������� ������� ����-���� �������� ������������
 * ������������ ��������:
 * true,false
 */
bool CCompileModule::IsDelimeter(char c)
{
	if (m_nCurrentCompile + 1 < m_aLexemList.size())
	{
		CLexem lex = m_aLexemList[m_nCurrentCompile];
		if (lex.m_nType == DELIMITER && lex.m_nData == c) return true;
	}
	return false;
}

/**
 * IsNextDelimeter
 * ����������:
 * ��������� �������� �� ��������� ������� ����-���� �������� ������������
 * ������������ ��������:
 * true,false
 */
bool CCompileModule::IsNextDelimeter(char c)
{
	if (m_nCurrentCompile + 1 < m_aLexemList.size())
	{
		CLexem lex = m_aLexemList[m_nCurrentCompile + 1];
		if (lex.m_nType == DELIMITER && lex.m_nData == c) return true;
	}
	return false;
}

/**
 * GETKeyWord
 * �������� ��������� ������� ��� �������� �������� �����
 * ������������ ��������:
 * ��� (� ������ ������� ��������� ����������)
 */
void CCompileModule::GETKeyWord(int nKey)
{
	CLexem lex = GETLexem();
	if (!(lex.m_nType == KEYWORD && lex.m_nData == nKey))
	{
		SetError(ERROR_KEYWORD, wxString::Format(wxT("%s"), aKeyWords[nKey].Eng));
	}
}

/**
 * GETIdentifier
 * �������� ��������� ������� ��� �������� �������� �����
 * ������������ ��������:
 * ������-�������������
 */
wxString CCompileModule::GETIdentifier(bool realName)
{
	CLexem lex = GETLexem();

	if (lex.m_nType != IDENTIFIER)
	{
		if (realName && lex.m_nType == KEYWORD) return lex.m_sData;
		SetError(ERROR_IDENTIFIER_DEFINE);
	}

	if (realName) return lex.m_vData.m_sData;
	else return lex.m_sData;
}

/**
 * GETConstant
 * �������� ��������� ������� ��� ���������
 * ������������ ��������:
 * ���������
 */
CValue CCompileModule::GETConstant()
{
	CLexem lex;
	int iNumRequire = 0;
	if (IsNextDelimeter('-') || IsNextDelimeter('+'))
	{
		iNumRequire = 1;
		if (IsNextDelimeter('-'))
			iNumRequire = -1;
		lex = GETLexem();
	}

	lex = GETLexem();

	if (lex.m_nType != CONSTANT) SetError(ERROR_CONST_DEFINE);

	if (iNumRequire)
	{
		//�������� �� �� ����� ��������� ����� �������� ���
		if (lex.m_vData.GetType() != eValueTypes::TYPE_NUMBER)
			SetError(ERROR_CONST_DEFINE);

		//������ ���� ��� ������
		if (iNumRequire == -1) lex.m_vData.m_fData = -lex.m_vData.m_fData;
	}
	return lex.m_vData;
}

//��������� ������ ���������� ������ (��� ����������� ������ ������)
int CCompileModule::GetConstString(const wxString &sMethod)
{
	if (!m_aHashConstList[sMethod])
	{
		m_cByteCode.m_aConstList.push_back(sMethod);
		m_aHashConstList[sMethod] = m_cByteCode.m_aConstList.size();
	}

	return m_aHashConstList[sMethod] - 1;
}

/**
 * AddVariable
 * ����������:
 * �������� ��� � ����� ������� ���������� � ����������� ������ ��� ����������� �������������
 */
void CCompileModule::AddVariable(const wxString &nameVariable, const CValue &vObject)
{
	if (nameVariable.IsEmpty())
		return;

	//��������� ������� ���������� ��� ����������
	m_aExternValues[nameVariable.Upper()] = vObject.m_typeClass == eValueTypes::TYPE_REFFER ? vObject.GetRef() : const_cast<CValue *>(&vObject);

	//������ ���� ��� ��������������
	m_bNeedRecompile = true;
}

/**
 * AddVariable
 * ����������:
 * �������� ��� � ����� ������� ��������� � ����������� ������ ��� ����������� �������������
 */
void CCompileModule::AddVariable(const wxString &nameVariable, CValue *pValue)
{
	if (nameVariable.IsEmpty())
		return;

	//��������� ������� ���������� ��� ����������
	m_aExternValues[nameVariable.Upper()] = pValue;

	//������ ���� ��� ��������������
	m_bNeedRecompile = true;
}

/**
 * AddContextVariable
 * ����������:
 * �������� ��� � ����� ������� ��������� � ����������� ������ ��� ����������� �������������
 */
void CCompileModule::AddContextVariable(const wxString &nameVariable, const CValue &vObject)
{
	if (nameVariable.IsEmpty())
		return;

	//��������� ���������� �� ���������
	m_aContextValues[nameVariable.Upper()] = vObject.m_typeClass == eValueTypes::TYPE_REFFER ? vObject.GetRef() : const_cast<CValue *>(&vObject);

	//������ ���� ��� ��������������
	m_bNeedRecompile = true;
}

/**
 * AddContextVariable
 * ����������:
 * �������� ��� � ����� ������� ��������� � ����������� ������ ��� ����������� �������������
 */
void CCompileModule::AddContextVariable(const wxString &nameVariable, CValue *pValue)
{
	if (nameVariable.IsEmpty())
		return;

	//��������� ���������� �� ���������
	m_aContextValues[nameVariable.Upper()] = pValue;

	//������ ���� ��� ��������������
	m_bNeedRecompile = true;
}

/**
 * RemoveVariable
 * ����������:
 * ������� ��� � ����� ������� ���������
 */
void CCompileModule::RemoveVariable(const wxString &nameVariable)
{
	if (nameVariable.IsEmpty())
		return;

	m_aExternValues.erase(nameVariable.Upper());
	m_aContextValues.erase(nameVariable.Upper());

	//������ ���� ��� ��������������
	m_bNeedRecompile = true;
}

/**
 * Recompile
 * ����������:
 * ��������� � �������������� �������� ��������� ���� � ����-��� (��������� ���)
 * ������������ ��������:
 * true,false
 */
bool CCompileModule::Recompile()
{
	wxASSERT(m_moduleObject);

	//clear functions & variables 
	Reset();

	//�������� ������ ������
	m_pContext = GetContext();

	m_cByteCode.m_sModuleName = m_moduleObject->GetFullName();
	m_cByteCode.m_sDocPath = m_moduleObject->GetDocPath();
	m_cByteCode.m_sFileName = m_moduleObject->GetFileName();

	m_cByteCode.m_sModule = m_moduleObject->GetModuleText();

	if (m_pParent)
	{
		m_cByteCode.m_pParent = &m_pParent->m_cByteCode;
		m_cContext.m_parentContext = &m_pParent->m_cContext;
	}

	Load(m_moduleObject->GetModuleText());

	//prepare lexem 
	if (!PrepareLexem()) {
		return false;
	}

	//����������� ����������� ���������� 
	PrepareModuleData();

	//���������� 
	if (CompileModule()) {
		m_bNeedRecompile = false; return true;
	}

	m_bNeedRecompile = true;
	return false;
}

/**
 * Compile
 * ����������:
 * ��������� � ���������� ��������� ���� � ����-��� (��������� ���)
 * ������������ ��������:
 * true,false
 */
bool CCompileModule::Compile()
{
	wxASSERT(m_moduleObject);

	//clear functions & variables 
	Reset();

	//�������� ������ ������
	m_pContext = GetContext();

	//���������� ����������� ������ �� ������ �����-���� ��������� 
	if (m_pParent) {

		std::stack<CCompileModule *> aCompileModules;

		CCompileModule *parentModule = m_pParent; bool needRecompile = false;

		while (parentModule)
		{
			if (parentModule->m_bNeedRecompile) {
				needRecompile = true;
			}

			if (needRecompile) {
				aCompileModules.push(parentModule);
			}

			parentModule = parentModule->GetParent();
		}

		while (!aCompileModules.empty())
		{
			CCompileModule *compileModule = aCompileModules.top();

			if (!compileModule->Recompile()) {
				return false;
			}

			aCompileModules.pop();
		}
	}

	m_cByteCode.m_sModuleName = m_moduleObject->GetFullName();
	m_cByteCode.m_sDocPath = m_moduleObject->GetDocPath();
	m_cByteCode.m_sFileName = m_moduleObject->GetFileName();

	m_cByteCode.m_sModule = m_moduleObject->GetModuleText();

	if (m_pParent)
	{
		m_cByteCode.m_pParent = &m_pParent->m_cByteCode;
		m_cContext.m_parentContext = &m_pParent->m_cContext;
	}

	Load(m_moduleObject->GetModuleText());

	//prepare lexem 
	if (!PrepareLexem()) {
		return false;
	}

	//����������� ����������� ���������� 
	PrepareModuleData();

	//���������� 
	if (CompileModule()) {
		m_bNeedRecompile = false; return true;
	}

	m_bNeedRecompile = true;
	return false;
}

bool CCompileModule::IsTypeVar(const wxString &m_sType)
{
	if (!m_sType.IsEmpty())
	{
		if (CValue::IsRegisterObject(m_sType, eObjectType::eObjectType_simple))
			return true;
	}
	else
	{
		CLexem lex = PreviewGetLexem();

		if (CValue::IsRegisterObject(lex.m_sData, eObjectType::eObjectType_simple))
			return true;
	}

	return false;
}

wxString CCompileModule::GetTypeVar(const wxString &m_sType)
{
	if (!m_sType.IsEmpty())
	{
		if (!CValue::IsRegisterObject(m_sType, eObjectType::eObjectType_simple))
			SetError(ERROR_TYPE_DEF);

		return m_sType.Upper();
	}
	else
	{
		CLexem lex = GETLexem();

		if (!CValue::IsRegisterObject(lex.m_sData, eObjectType::eObjectType_simple))
			SetError(ERROR_TYPE_DEF);

		return lex.m_sData.Upper();
	}
}

/**
 * CompileDeclaration
 * ����������:
 * ���������� ������ ���������� ����������
 * ������������ ��������:
 * true,false
 */
bool CCompileModule::CompileDeclaration()
{
	wxString m_sType;
	CLexem lex = PreviewGetLexem();

	if (IDENTIFIER == lex.m_nType) m_sType = GetTypeVar(); //�������������� ������� ����������
	else GETKeyWord(KEY_VAR);

	while (true)
	{
		wxString csName0 = GETIdentifier(true);
		wxString m_sName = StringUtils::MakeUpper(csName0);

		int nParentNumber = 0;
		CCompileContext *pCurContext = GetContext();

		while (pCurContext)
		{
			nParentNumber++;

			if (nParentNumber > MAX_OBJECTS_LEVEL)
			{
				CSystemObjects::Message(pCurContext->m_compileModule->GetModuleName());
				if (nParentNumber > 2 * MAX_OBJECTS_LEVEL) CTranslateError::Error(_("Recursive call of modules!"));
			}

			if (pCurContext->FindVariable(m_sName))//�����
			{
				CVariable cCurrentVariable = pCurContext->m_cVariables[m_sName];
				if (cCurrentVariable.m_bExport || pCurContext->m_compileModule == this)
				{
					SetError(ERROR_DEF_VARIABLE, csName0);
				}
			}

			pCurContext = pCurContext->m_parentContext;
		}

		int nArrayCount = -1;
		if (IsNextDelimeter('['))//��� ���������� �������
		{
			nArrayCount = 0;
			GETDelimeter('[');
			if (!IsNextDelimeter(']'))
			{
				CValue vConst = GETConstant();
				if (vConst.GetType() != eValueTypes::TYPE_NUMBER || vConst.GetNumber() < 0) SetError(ERROR_ARRAY_SIZE_CONST);
				nArrayCount = vConst.ToInt();
			}
			GETDelimeter(']');
		}

		bool m_bExport = false;

		if (IsNextKeyWord(KEY_EXPORT))
		{
			if (m_bExport)//���� ���������� �������
				break;
			GETKeyWord(KEY_EXPORT);
			m_bExport = true;
		}


		//�� ���� ��� ���������� ���������� - ���������
		SParam Variable = m_pContext->AddVariable(csName0, m_sType, m_bExport);

		if (nArrayCount >= 0)//���������� ���������� � ��������
		{
			CByte code;
			AddLineInfo(code);
			code.m_nOper = OPER_SET_ARRAY_SIZE;
			code.m_param1 = Variable;
			code.m_param2.m_nArray = nArrayCount;//����� ��������� � �������
			m_cByteCode.m_aCodeList.push_back(code);
		}

		AddTypeSet(Variable);

		if (IsNextDelimeter('='))//��������� ������������� - �������� ������ ������ ������ ������� (�� �� ���� ������. �������� � �������)
		{
			if (nArrayCount >= 0) GETDelimeter(',');//Error!

			GETDelimeter('=');

			CByte code;
			AddLineInfo(code);
			code.m_nOper = OPER_LET;
			code.m_param1 = Variable;
			code.m_param2 = GetExpression();
			m_cByteCode.m_aCodeList.push_back(code);
		}

		if (!IsNextDelimeter(','))
			break;
		GETDelimeter(',');
	}
	return true;
}

/**
 * CompileModule
 * ����������:
 * ���������� ����� ����-���� (�������� �� ������ ������ ���������� ����)
 * ������������ ��������:
 * true,false
*/
bool CCompileModule::CompileModule()
{
	//������������� ������ �� ������ ������� ������
	m_nCurrentCompile = -1;

	m_pContext = GetContext();//�������� ������ ������

	CLexem lex;

	while ((lex = PreviewGetLexem()).m_nType != ERRORTYPE)
	{
		if ((KEYWORD == lex.m_nType && KEY_VAR == lex.m_nData) || (IDENTIFIER == lex.m_nType && IsTypeVar(lex.m_sData)))
		{
			if (!m_bCommonModule)
			{
				m_pContext = GetContext();
				CompileDeclaration();//��������� ���������� ����������
			}
			else
			{
				SetError(ERROR_ONLY_FUNCTION);
			}
		}
		else if (KEYWORD == lex.m_nType && (KEY_PROCEDURE == lex.m_nData || KEY_FUNCTION == lex.m_nData))
		{
			CompileFunction();//��������� ���������� �������
			//�� �������� ��������������� ������� �������� ������ (���� ��� �����)...
		}
		else
		{
			break;
		}
	}

	//��������� ����������� ���� ������
	m_pContext = GetContext();//�������� ������ ������
	m_cByteCode.m_nStartModule = 0;//m_cByteCode.m_aCodeList.size() - 1;
	CompileBlock();
	m_pContext->DoLabels();

	//������ ������� ����� ���������
	CByte code;
	AddLineInfo(code);
	code.m_nOper = OPER_END;
	m_cByteCode.m_aCodeList.push_back(code);
	m_cByteCode.m_nVarCount = m_pContext->m_cVariables.size();

	m_pContext = GetContext();

	//�������������� ��������� � �������, ������ ������� ���� �� �� ����������
	//��� ��� � ����� ������� ����-����� ��������� ����� ��� �� ������ ����� �������,
	//� ��� ���������� ������ � ����� ������� ������ ��������� ��������� GOTO
	for (unsigned int i = 0; i < m_apCallFunctions.size(); i++)
	{
		m_cByteCode.m_aCodeList[m_apCallFunctions[i]->m_nAddLine].m_param1.m_nIndex = m_cByteCode.m_aCodeList.size();//������� �� ����� �������

		AddCallFunction(m_apCallFunctions[i]);

		//������������� ��������
		CByte code;
		AddLineInfo(code);
		code.m_nOper = OPER_GOTO;
		code.m_param1.m_nIndex = m_apCallFunctions[i]->m_nAddLine + 1;//������ ������ ������� ������������ �����
		m_cByteCode.m_aCodeList.push_back(code);
	}

	m_pContext = GetContext();

	//�������� ������ ����������
	for (auto it : m_pContext->m_cVariables)
	{
		if (it.second.m_bTempVar || it.second.m_bContext) continue;

		m_cByteCode.m_aVarList[it.first] = it.second.m_nNumber;
		if (it.second.m_bExport) m_cByteCode.m_aExportVarList[it.first] = it.second.m_nNumber;
	}

	if (m_nCurrentCompile + 1 < m_aLexemList.size() - 1) SetError(ERROR_END_PROGRAM);

	m_cByteCode.SetModule(this);

	//���������� ��������� �������
	m_cByteCode.m_bCompile = true;

	return true;
}

//����� ����������� ������� � ������� ������ � �� ���� ������������
CFunction *CCompileModule::GetFunction(const wxString &m_sName, int *pNumber)
{
	int nCanUseLocalInParent = m_cContext.m_nFindLocalInParent - 1;
	int m_nNumber = 0;

	//���� � ������� ������
	CFunction *pDefFunction = NULL;

	if (GetContext()->FindFunction(m_sName)) pDefFunction = GetContext()->m_cFunctions[m_sName];//���� � ������� ������

	if (!pDefFunction)
	{
		CCompileModule *pCurModule = m_pParent;

		while (pCurModule)
		{
			m_nNumber++;

			pDefFunction = pCurModule->m_pContext->m_cFunctions[m_sName];

			if (pDefFunction)//�����
			{
				//������� ��� ���������� ������� ��� ���
				if (nCanUseLocalInParent > 0 || pDefFunction->m_bExport)
					break;//��

				pDefFunction = NULL;
			}

			nCanUseLocalInParent--;
			pCurModule = pCurModule->m_pParent;
		}
	}
	if (pNumber) *pNumber = m_nNumber;
	return pDefFunction;
}

//���������� � ������ ����-���� ������ �������
bool CCompileModule::AddCallFunction(CCallFunction* pRealCall)
{
	int nModuleNumber = 0;

	//������� ����������� �������
	CFunction *pDefFunction = GetFunction(pRealCall->m_sName, &nModuleNumber);
	if (!pDefFunction)
	{
		m_nCurrentCompile = pRealCall->m_nError;
		SetError(ERROR_CALL_FUNCTION, pRealCall->m_sRealName);//��� ����� ������� � ������
		return false;
	}

	//��������� ������������ ���������� ���������� � ����������� ����������
	unsigned int nRealCount = pRealCall->m_aParamList.size();
	unsigned int nDefCount = pDefFunction->m_aParamList.size();

	if (nRealCount > nDefCount)
	{
		m_nCurrentCompile = pRealCall->m_nError;
		SetError(ERROR_MANY_PARAMS);//������� ����� ����������� ����������
		return false;
	}

	CByte code;
	AddLineInfo(code);

	code.m_nNumberString = pRealCall->m_nNumberString;
	code.m_nNumberLine = pRealCall->m_nNumberLine;
	code.m_sModuleName = pRealCall->m_sModuleName;

	if (pDefFunction->m_bContext)//����������� ������� - ����� ������� �� ����������� ��������.����������(...)
	{
		code.m_nOper = OPER_CALL_M;
		code.m_param1 = pRealCall->m_sRetValue;//����������, � ������� ������������ ��������
		code.m_param2 = pRealCall->m_sContextVal;//���������� � ������� ���������� �����
		code.m_param3.m_nIndex = GetConstString(pRealCall->m_sName);//����� ����������� ������ �� ������ ������������� �������
		code.m_param3.m_nArray = nDefCount;//����� ����������
	}
	else
	{
		code.m_nOper = OPER_CALL;
		code.m_param1 = pRealCall->m_sRetValue;//����������, � ������� ������������ ��������
		code.m_param2.m_nArray = nModuleNumber;//����� ������
		code.m_param2.m_nIndex = pDefFunction->m_nStart;//��������� �������
		code.m_param3.m_nArray = nDefCount;//����� ����������
		code.m_param3.m_nIndex = pDefFunction->m_nVarCount;//����� ��������� ����������
		code.m_param4 = pRealCall->m_sContextVal;//����������� ����������
	}

	m_cByteCode.m_aCodeList.push_back(code);

	for (unsigned int i = 0; i < nDefCount; i++)
	{
		CByte code;
		AddLineInfo(code);
		code.m_nOper = OPER_SET;//���� �������� ����������

		bool bDefaultValue = false;

		if (i < nRealCount) {
			code.m_param1 = pRealCall->m_aParamList[i];

			if (code.m_param1.m_nArray == DEF_VAR_SKIP) { //����� ���������� �������� �� ���������
				bDefaultValue = true;
			}
			else {  //��� �������� ��������
				code.m_param2.m_nIndex = pDefFunction->m_aParamList[i].m_bByRef;
			}
		}
		else {
			bDefaultValue = true;
		}

		if (bDefaultValue) {
			if (pDefFunction->m_aParamList[i].m_vData.m_nArray == DEF_VAR_SKIP) {
				m_nCurrentCompile = pRealCall->m_nError;
				SetError(ERROR_FEW_PARAMS);//������������ ����������� ����������
			}

			code.m_nOper = OPER_SETCONST;//�������� �� ���������
			code.m_param1 = pDefFunction->m_aParamList[i].m_vData;
		}

		m_cByteCode.m_aCodeList.push_back(code);
	}

	return true;
}

/**
 * CompileFunction
 * ����������:
 * �������� ���������� ���� ��� ����� ������� (���������)
 * ��������:
 * -���������� ����� ���������� ����������
 * -���������� ������� ������ ���������� ���������� (�� ������ ��� �� ��������)
 * -���������� �������� �� ���������
 * -���������� ����� ��������� ����������
 * -���������� ���������� �� ������� ��������
 *
 * ������������ ��������:
 * true,false
 */
bool CCompileModule::CompileFunction()
{
	//������ �� �� ������ �������, ��� ������ �������� ����� FUNCTION ��� PROCEDURE
	CLexem lex;

	if (IsNextKeyWord(KEY_FUNCTION))
	{
		GETKeyWord(KEY_FUNCTION);
		m_pContext = new CCompileContext(GetContext());//������� ����� ��������, � ������� ����� ������������� ���� �������
		m_pContext->SetModule(this);
		m_pContext->m_nReturn = RETURN_FUNCTION;
	}
	else if (IsNextKeyWord(KEY_PROCEDURE))
	{
		GETKeyWord(KEY_PROCEDURE);
		m_pContext = new CCompileContext(GetContext());//������� ����� ��������, � ������� ����� ������������� ���� ���������
		m_pContext->SetModule(this);
		m_pContext->m_nReturn = RETURN_PROCEDURE;
	}
	else
	{
		SetError(ERROR_FUNC_DEFINE);
	}

	//����������� ����� ���������� �������
	lex = PreviewGetLexem();

	wxString sShortDescription;
	int m_nNumberLine = lex.m_nNumberLine;
	int nRes = m_sBuffer.find('\n', lex.m_nNumberString);
	if (nRes >= 0)
	{
		sShortDescription = m_sBuffer.Mid(lex.m_nNumberString, nRes - lex.m_nNumberString - 1);
		nRes = sShortDescription.find_first_of('/');
		if (nRes > 0)
		{
			if (sShortDescription[nRes - 1] == '/')//���� - ��� �����������
			{
				sShortDescription = sShortDescription.Mid(nRes + 1);
			}
		}
		else
		{
			nRes = sShortDescription.find_first_of(')');
			sShortDescription = sShortDescription.Left(nRes + 1);
		}
	}

	//�������� ��� �������
	wxString sFuncName0 = GETIdentifier(true);
	wxString sFuncName = StringUtils::MakeUpper(sFuncName0);

	int m_nError = m_nCurrentCompile;

	CFunction *pFunction = new CFunction(sFuncName, m_pContext);
	pFunction->m_sRealName = sFuncName0;
	pFunction->m_sShortDescription = sShortDescription;
	pFunction->m_nNumberLine = m_nNumberLine;

	//����������� ������ ���������� ���������� + ������������ �� ��� ���������
	GETDelimeter('(');

	if (!IsNextDelimeter(')'))
	{
		while (true)
		{
			wxString m_sType = wxEmptyString;
			//�������� �� ����������������
			if (IsTypeVar()) m_sType = GetTypeVar();

			CParamVariable cVariable;
			if (IsNextKeyWord(KEY_VAL)) {
				GETKeyWord(KEY_VAL);
				cVariable.m_bByRef = true;
			}

			wxString m_sRealName = GETIdentifier(true);

			cVariable.m_sName = m_sRealName;
			cVariable.m_sType = m_sType;

			//������������ ��� ���������� ��� ���������
			if (m_pContext->FindVariable(m_sRealName)) { //���� ���������� + ��������� ���������� = ������
				SetError(ERROR_IDENTIFIER_DUPLICATE, m_sRealName);
			}

			if (IsNextDelimeter('[')) {//��� ������
				GETDelimeter('[');
				GETDelimeter(']');
			}
			else if (IsNextDelimeter('=')) {
				GETDelimeter('=');
				cVariable.m_vData = FindConst(GETConstant());
			}

			m_pContext->AddVariable(m_sRealName, m_sType);
			pFunction->m_aParamList.push_back(cVariable);

			if (IsNextDelimeter(')')) break;
			GETDelimeter(',');
		}
	}

	GETDelimeter(')');

	if (IsNextKeyWord(KEY_EXPORT)) {
		GETKeyWord(KEY_EXPORT);
		pFunction->m_bExport = true;
	}

	int nParentNumber = 0;
	CCompileContext *pCurContext = GetContext();

	while (pCurContext)
	{
		nParentNumber++;

		if (nParentNumber > MAX_OBJECTS_LEVEL)
		{
			CSystemObjects::Message(pCurContext->m_compileModule->GetModuleName());
			if (nParentNumber > 2 * MAX_OBJECTS_LEVEL) CTranslateError::Error(_("Recursive call of modules!"));
		}

		if (pCurContext->FindFunction(sFuncName))//�����
		{
			CFunction *m_pCurrentFunc = pCurContext->m_cFunctions[sFuncName];
			if (m_pCurrentFunc->m_bExport || pCurContext->m_compileModule == this)
			{
				m_nCurrentCompile = m_nError;
				SetError(ERROR_DEF_FUNCTION, sFuncName0);
			}
		}

		pCurContext = pCurContext->m_parentContext;
	}

	//�������� �� ����������������
	GetContext()->m_cFunctions[sFuncName] = pFunction;

	//��������� ���������� � ������� � ������ ����-�����:
	CByte code0;
	AddLineInfo(code0);
	code0.m_nOper = OPER_FUNC;

#if defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(_WIN64)
	code0.m_param1.m_nArray = reinterpret_cast<wxLongLong_t>(m_pContext);
#else
	code0.m_param1.m_nArray = reinterpret_cast<int>(m_pContext);
#endif

	m_cByteCode.m_aCodeList.push_back(code0);
	
	int nAddres = pFunction->m_nStart = m_cByteCode.m_aCodeList.size() - 1;
	m_cByteCode.m_aFuncList[sFuncName] = nAddres + 1;

	if (pFunction->m_bExport) {
		m_cByteCode.m_aExportFuncList[sFuncName] = nAddres + 1;
	}

	for (unsigned int i = 0; i < pFunction->m_aParamList.size(); i++)
	{
		//add set oper
		CByte code;
		AddLineInfo(code);

		if (pFunction->m_aParamList[i].m_vData.m_nArray == DEF_VAR_CONST) {
			code.m_nOper = OPER_SETCONST;//���� �������� ����������
		}
		else {
			code.m_nOper = OPER_SET;//���� �������� ����������
		}

		code.m_param1 = pFunction->m_aParamList[i].m_vData;
		code.m_param2.m_nIndex = pFunction->m_aParamList[i].m_bByRef;

		m_cByteCode.m_aCodeList.push_back(code);

		//Set type variable
		SParam Variable;

		Variable.m_sType = pFunction->m_aParamList[i].m_sType;
		Variable.m_nArray = 0;
		Variable.m_nIndex = i;//������ ��������� � �������

		AddTypeSet(Variable);
	}

	GetContext()->m_sCurFuncName = sFuncName;
	CompileBlock();
	m_pContext->DoLabels();
	GetContext()->m_sCurFuncName = wxEmptyString;

	if (m_pContext->m_nReturn == RETURN_FUNCTION) GETKeyWord(KEY_ENDFUNCTION);
	else GETKeyWord(KEY_ENDPROCEDURE);

	CByte code;
	AddLineInfo(code);
	code.m_nOper = OPER_ENDFUNC;
	m_cByteCode.m_aCodeList.push_back(code);

	pFunction->m_nFinish = m_cByteCode.m_aCodeList.size() - 1;
	pFunction->m_nVarCount = m_pContext->m_cVariables.size();

	m_cByteCode.m_aCodeList[nAddres].m_param3.m_nIndex = pFunction->m_nVarCount;//����� ��������� ����������
	m_cByteCode.m_aCodeList[nAddres].m_param3.m_nArray = pFunction->m_aParamList.size();//����� ���������� ����������

	m_pContext->SetFunction(pFunction);
	return true;
}

/**
 * ���������� ���������� � ���� ����������
 */
void CCompileModule::AddTypeSet(const SParam &sVariable)
{
	wxString typeName = sVariable.m_sType;

	if (!typeName.IsEmpty()) {
		CByte code;
		AddLineInfo(code);
		code.m_nOper = OPER_SET_TYPE;

		code.m_param1 = sVariable;
		code.m_param2.m_nArray = CValue::GetIDObjectFromString(typeName);

		m_cByteCode.m_aCodeList.push_back(code);
	}
}

//������ �������� ���������� Var ���� Str
#define CheckTypeDef(var,type) if(wxStrlen(type) > 0)\
	{\
		if(var.m_sType!=type)\
		{\
			if (CValue::CompareObjectName(type, eValueTypes::TYPE_BOOLEAN)) SetError(ERROR_BAD_TYPE_EXPRESSION_B);\
			else if (CValue::CompareObjectName(type, eValueTypes::TYPE_NUMBER)) SetError(ERROR_BAD_TYPE_EXPRESSION_N);\
			else if (CValue::CompareObjectName(type, eValueTypes::TYPE_STRING))  SetError(ERROR_BAD_TYPE_EXPRESSION_S);\
			else if (CValue::CompareObjectName(type, eValueTypes::TYPE_DATE)) SetError(ERROR_BAD_TYPE_EXPRESSION_D);\
			else SetError(ERROR_BAD_TYPE_EXPRESSION);\
		}\
		if (CValue::CompareObjectName(type, eValueTypes::TYPE_NUMBER)) code.m_nOper+=TYPE_DELTA1;\
		else if (CValue::CompareObjectName(type, eValueTypes::TYPE_STRING)) code.m_nOper+=TYPE_DELTA2;\
		else if (CValue::CompareObjectName(type, eValueTypes::TYPE_DATE)) code.m_nOper+=TYPE_DELTA3;\
        else if (CValue::CompareObjectName(type, eValueTypes::TYPE_BOOLEAN)) code.m_nOper+=TYPE_DELTA4;\
	}


//������ ������������� �������� �� ���� ����������
//���� ��� �������������, �� ����� ����������� �������������� ��������
#define CorrectTypeDef(sKey)\
if(!sKey.m_sType.IsEmpty())\
{\
	if (CValue::CompareObjectName(sKey.m_sType, eValueTypes::TYPE_NUMBER)) code.m_nOper+=TYPE_DELTA1;\
	else if (CValue::CompareObjectName(sKey.m_sType, eValueTypes::TYPE_STRING)) code.m_nOper+=TYPE_DELTA2;\
	else if (CValue::CompareObjectName(sKey.m_sType, eValueTypes::TYPE_DATE)) code.m_nOper+=TYPE_DELTA3;\
    else if (CValue::CompareObjectName(sKey.m_sType, eValueTypes::TYPE_BOOLEAN))  code.m_nOper+=TYPE_DELTA4;\
	else SetError(ERROR_BAD_TYPE_EXPRESSION);\
}

/**
 * CompileBlock
 * ����������:
 * �������� ���������� ���� ��� ������ ����� (����� ���� ����� ������-����
 * ������������ �������� ���� ����...���������, ����...��������� � �.�.
 * nIterNumber - ����� ���������� �����
 * ������������ ��������:
 * true,false
 */
bool CCompileModule::CompileBlock()
{
	CLexem lex;
	while ((lex = PreviewGetLexem()).m_nType != ERRORTYPE)
	{
		if (IDENTIFIER == lex.m_nType&&IsTypeVar(lex.m_sData))
		{
			CompileDeclaration();
		}
		if (KEYWORD == lex.m_nType)
		{
			switch (lex.m_nData)
			{
			case KEY_VAR://������� ���������� � ��������
				CompileDeclaration();
				break;
			case KEY_NEW:
				CompileNewObject();
				break;
			case KEY_IF:
				CompileIf();
				break;
			case KEY_WHILE:
				CompileWhile();
				break;
			case KEY_FOREACH:
				CompileForeach();
				break;
			case KEY_FOR:
				CompileFor();
				break;
			case KEY_GOTO:
				CompileGoto();
				break;
			case KEY_RETURN:
			{
				GETKeyWord(KEY_RETURN);

				if (m_pContext->m_nReturn == RETURN_NONE) SetError(ERROR_USE_RETURN); //�������� Return (�������) �� ����� ������������� ��� ��������� ��� �������

				CByte code;
				AddLineInfo(code);
				code.m_nOper = OPER_RET;

				if (m_pContext->m_nReturn == RETURN_FUNCTION)//������������ �����-�� ��������
				{
					if (IsNextDelimeter(';'))
					{
						SetError(ERROR_EXPRESSION_REQUIRE);
					}

					code.m_param1 = GetExpression();
				}
				else
				{
					code.m_param1.m_nArray = DEF_VAR_NORET;
					code.m_param1.m_nIndex = DEF_VAR_NORET;
				}

				m_cByteCode.m_aCodeList.push_back(code);
				break;
			}
			case KEY_TRY:
			{
				GETKeyWord(KEY_TRY);
				CByte code;
				AddLineInfo(code);
				code.m_nOper = OPER_TRY;
				m_cByteCode.m_aCodeList.push_back(code);
				int nLineTry = m_cByteCode.m_aCodeList.size() - 1;

				CompileBlock();
				code.m_nOper = OPER_ENDTRY;
				m_cByteCode.m_aCodeList.push_back(code);
				int nAddrLine = m_cByteCode.m_aCodeList.size() - 1;

				m_cByteCode.m_aCodeList[nLineTry].m_param1.m_nIndex = m_cByteCode.m_aCodeList.size();

				GETKeyWord(KEY_EXCEPT);
				CompileBlock();
				GETKeyWord(KEY_ENDTRY);

				m_cByteCode.m_aCodeList[nAddrLine].m_param1.m_nIndex = m_cByteCode.m_aCodeList.size();
				break;
			}
			case KEY_RAISE:
			{
				GETKeyWord(KEY_RAISE);
				CByte code;
				AddLineInfo(code);
				code.m_nOper = OPER_RAISE;
				m_cByteCode.m_aCodeList.push_back(code);
				break;
			}
			case KEY_CONTINUE:
			{
				GETKeyWord(KEY_CONTINUE);
				if (m_pContext->aContinueList[m_pContext->m_nDoNumber])
				{
					CByte code;
					AddLineInfo(code);
					code.m_nOper = OPER_GOTO;
					m_cByteCode.m_aCodeList.push_back(code);
					int nAddrLine = m_cByteCode.m_aCodeList.size() - 1;
					CDefIntList *pList = m_pContext->aContinueList[m_pContext->m_nDoNumber];
					pList->push_back(nAddrLine);
				}
				else SetError(ERROR_USE_CONTINUE);//��������� Continue (����������)  ����� ������������� ������ ������ �����		
				break;
			}
			case KEY_BREAK:
			{
				GETKeyWord(KEY_BREAK);
				if (m_pContext->aBreakList[m_pContext->m_nDoNumber])
				{
					CByte code;
					AddLineInfo(code);
					code.m_nOper = OPER_GOTO;
					m_cByteCode.m_aCodeList.push_back(code);
					int nAddrLine = m_cByteCode.m_aCodeList.size() - 1;
					CDefIntList *pList = m_pContext->aBreakList[m_pContext->m_nDoNumber];
					pList->push_back(nAddrLine);
				}
				else SetError(ERROR_USE_BREAK);//��������� Break(��������)  ����� ������������� ������ ������ �����
				break;
			}
			case KEY_FUNCTION:
			case KEY_PROCEDURE:
			{
				GetLexem();
				SetError(ERROR_USE_BLOCK);
				break;
			}
			default: return true;//������ ����������� ����������� ������ ���� ����������� ������ (�������� ���������, ����������, ������������ � �.�.)		
			}
		}
		else
		{
			lex = GetLexem();

			if (IDENTIFIER == lex.m_nType)
			{
				m_pContext->m_nTempVar = 0;

				if (IsNextDelimeter(':'))//��� ����������� ������� �����
				{
					unsigned int pLabel = m_pContext->m_cLabelsDef[lex.m_sData];
					if (pLabel > 0) SetError(ERROR_IDENTIFIER_DUPLICATE, lex.m_sData);//��������� ������������� ����������� �����

					//���������� ����� ��������:
					m_pContext->m_cLabelsDef[lex.m_sData] = m_cByteCode.m_aCodeList.size() - 1;
					GETDelimeter(':');
				}
				else//����� �������������� ������ �������, �������, ������������� ���������
				{
					m_nCurrentCompile--;//��� �����
					int nSet = 1;
					if (m_bCommonModule && m_pContext == GetContext()) SetError(ERROR_ONLY_FUNCTION);
					SParam Variable = GetCurrentIdentifier(nSet);//�������� ����� ����� ��������� (�� ����� '=')
					if (nSet)//���� ���� ������ �����, �.�. ���� '='
					{
						GETDelimeter('=');//��� ������������ ���������� ������-�� ���������
						SParam sExpression = GetExpression();
						CByte code;
						code.m_nOper = OPER_LET;
						AddLineInfo(code);

						CheckTypeDef(sExpression, Variable.m_sType);
						Variable.m_sType = sExpression.m_sType;

						bool bShortLet = false; int n = 0;

						if (DEF_VAR_TEMP == sExpression.m_nArray)//��������� ������ ��������� ����������
						{
							n = m_cByteCode.m_aCodeList.size() - 1;
							if (n >= 0)
							{
								int nOperation = m_cByteCode.m_aCodeList[n].m_nOper;
								nOperation = nOperation % TYPE_DELTA1;
								if (OPER_MULT == nOperation ||
									OPER_DIV == nOperation ||
									OPER_ADD == nOperation ||
									OPER_SUB == nOperation ||
									OPER_MOD == nOperation ||
									OPER_GT == nOperation ||
									OPER_GE == nOperation ||
									OPER_LS == nOperation ||
									OPER_LE == nOperation ||
									OPER_NE == nOperation ||
									OPER_EQ == nOperation
									)
								{
									bShortLet = true;//��������� ���� ������������
								}
							}
						}

						if (bShortLet)
						{
							m_cByteCode.m_aCodeList[n].m_param1 = Variable;
						}
						else
						{
							code.m_param1 = Variable;
							code.m_param2 = sExpression;
							m_cByteCode.m_aCodeList.push_back(code);
						}
					}
				}
			}
			else if (DELIMITER == lex.m_nType&&';' == lex.m_nData) {}
			else if (ENDPROGRAM == lex.m_nType) 	break;
			else SetError(ERROR_CODE);
		}
	}//while
	return true;
}//CompileBlock

bool CCompileModule::CompileNewObject()
{
	GETKeyWord(KEY_NEW);

	wxString sObjectName = GETIdentifier(true);
	int m_nNumber = GetConstString(sObjectName);

	std::vector <SParam> m_aParamList;

	if (IsNextDelimeter('('))//��� ����� ������
	{
		GETDelimeter('(');

		while (!IsNextDelimeter(')'))
		{
			if (IsNextDelimeter(','))
			{
				SParam data;
				data.m_nArray = DEF_VAR_SKIP;//����������� ��������
				data.m_nIndex = DEF_VAR_SKIP;
				m_aParamList.push_back(data);
			}
			else
			{
				m_aParamList.emplace_back(GetExpression());
				if (IsNextDelimeter(')')) break;
			}
			GETDelimeter(',');
		}

		GETDelimeter(')');
	}

	if (!CValue::IsRegisterObject(sObjectName, eObjectType::eObjectType_object))
		SetError(ERROR_CALL_CONSTRUCTOR, sObjectName);

	CByte code;
	AddLineInfo(code);
	code.m_nOper = OPER_NEW;

	code.m_param2.m_nIndex = m_nNumber;//����� ����������� ������ �� ������ ������������� �������
	code.m_param2.m_nArray = m_aParamList.size();//����� ����������

	SParam Variable = GetVariable();
	code.m_param1 = Variable;//����������, � ������� ������������ ��������
	m_cByteCode.m_aCodeList.push_back(code);

	for (unsigned int i = 0; i < m_aParamList.size(); i++)
	{
		CByte code;
		AddLineInfo(code);
		code.m_nOper = OPER_SET;
		code.m_param1 = m_aParamList[i];
		m_cByteCode.m_aCodeList.push_back(code);
	}

	return true;
}

/**
 * CompileGoto
 * ����������:
 * �������������� ��������� GOTO (����������� ������������ ����� ��������
 * ��� ����������� �� ������ �� ����� � ��� = LABEL)
 * ������������ ��������:
 * true,false
 */
bool CCompileModule::CompileGoto()
{
	GETKeyWord(KEY_GOTO);

	SLabel data;
	data.m_sName = GETIdentifier();
	data.m_nLine = m_cByteCode.m_aCodeList.size();//���������� �� ��������, ������� ����� ���� ����� ����������
	data.m_nError = m_nCurrentCompile;
	m_pContext->m_cLabels.push_back(data);

	CByte code;
	AddLineInfo(code);
	code.m_nOper = OPER_GOTO;
	m_cByteCode.m_aCodeList.push_back(code);

	return true;
}

/*
 * GetCurrentIdentifier
 * ����������:
 * �������������� �������������� (����������� ��� ���� ��� ����������,�������� ��� �������,������)
 * nIsSet - �� �����:  1 - ������� ���� ��� �������� ��������� ������������ ��������� (���� ���������� ���� '=')
 * ������������ ��������:
 * nIsSet - �� ������: 1 - ������� ���� ��� ����� ��������� ������������ ��������� (�.�. ������ ����������� ���� '=')
 * ����� ������ ����������, ��� ����� �������� ��������������
*/
SParam CCompileModule::GetCurrentIdentifier(int &nIsSet)
{
	SParam Variable; int nPrevSet = nIsSet;

	wxString m_sRealName = GETIdentifier(true);
	wxString m_sName = StringUtils::MakeUpper(m_sRealName);

	if (IsNextDelimeter('('))//��� ����� �������
	{
		wxString sContextName;

		if (m_cContext.FindFunction(m_sName, sContextName, true))
		{
			int m_nNumber = GetConstString(m_sRealName);

			std::vector <SParam> m_aParamList;
			GETDelimeter('(');
			while (!IsNextDelimeter(')'))
			{
				if (IsNextDelimeter(','))
				{
					SParam data;
					data.m_nArray = DEF_VAR_SKIP;//����������� ��������
					data.m_nIndex = DEF_VAR_SKIP;
					m_aParamList.push_back(data);
				}
				else
				{
					m_aParamList.emplace_back(GetExpression());
					if (IsNextDelimeter(')')) break;
				}
				GETDelimeter(',');
			}

			GETDelimeter(')');

			CByte code;
			AddLineInfo(code);
			code.m_nOper = OPER_CALL_M;

			// ���������� � ������� ���������� �����
			code.m_param2 = GetVariable(sContextName, false, true);

			code.m_param3.m_nIndex = m_nNumber;//����� ����������� ������ �� ������ ������������� �������
			code.m_param3.m_nArray = m_aParamList.size();//����� ����������
			Variable = GetVariable();
			code.m_param1 = Variable;//����������, � ������� ������������ ��������
			m_cByteCode.m_aCodeList.push_back(code);

			for (unsigned int i = 0; i < m_aParamList.size(); i++)
			{
				CByte code;
				AddLineInfo(code);
				code.m_nOper = OPER_SET;
				code.m_param1 = m_aParamList[i];
				m_cByteCode.m_aCodeList.push_back(code);
			}
		}
		else
		{
			CFunction *pDefFunction = NULL;

			if (m_bExpressionOnly) {
				pDefFunction = GetFunction(m_sName);
			}
			else if (GetContext()->FindFunction(m_sName)) {
				pDefFunction = GetContext()->m_cFunctions[m_sName];//���� � ������� ������
			}

			if (!nIsSet && pDefFunction && pDefFunction->m_pContext && pDefFunction->m_pContext->m_nReturn == RETURN_PROCEDURE) {
				SetError(ERROR_USE_PROCEDURE_AS_FUNCTION, pDefFunction->m_sRealName);
			}

			Variable = GetCallFunction(m_sRealName);
		}

		if (IsTypeVar(m_sRealName)) {
			Variable.m_sType = GetTypeVar(m_sRealName);//��� ���������� �����
		}

		nIsSet = 0;
	}
	else//��� ����� ����������
	{
		wxString sContextName; nIsSet = 1;

		if (m_cContext.FindVariable(m_sRealName, sContextName, true))
		{
			CByte code;
			AddLineInfo(code);

			int m_nNumber = GetConstString(m_sRealName);

			if (IsNextDelimeter('=') && nPrevSet == 1)
			{
				GETDelimeter('='); nIsSet = 0;

				code.m_nOper = OPER_SET_A;
				code.m_param1 = GetVariable(sContextName, false, true);//���������� � ������� ���������� �������
				code.m_param2.m_nIndex = m_nNumber;//����� ����������� ������ �� ������ ������������� ��������� � �������
				code.m_param3 = GetExpression();

				m_cByteCode.m_aCodeList.push_back(code);

				return Variable;
			}
			else
			{
				code.m_nOper = OPER_GET_A;
				code.m_param2 = GetVariable(sContextName, false, true);//���������� � ������� ���������� �������
				code.m_param3.m_nIndex = m_nNumber;//����� ����������� �������� �� ������ ������������� ��������� � �������
				Variable = GetVariable();
				code.m_param1 = Variable;//����������, � ������� ������������ ��������
				m_cByteCode.m_aCodeList.push_back(code);
			}
		}
		else
		{
			bool bCheckError = !nPrevSet;

			if (IsNextDelimeter('.'))//��� ���������� �������� ����� ������
				bCheckError = true;

			Variable = GetVariable(m_sRealName, bCheckError);
		}
	}

MLabel:

	if (IsNextDelimeter('['))//��� ������
	{
		GETDelimeter('[');
		SParam sKey = GetExpression();
		GETDelimeter(']');
		//���������� ��� ������ (�.�. ��� ��������� �������� ������� ��� ���������)
		//������:
		//���[10]=12; - Set
		//�=���[10]; - Get
		//���[10][2]=12; - Get,Set
		nIsSet = 0;

		if (IsNextDelimeter('['))//�������� ���� ���������� ������� (��������� ����������� ��������)
		{
			CByte code;
			AddLineInfo(code);
			code.m_nOper = OPER_CHECK_ARRAY;
			code.m_param1 = Variable;//���������� - ������
			code.m_param2 = sKey;//������ �������
			m_cByteCode.m_aCodeList.push_back(code);
		}

		if (IsNextDelimeter('=') && nPrevSet == 1)
		{
			GETDelimeter('=');
			CByte code;
			AddLineInfo(code);
			code.m_nOper = OPER_SET_ARRAY;
			code.m_param1 = Variable;//���������� - ������
			code.m_param2 = sKey;//������ ������� (������ ���� �.�. ������������ ������������� ������)
			code.m_param3 = GetExpression();

			CorrectTypeDef(sKey);//�������� ���� �������� ��������� ����������

			m_cByteCode.m_aCodeList.push_back(code);
			return Variable;
		}
		else
		{
			CByte code;
			AddLineInfo(code);
			code.m_nOper = OPER_GET_ARRAY;

			code.m_param2 = Variable;//���������� - ������
			code.m_param3 = sKey;//������ ������� (������ ���� �.�. ������������ ������������� ������)
			Variable = GetVariable();
			code.m_param1 = Variable;//����������, � ������� ������������ ��������

			CorrectTypeDef(sKey);//�������� ���� �������� ��������� ����������

			m_cByteCode.m_aCodeList.push_back(code);
		}

		goto MLabel;
	}

	if (IsNextDelimeter('.'))//��� ����� ������ ��� �������� ����������� �������
	{
		GETDelimeter('.');

		wxString sRealMethod = GETIdentifier(true);
		//wxString sMethod = StringUtils::MakeUpper(sRealMethod);

		int m_nNumber = GetConstString(sRealMethod);

		if (IsNextDelimeter('('))//��� ����� ������
		{
			std::vector <SParam> m_aParamList;
			GETDelimeter('(');
			while (!IsNextDelimeter(')'))
			{
				if (IsNextDelimeter(','))
				{
					SParam data;
					data.m_nArray = DEF_VAR_SKIP;//����������� ��������
					data.m_nIndex = DEF_VAR_SKIP;
					m_aParamList.push_back(data);
				}
				else
				{
					m_aParamList.emplace_back(GetExpression());
					if (IsNextDelimeter(')')) break;
				}
				GETDelimeter(',');
			}

			GETDelimeter(')');

			CByte code;
			AddLineInfo(code);
			code.m_nOper = OPER_CALL_M;

			code.m_param2 = Variable; // ���������� � ������� ���������� �����
			code.m_param3.m_nIndex = m_nNumber;//����� ����������� ������ �� ������ ������������� �������
			code.m_param3.m_nArray = m_aParamList.size();//����� ����������
			Variable = GetVariable();
			code.m_param1 = Variable;//����������, � ������� ������������ ��������
			m_cByteCode.m_aCodeList.push_back(code);

			for (unsigned int i = 0; i < m_aParamList.size(); i++)
			{
				CByte code;
				AddLineInfo(code);
				code.m_nOper = OPER_SET;
				code.m_param1 = m_aParamList[i];
				m_cByteCode.m_aCodeList.push_back(code);
			}

			nIsSet = 0;
		}
		else//����� - ����� ��������
		{
			//���������� ��� ������ (�.�. ��� ��������� �������� ��� ���������)
			//������:
			//�=���.�����; - Get
			//���.�����=0; - Set
			//���.�����.���=0;  - Get,Set
			CByte code;
			AddLineInfo(code);

			if (IsNextDelimeter('=') && nPrevSet == 1)
			{
				GETDelimeter('='); 	nIsSet = 0;
				code.m_nOper = OPER_SET_A;
				code.m_param1 = Variable;//���������� � ������� ���������� �������
				code.m_param2.m_nIndex = m_nNumber;//����� ����������� ������ �� ������ ������������� ��������� � �������
				code.m_param3 = GetExpression();
				m_cByteCode.m_aCodeList.push_back(code);
				return Variable;
			}
			else
			{
				code.m_nOper = OPER_GET_A;
				code.m_param2 = Variable;//���������� � ������� ���������� �������
				code.m_param3.m_nIndex = m_nNumber;//����� ����������� �������� �� ������ ������������� ��������� � �������
				Variable = GetVariable();
				code.m_param1 = Variable;//����������, � ������� ������������ ��������
				m_cByteCode.m_aCodeList.push_back(code);
			}
		}

		goto MLabel;
	}

	return Variable;
}

bool CCompileModule::CompileIf()
{
	std::vector <int>aAddrLine;

	GETKeyWord(KEY_IF);

	SParam sParam;
	CByte code;
	AddLineInfo(code);
	code.m_nOper = OPER_IF;

	sParam = GetExpression();
	code.m_param1 = sParam;
	CorrectTypeDef(sParam);//�������� ���� ��������

	m_cByteCode.m_aCodeList.push_back(code);

	int nLastIFLine = m_cByteCode.m_aCodeList.size() - 1;

	GETKeyWord(KEY_THEN);
	CompileBlock();

	while (IsNextKeyWord(KEY_ELSEIF))
	{
		//���������� ����� �� ���� �������� ��� ����������� �����
		code.m_nOper = OPER_GOTO;
		m_cByteCode.m_aCodeList.push_back(code);
		aAddrLine.push_back(m_cByteCode.m_aCodeList.size() - 1);//�������� ��� ��������� GOTO ����� �������� �����

		//��� ����������� ������� ������������� ����� ������� ��� ������������ �������
		m_cByteCode.m_aCodeList[nLastIFLine].m_param2.m_nIndex = m_cByteCode.m_aCodeList.size();

		GETKeyWord(KEY_ELSEIF);
		AddLineInfo(code);
		code.m_nOper = OPER_IF;

		sParam = GetExpression();
		code.m_param1 = sParam;
		CorrectTypeDef(sParam);//�������� ���� ��������

		m_cByteCode.m_aCodeList.push_back(code);
		nLastIFLine = m_cByteCode.m_aCodeList.size() - 1;

		GETKeyWord(KEY_THEN);
		CompileBlock();
	}

	if (IsNextKeyWord(KEY_ELSE))
	{
		//���������� ����� �� ���� �������� ��� ����������� �����
		AddLineInfo(code);
		code.m_nOper = OPER_GOTO;
		m_cByteCode.m_aCodeList.push_back(code);
		aAddrLine.push_back(m_cByteCode.m_aCodeList.size() - 1);//�������� ��� ��������� GOTO ����� �������� �����

		//��� ����������� ������� ������������� ����� ������� ��� ������������ �������
		m_cByteCode.m_aCodeList[nLastIFLine].m_param2.m_nIndex = m_cByteCode.m_aCodeList.size();
		nLastIFLine = 0;

		GETKeyWord(KEY_ELSE);
		CompileBlock();
	}

	GETKeyWord(KEY_ENDIF);

	int nCurCompile = m_cByteCode.m_aCodeList.size();

	//��� ���������� ������� ������������� ����� ������� ��� ������������ �������
	m_cByteCode.m_aCodeList[nLastIFLine].m_param2.m_nIndex = nCurCompile;

	//������������� �������� ��� ��������� GOTO - ����� �� ���� ��������� �������
	for (unsigned int i = 0; i < aAddrLine.size(); i++)
	{
		m_cByteCode.m_aCodeList[aAddrLine[i]].m_param1.m_nIndex = nCurCompile;
	}

	return true;
}

bool CCompileModule::CompileWhile()
{
	m_pContext->StartDoList();

	GETKeyWord(KEY_WHILE);
	CByte code;
	AddLineInfo(code);
	code.m_nOper = OPER_IF;

	SParam sParam = GetExpression();
	code.m_param1 = sParam;
	CorrectTypeDef(sParam);//�������� ���� ��������

	m_cByteCode.m_aCodeList.push_back(code);

	int nStartWhile = m_cByteCode.m_aCodeList.size() - 1;

	GETKeyWord(KEY_DO);
	CompileBlock();
	GETKeyWord(KEY_ENDDO);

	CByte code2;
	AddLineInfo(code2);
	code2.m_nOper = OPER_GOTO;
	code2.m_param1.m_nIndex = nStartWhile;
	m_cByteCode.m_aCodeList.push_back(code2);

	m_cByteCode.m_aCodeList[nStartWhile].m_param2.m_nIndex = m_cByteCode.m_aCodeList.size();

	//���������� ������ ��������� ��� ������ Continue � Break
	m_pContext->FinishDoList(m_cByteCode, m_cByteCode.m_aCodeList.size() - 1, m_cByteCode.m_aCodeList.size());

	return true;
}

bool CCompileModule::CompileFor()
{
	m_pContext->StartDoList();

	GETKeyWord(KEY_FOR);

	wxString m_sRealName = GETIdentifier(true);
	wxString m_sName = StringUtils::MakeUpper(m_sRealName);

	SParam Variable = GetVariable(m_sRealName);

	//�������� ���� ����������
	if (!Variable.m_sType.IsEmpty())
	{
		if (!CValue::CompareObjectName(Variable.m_sType, eValueTypes::TYPE_NUMBER))
			SetError(ERROR_NUMBER_TYPE);
	}

	GETDelimeter('=');
	SParam Variable2 = GetExpression();

	CByte code0;
	AddLineInfo(code0);
	code0.m_nOper = OPER_LET;
	code0.m_param1 = Variable;
	code0.m_param2 = Variable2;
	m_cByteCode.m_aCodeList.push_back(code0);

	//�������� ���� ��������
	if (!Variable.m_sType.IsEmpty())
	{
		if (!CValue::CompareObjectName(Variable2.m_sType, eValueTypes::TYPE_NUMBER))
			SetError(ERROR_BAD_TYPE_EXPRESSION);
	}

	GETKeyWord(KEY_TO);
	SParam VariableTo = m_pContext->GetVariable(m_sName + wxT("@to"), true, false, false, true); //loop variable

	CByte code1;
	AddLineInfo(code1);
	code1.m_nOper = OPER_LET;
	code1.m_param1 = VariableTo;
	code1.m_param2 = GetExpression();
	m_cByteCode.m_aCodeList.push_back(code1);

	CByte code;
	AddLineInfo(code);
	code.m_nOper = OPER_FOR;
	code.m_param1 = Variable;
	code.m_param2 = VariableTo;
	m_cByteCode.m_aCodeList.push_back(code);

	int nStartFOR = m_cByteCode.m_aCodeList.size() - 1;

	GETKeyWord(KEY_DO);
	CompileBlock();
	GETKeyWord(KEY_ENDDO);

	CByte code2;
	AddLineInfo(code2);
	code2.m_nOper = OPER_NEXT;
	code2.m_param1 = Variable;
	code2.m_param2.m_nIndex = nStartFOR;
	m_cByteCode.m_aCodeList.push_back(code2);

	m_cByteCode.m_aCodeList[nStartFOR].m_param3.m_nIndex = m_cByteCode.m_aCodeList.size();

	//���������� ������ ��������� ��� ������ Continue � Break
	m_pContext->FinishDoList(m_cByteCode, m_cByteCode.m_aCodeList.size() - 1, m_cByteCode.m_aCodeList.size());

	return true;
}

bool CCompileModule::CompileForeach()
{
	m_pContext->StartDoList();

	GETKeyWord(KEY_FOREACH);

	wxString m_sRealName = GETIdentifier(true);
	wxString m_sName = StringUtils::MakeUpper(m_sRealName);

	SParam Variable = GetVariable(m_sRealName);

	GETKeyWord(KEY_IN);

	SParam VariableIn = m_pContext->GetVariable(m_sName + wxT("@in"), true, false, false, true); //loop variable

	CByte code1;
	AddLineInfo(code1);
	code1.m_nOper = OPER_LET;
	code1.m_param1 = VariableIn;
	code1.m_param2 = GetExpression();
	m_cByteCode.m_aCodeList.push_back(code1);

	SParam VariableIt = m_pContext->GetVariable(m_sName + wxT("@it"), true, false, false, true);  //storage iterpos;

	CByte code;
	AddLineInfo(code);
	code.m_nOper = OPER_FOREACH;
	code.m_param1 = Variable;
	code.m_param2 = VariableIn;
	code.m_param3 = VariableIt; // for storage iterpos;
	m_cByteCode.m_aCodeList.push_back(code);

	int nStartFOREACH = m_cByteCode.m_aCodeList.size() - 1;

	GETKeyWord(KEY_DO);
	CompileBlock();
	GETKeyWord(KEY_ENDDO);

	CByte code2;
	AddLineInfo(code2);
	code2.m_nOper = OPER_NEXT_ITER;
	code2.m_param1 = VariableIt; // for storage iterpos;
	code2.m_param2.m_nIndex = nStartFOREACH;
	m_cByteCode.m_aCodeList.push_back(code2);

	m_cByteCode.m_aCodeList[nStartFOREACH].m_param4.m_nIndex = m_cByteCode.m_aCodeList.size();

	//���������� ������ ��������� ��� ������ Continue � Break
	m_pContext->FinishDoList(m_cByteCode, m_cByteCode.m_aCodeList.size() - 1, m_cByteCode.m_aCodeList.size());

	return true;
}

/**
 * ��������� ������ ������� ��� ���������
 */
SParam CCompileModule::GetCallFunction(const wxString &m_sRealName)
{
	CCallFunction *pRealCall = new CCallFunction();
	wxString m_sName = StringUtils::MakeUpper(m_sRealName);

	CFunction *pDefFunction = NULL;

	if (m_bExpressionOnly)
		pDefFunction = GetFunction(m_sName);
	else if (GetContext()->FindFunction(m_sName))
		pDefFunction = GetContext()->m_cFunctions[m_sName];//���� � ������� ������

	pRealCall->m_nError = m_nCurrentCompile;//��� ������ ��������� ��� �������

	pRealCall->m_sName = m_sName;
	pRealCall->m_sRealName = m_sRealName;

	GETDelimeter('(');
	while (!IsNextDelimeter(')'))
	{
		if (IsNextDelimeter(','))
		{
			SParam data;
			data.m_nArray = DEF_VAR_SKIP;//����������� ��������
			data.m_nIndex = DEF_VAR_SKIP;
			pRealCall->m_aParamList.push_back(data);
		}
		else
		{
			pRealCall->m_aParamList.emplace_back(GetExpression());
			if (IsNextDelimeter(')')) break;
		}
		GETDelimeter(',');
	}
	GETDelimeter(')');

	SParam Variable = GetVariable();

	CByte code;
	AddLineInfo(code);

	pRealCall->m_nNumberString = code.m_nNumberString;
	pRealCall->m_nNumberLine = code.m_nNumberLine;
	pRealCall->m_sModuleName = code.m_sModuleName;
	pRealCall->m_sRetValue = Variable;

	if (pDefFunction && GetContext()->m_sCurFuncName != m_sName)
	{
		AddCallFunction(pRealCall);
		delete pRealCall;
	}
	else
	{
		if (m_bExpressionOnly) SetError(ERROR_CALL_FUNCTION, m_sRealName);

		code.m_nOper = OPER_GOTO;//������� � ����� ����-����, ��� ����� ������������� ����������� �����
		m_cByteCode.m_aCodeList.push_back(code);
		pRealCall->m_nAddLine = m_cByteCode.m_aCodeList.size() - 1;
		m_apCallFunctions.push_back(pRealCall);
	}

	return Variable;
}

/**
 * �������� ����� ��������� �� ����������� ������ ��������
 * (���� ������ �������� � ������ ���, �� ��� ���������)
 */
SParam CCompileModule::FindConst(CValue &m_vData)
{
	SParam Const; Const.m_nArray = DEF_VAR_CONST;

	wxString sConst = wxString::Format(wxT("%d:%s"), m_vData.GetType(), m_vData.GetString());

	if (m_aHashConstList[sConst]) {
		Const.m_nIndex = m_aHashConstList[sConst] - 1;
	}
	else {
		Const.m_nIndex = m_cByteCode.m_aConstList.size();
		m_cByteCode.m_aConstList.push_back(m_vData);
		m_aHashConstList[sConst] = Const.m_nIndex + 1;
	}

	Const.m_sType = GetTypeVar(m_vData.GetTypeString());
	return Const;
}

#define SetOper(x)	code.m_nOper=x;

/**
 * ���������� ������������� ��������� (��������� ������ �� ����� �������)
 */
SParam CCompileModule::GetExpression(int nPriority)
{
	SParam Variable;
	CLexem lex = GETLexem();

	//������� ������������ ����� ���������
	if ((lex.m_nType == KEYWORD && lex.m_nData == KEY_NOT) || (lex.m_nType == DELIMITER && lex.m_nData == '!'))
	{
		Variable = GetVariable();
		SParam Variable2 = GetExpression(aPriority['!']);
		CByte code;
		code.m_nOper = OPER_NOT;
		AddLineInfo(code);

		if (!Variable2.m_sType.IsEmpty())
		{
			CheckTypeDef(Variable2, CValue::GetNameObjectFromVT(eValueTypes::TYPE_BOOLEAN));
		}

		Variable.m_sType = CValue::GetNameObjectFromVT(eValueTypes::TYPE_BOOLEAN, true);

		code.m_param1 = Variable;
		code.m_param2 = Variable2;
		m_cByteCode.m_aCodeList.push_back(code);
	}
	else if ((lex.m_nType == KEYWORD && lex.m_nData == KEY_NEW))
	{
		wxString sObjectName = GETIdentifier(true);
		int m_nNumber = GetConstString(sObjectName);

		std::vector <SParam> m_aParamList;

		if (IsNextDelimeter('('))//��� ����� ������
		{
			GETDelimeter('(');

			while (!IsNextDelimeter(')'))
			{
				if (IsNextDelimeter(','))
				{
					SParam data;
					data.m_nArray = DEF_VAR_SKIP;//����������� ��������
					data.m_nIndex = DEF_VAR_SKIP;
					m_aParamList.push_back(data);
				}
				else
				{
					m_aParamList.emplace_back(GetExpression());
					if (IsNextDelimeter(')')) break;
				}
				GETDelimeter(',');
			}

			GETDelimeter(')');
		}

		if (lex.m_nData == KEY_NEW && !CValue::IsRegisterObject(sObjectName, eObjectType::eObjectType_object))
			SetError(ERROR_CALL_CONSTRUCTOR, sObjectName);

		CByte code;
		AddLineInfo(code);
		code.m_nOper = OPER_NEW;

		code.m_param2.m_nIndex = m_nNumber;//����� ����������� ������ �� ������ ������������� �������
		code.m_param2.m_nArray = m_aParamList.size();//����� ����������

		Variable = GetVariable();
		code.m_param1 = Variable;//����������, � ������� ������������ ��������
		m_cByteCode.m_aCodeList.push_back(code);

		for (unsigned int i = 0; i < m_aParamList.size(); i++)
		{
			CByte code;
			AddLineInfo(code);
			code.m_nOper = OPER_SET;
			code.m_param1 = m_aParamList[i];
			m_cByteCode.m_aCodeList.push_back(code);
		}
	}
	else if (lex.m_nType == DELIMITER && lex.m_nData == '(')
	{
		Variable = GetExpression();
		GETDelimeter(')');
	}
	else if (lex.m_nType == DELIMITER && lex.m_nData == '?')
	{
		Variable = GetVariable();
		CByte code;
		AddLineInfo(code);
		code.m_nOper = OPER_ITER;
		code.m_param1 = Variable;
		GETDelimeter('(');
		code.m_param2 = GetExpression();
		GETDelimeter(',');
		code.m_param3 = GetExpression();
		GETDelimeter(',');
		code.m_param4 = GetExpression();
		GETDelimeter(')');
		m_cByteCode.m_aCodeList.push_back(code);
	}
	else if (lex.m_nType == IDENTIFIER)
	{
		m_nCurrentCompile--;//��� �����
		int nSet = 0;
		Variable = GetCurrentIdentifier(nSet);
	}
	else if (lex.m_nType == CONSTANT)
	{
		Variable = FindConst(lex.m_vData);
	}
	else if ((lex.m_nType == DELIMITER && lex.m_nData == '+') || (lex.m_nType == DELIMITER && lex.m_nData == '-'))
	{
		//��������� ������������ ������ �������
		int nCurPriority = aPriority[lex.m_nData];

		if (nPriority >= nCurPriority) SetError(ERROR_EXPRESSION);//���������� ���������� ����� (���������� ��������) � ������� ����������� ��������

		//��� ������� ������������� ����� ���������
		if (lex.m_nData == '+')//������ �� ������ (����������)
		{
			CByte code;
			Variable = GetExpression(nPriority);
			if (!Variable.m_sType.IsEmpty()) { CheckTypeDef(Variable, CValue::GetNameObjectFromVT(eValueTypes::TYPE_NUMBER)); }
			Variable.m_sType = CValue::GetNameObjectFromVT(eValueTypes::TYPE_NUMBER, true);
			return Variable;
		}
		else
		{
			Variable = GetExpression(100);//����� ������� ���������!
			CByte code;
			AddLineInfo(code);
			code.m_nOper = OPER_INVERT;

			if (!Variable.m_sType.IsEmpty()) CheckTypeDef(Variable, CValue::GetNameObjectFromVT(eValueTypes::TYPE_NUMBER));

			code.m_param2 = Variable;
			Variable = GetVariable();
			Variable.m_sType = CValue::GetNameObjectFromVT(eValueTypes::TYPE_NUMBER, true);
			code.m_param1 = Variable;
			m_cByteCode.m_aCodeList.push_back(code);
		}
	}
	else
	{
		SetError(ERROR_EXPRESSION);
	}

	//������ ������������ ������ ���������
	//���� � Variable ����� ������ ������ ���������� ���������

MOperation:

	lex = PreviewGetLexem();

	if (lex.m_nType == DELIMITER && lex.m_nData == ')') return Variable;

	//������� ���� �� ����� ��������� ���������� �������� ��� ������ ����������
	if ((lex.m_nType == DELIMITER && lex.m_nData != ';') || (lex.m_nType == KEYWORD && lex.m_nData == KEY_AND) || (lex.m_nType == KEYWORD && lex.m_nData == KEY_OR))
	{
		if (lex.m_nData >= 0 && lex.m_nData <= 255)
		{
			int nCurPriority = aPriority[lex.m_nData];
			if (nPriority < nCurPriority)//���������� ���������� ����� (���������� ��������) � ������� ����������� ��������
			{
				CByte code;
				AddLineInfo(code);
				lex = GetLexem();

				if (lex.m_nData == '*') {
					SetOper(OPER_MULT);
				}
				else if (lex.m_nData == '/') {
					SetOper(OPER_DIV);
				}
				else if (lex.m_nData == '+') {
					SetOper(OPER_ADD);
				}
				else if (lex.m_nData == '-') {
					SetOper(OPER_SUB);
				}
				else if (lex.m_nData == '%') {
					SetOper(OPER_MOD);
				}
				else if (lex.m_nData == KEY_AND) {
					SetOper(OPER_AND);
				}
				else if (lex.m_nData == KEY_OR) {
					SetOper(OPER_OR);
				}
				else if (lex.m_nData == '>')
				{
					SetOper(OPER_GT);
					if (IsNextDelimeter('=')) {
						GETDelimeter('=');
						SetOper(OPER_GE);
					}
				}
				else if (lex.m_nData == '<')
				{
					SetOper(OPER_LS);
					if (IsNextDelimeter('=')) {
						GETDelimeter('=');
						SetOper(OPER_LE);
					}
					else if (IsNextDelimeter('>')) {
						GETDelimeter('>');
						SetOper(OPER_NE);
					}
				}
				else if (lex.m_nData == '=') {
					SetOper(OPER_EQ);
				}
				else {
					SetError(ERROR_EXPRESSION);
				}

				SParam Variable1 = GetVariable();
				SParam Variable2 = Variable;
				SParam Variable3 = GetExpression(nCurPriority);

				if (DEF_VAR_TEMP != Variable3.m_nArray && DEF_VAR_CONST != Variable3.m_nArray) { //���. �������� �� ����������� ��������
					if (CValue::CompareObjectName(Variable2.m_sType, eValueTypes::TYPE_STRING)) {
						if (OPER_DIV == code.m_nOper || OPER_MOD == code.m_nOper || OPER_MULT == code.m_nOper || OPER_AND == code.m_nOper || OPER_OR == code.m_nOper) {
							SetError(ERROR_TYPE_OPERATION);
						}
					}
				}

				if (DEF_VAR_CONST != Variable2.m_nArray && DEF_VAR_TEMP != Variable2.m_nArray) { //��������� �� ���������  - �.�. ��� ������������ �� �������
					CheckTypeDef(Variable3, Variable2.m_sType);
				}

				Variable1.m_sType = Variable2.m_sType;

				if (code.m_nOper >= OPER_GT && code.m_nOper <= OPER_NE) {
					Variable1.m_sType = CValue::GetNameObjectFromVT(eValueTypes::TYPE_BOOLEAN, true);
				}

				code.m_param1 = Variable1;
				code.m_param2 = Variable2;
				code.m_param3 = Variable3;

				m_cByteCode.m_aCodeList.push_back(code);

				Variable = Variable1;
				goto MOperation;
			}
		}
	}

	return Variable;
}

void CCompileModule::SetParent(CCompileModule *pSetParent)
{
	m_cByteCode.m_pParent = NULL;

	m_pParent = pSetParent;
	m_cContext.m_parentContext = NULL;

	if (m_pParent)
	{
		m_cByteCode.m_pParent = &m_pParent->m_cByteCode;
		m_cContext.m_parentContext = &m_pParent->m_cContext;
	}

	OnSetParent(m_pParent);
};

SParam CCompileModule::AddVariable(const wxString &m_sName, const wxString &m_sType, bool m_bExport, bool m_bContext, bool m_bTempVar)
{
	return m_pContext->AddVariable(m_sName, m_sType, m_bExport, m_bContext, m_bTempVar);
}

/**
 * ������� ���������� ����� ���������� �� ���������� �����
 */
SParam CCompileModule::GetVariable(const wxString &m_sName, bool bCheckError, bool bLoadFromContext)
{
	return m_pContext->GetVariable(m_sName, true, bCheckError, bLoadFromContext);
}

/**
 * C������ ����� ������������� ����������
 */
SParam CCompileModule::GetVariable()
{
	wxString m_sName = wxString::Format(wxT("@%d"), m_pContext->m_nTempVar);//@ - ��� �������� ������������ �����
	SParam Variable = m_pContext->GetVariable(m_sName, false, false, false, true);//��������� ���������� ���� ������ � ��������� ���������
	Variable.m_nArray = DEF_VAR_TEMP;//������� ��������� ��������� ����������
	m_pContext->m_nTempVar++;
	return Variable;
}

#pragma warning(pop)