////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, 2�-team
//	Description : translate module 
////////////////////////////////////////////////////////////////////////////

#include "translateModule.h"
#include "definition.h"
#include "utils/stringutils.h"

extern std::map<wxString, void *>	aHelpDescription;//�������� �������� ���� � ��������� �������
extern std::map<wxString, void *>	aHashKeywordList;

CDefList CTranslateModule::glDefList; //���������� ������ �����������

std::map<wxString, void *>	CTranslateModule::m_aHashKeyWords;//������ �������� ����

//////////////////////////////////////////////////////////////////////
// Global array
//////////////////////////////////////////////////////////////////////
struct aKeyWordsDef aKeyWords[] =
{
	{"if"},
	{"then"},
	{"else"},
	{"elseif"},
	{"endif"},
	{"for"},
	{"foreach"},
	{"to"},
	{"in"},
	{"do"},
	{"enddo"},
	{"while"},
	{"goto"},
	{"not"},
	{"and"},
	{"or"},
	{"procedure"},
	{"endprocedure"},
	{"function"},
	{"endfunction"},
	{"export"},
	{"val"},
	{"return"},
	{"try"},
	{"except"},
	{"endtry"},
	{"continue"},
	{"break"},
	{"raise"},
	{"var"},

	//create object
	{"new"},

	//undefined type
	{"undefined"},

	//null type
	{"null"},

	//boolean type
	{"true"},
	{"false"},

	//preprocessor:
	{"#define"},
	{"#undef"},
	{"#ifdef"},
	{"#ifndef"},
	{"#else"},
	{"#endif"},
	{"#region"},
	{"#endregion"},
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTranslateModule::CTranslateModule(const wxString &moduleName) : m_aDefList(NULL), m_sModuleName(moduleName),
m_bAutoDeleteDefList(false), m_nModePreparing(LEXEM_ADD)
{
	//���������� ������ �������� ����
	if (m_aHashKeyWords.size() == 0) {
		LoadKeyWords(); //������ ���� ���
	}

	Clear();
}

CTranslateModule::~CTranslateModule()
{
	if (m_bAutoDeleteDefList && m_aDefList) {
		delete m_aDefList;
	}
}

/**
 *���������� ������ �������� ����
 */
void CTranslateModule::LoadKeyWords()
{
	m_aHashKeyWords.clear();

#if defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(_WIN64)
	for (unsigned long long i = 0; i < sizeof(aKeyWords) / sizeof(aKeyWords[0]); i++)
#else 
	for (unsigned int i = 0; i < sizeof(aKeyWords) / sizeof(aKeyWords[0]); i++)
#endif 
	{
		wxString csEng = StringUtils::MakeUpper(aKeyWords[i].Eng);

		m_aHashKeyWords[csEng] = (void*)(i + 1);

		//��������� � ������ ��� ��������������� �����������
		aHashKeywordList[csEng] = (void*)1;
		aHelpDescription[csEng] = &aKeyWords[i].sShortDescription;
	}
}

//////////////////////////////////////////////////////////////////////
// Translating
//////////////////////////////////////////////////////////////////////

/**
* Clear
* ����������:
* ����������� ���������� ��� ������ ����������
* ������������ ��������:
* ���
*/
void CTranslateModule::Clear()
{
	m_sBuffer.clear();
	m_sBUFFER.clear();

	m_nSizeText = m_nCurPos = m_nCurLine = 0;
}

/**
 * Load
 * ����������:
 * ��������� ����� �������� ������� + ����������� ���������� ��� ����������
 * ������������ ��������:
 * ���
 */
void CTranslateModule::Load(const wxString &sCode)
{
	Clear();

	m_sBuffer = sCode;
	m_sBuffer.Trim();

	m_nSizeText = m_sBuffer.Length();

	if (m_sBuffer.IsEmpty()) return;

	m_sBUFFER = m_sBuffer;
	m_sBUFFER.MakeUpper();
}

/**
 * SetError
 * ����������:
 * ��������� ������ ���������� � ������� ����������
 * ������������ ��������:
 * ����� �� ���������� ����������!s
 */
void CTranslateModule::SetError(int nErr, int m_nCurPos, const wxString &sError)
{
	wxString sErrorMessage = wxEmptyString;

	if (CTranslateError::IsSimpleMode())
	{
		sErrorMessage = CTranslateError::FindErrorCodeLine(m_sBuffer, m_nCurPos, 4);
	}
	else
	{
		sErrorMessage = wxString::Format("%s %s", m_sModuleName, CTranslateError::FindErrorCodeLine(m_sBuffer, m_nCurPos, 0));
	}

	CTranslateError::Error(nErr, sErrorMessage.wc_str(), sError.wc_str());
}

/**
 * SkipSpaces
 * ����������:
 * ���������� ��� ���������� ������� �� ������ �����
 * ���� ����������� �� ������ ����� �������������� � ����� ������
 * ������������ ��������:
 * ���
 */
void CTranslateModule::SkipSpaces()
{
	unsigned int i = m_nCurPos;

	for (; i < m_nSizeText; i++)
	{
		char c = m_sBUFFER[i];

		if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
		{
			if (c == '/')//����� ��� �����������
			{
				if (i + 1 < m_nSizeText)
				{
					if (m_sBUFFER[i + 1] == '/')//���������� �����������
					{
						for (unsigned int j = i; j < m_nSizeText; j++)
						{
							m_nCurPos = j;
							if (m_sBUFFER[j] == '\n' || m_sBUFFER[j] == 13)
							{
								//������������ ��������� ������
								SkipSpaces();
								return;
							}
						}
						i = m_nCurPos + 1;
					}
				}
			}
			m_nCurPos = i;
			break;
		}
		else if (c == '\n')
		{
			m_nCurLine++;
		}
	}

	if (i == m_nSizeText)
	{
		m_nCurPos = m_nSizeText;
	}
}

/**
 * IsByte
 * ����������:
 * ��������� �������� �� ��������� ���� (��� ����� ��������) ������
 * ��������� �����
 * ������������ ��������:
 * true,false
 */
bool CTranslateModule::IsByte(char c)
{
	SkipSpaces();

	if (m_nCurPos >= m_nSizeText) return false;
	if (m_sBUFFER[m_nCurPos] == c) return true;
	return false;
}

/**
 * GetByte
 * ����������:
 * �������� �� ������� ���� (��� ����� ��������)
 * ���� ������ ����� ���, �� ��������� ����������
 * ������������ ��������:
 * ���� �� ������
 */
char CTranslateModule::GetByte()
{
	SkipSpaces();

	if (m_nCurPos < m_nSizeText)  return m_sBuffer[m_nCurPos++];
	SetError(ERROR_TRANSLATE_BYTE, m_nCurPos);
	return 0;
}

/**
 * IsWord
 * ����������:
 * ��������� (�� ������� ������� �������� ������)
 * �������� �� ��������� ����� ���� ������ (��������� ������� � ��.)
 * ������������ ��������:
 * true,false
 */
bool CTranslateModule::IsWord()
{
	SkipSpaces();

	if (m_nCurPos < m_nSizeText)
	{
		if (((m_sBUFFER[m_nCurPos] == '_') ||
			(m_sBUFFER[m_nCurPos] >= 'A' && m_sBUFFER[m_nCurPos] <= 'Z') || (m_sBUFFER[m_nCurPos] >= 'a' && m_sBUFFER[m_nCurPos] <= 'z') ||
			(m_sBUFFER[m_nCurPos] >= '�' && m_sBUFFER[m_nCurPos] <= '�') || (m_sBUFFER[m_nCurPos] >= '�' && m_sBUFFER[m_nCurPos] <= '�') ||
			(m_sBUFFER[m_nCurPos] == '#')) && (m_sBUFFER[m_nCurPos] != '[' && m_sBUFFER[m_nCurPos] != ']'))
			return true;
	}
	return false;
}

/**
 * GetWord
 * ����������:
 * ������� �� ������ ��������� �����
 * ���� ����� ��� (�.�. ��������� ����� ���� �� �������� ������), �� ��������� ����������
 * ��������: bGetPoint
 * true - ��������� ����� ��� ��������� ����� ����� (��� ��������� ����� ���������)
 * ������������ ��������:
 * ����� �� ������
 */
wxString CTranslateModule::GetWord(bool bOrigin, bool bGetPoint, wxString *psOrig)
{
	SkipSpaces();

	if (m_nCurPos >= m_nSizeText) SetError(ERROR_TRANSLATE_WORD, m_nCurPos);

	int nNext = m_nCurPos;

	for (unsigned int i = m_nCurPos; i < m_nSizeText; i++)
	{
		char c = m_sBUFFER[i];

		// if array then break
		if (c == '[' || c == ']') break;

		if ((c == '_') ||
			(c >= 'A'&&c <= 'Z') || (c >= 'a'&&c <= 'z') ||
			(c >= '�'&&c <= '�') || (c >= '�'&&c <= '�') ||
			(c >= '0'&&c <= '9') ||
			(c == '#'&&i == m_nCurPos) || //���� ������ ������ # - ��� ��������� �����
			(c == '.'&&bGetPoint))
		{
			if (c == '.' && bGetPoint)
				bGetPoint = false; //����� ������ ����������� ������ ���� ���
			nNext = i + 1;
		}
		else break;
	}

	int nFirst = m_nCurPos;
	m_nCurPos = nNext;
	if (nFirst == nNext) SetError(ERROR_TRANSLATE_WORD, nFirst);

	if (bOrigin) return m_sBuffer.Mid(nFirst, nNext - nFirst);
	else
	{
		if (psOrig)
			*psOrig = m_sBuffer.Mid(nFirst, nNext - nFirst);
		return m_sBUFFER.Mid(nFirst, nNext - nFirst);
	}
}

/**
 * GetStrToEndLine
 * ����������:
 * �������� ��� ������ �� ����� (������� 13 ��� ����� ���� ���������)
 * ������
 */
wxString CTranslateModule::GetStrToEndLine()
{
	unsigned int nStart = m_nCurPos;
	unsigned int i = m_nCurPos;

	for (; i < m_nSizeText; i++)
	{
		if (m_sBuffer[i] == '\r' || m_sBuffer[i] == '\n') { i++; break; }
	}
	m_nCurPos = i;
	return m_sBuffer.Mid(nStart, m_nCurPos - nStart);
}

/**
 * IsNumber
 * ����������:
 * ��������� (�� ������� ������� �������� ������)
 * �������� �� ��������� ����� ���� ������-���������� (��������� ������� � ��.)
 * ������������ ��������:
 * true,false
*/
bool CTranslateModule::IsNumber()
{
	SkipSpaces();

	if (m_nCurPos < m_nSizeText)
	{
		return m_sBUFFER[m_nCurPos] >= '0' && m_sBUFFER[m_nCurPos] <= '9';
	}

	return false;
}

/**
 * GetNumber
 * ����������:
 * �������� �� ������� �����
 * ���� �������� ����� ���, �� ��������� ����������
 * ������������ ��������:
 * ����� �� ������
 */
wxString CTranslateModule::GetNumber()
{
	if (!IsNumber()) SetError(ERROR_TRANSLATE_NUMBER, m_nCurPos);

	SkipSpaces();

	if (m_nCurPos >= m_nSizeText) SetError(ERROR_TRANSLATE_NUMBER, m_nCurPos);
	int nNext = m_nCurPos; bool bGetPoint = true;

	for (unsigned int i = m_nCurPos; i < m_nSizeText; i++)
	{
		char c = m_sBUFFER[i];

		if ((c >= '0' && c <= '9') || (c == '.' && bGetPoint))
		{
			if (c == '.' && bGetPoint)
				bGetPoint = false; //����� ������ ����������� ������ ���� ���
			nNext = i + 1;
		}
		else break;
	}

	int nFirst = m_nCurPos;
	m_nCurPos = nNext;
	if (nFirst == nNext) SetError(ERROR_TRANSLATE_NUMBER, nFirst);

	return m_sBUFFER.Mid(nFirst, nNext - nFirst);
}

/**
 * IsString
 * ����������:
 * ��������� �������� �� ��������� ����� �������� (��� ����� ��������) �������-����������, ����������� � �������
 * ������������ ��������:
 * true,false
 */
bool CTranslateModule::IsString()
{
	return IsByte('\"');
}

/**
 * GetString
 * ����������:
 * �������� �� ������� ������, ����������� � �������
 * ���� ����� ������ ���, �� ��������� ����������
 * ������������ ��������:
 * ������ �� ������
 */
wxString CTranslateModule::GetString()
{
	wxString sString;
	if (!IsString()) SetError(ERROR_TRANSLATE_STRING, m_nCurPos);
	SkipSpaces();

	unsigned int nStart = 0;
	unsigned int i = m_nCurPos + 1;

	for (; i < m_nSizeText; i++)
	{
		wxChar c = m_sBuffer[i];

		if (c == '\"')
		{
			if (i < m_nSizeText - 1)
			{
				i++;
				c = m_sBuffer[i];
				if (c != '\"')
					break;
			}
		}
		else//��������� �������������� ������
		{
			if (c == '\n')
			{
				if (nStart > 0)
					break;

				nStart = sString.length();
			}
			else if (c == '|' && nStart > 0)
			{
				//�������� �������� �����
				sString = sString.Left(nStart);
				nStart = 0;
				continue;
			}
			else if (c == '/' && nStart > 0)
			{
				if (m_sBuffer[i + 1] == '/')//��� ����������� - ����������
				{
					i = m_sBuffer.find('\n', i + 1);
					if (i < 0)
					{
						i = m_nSizeText;
						break;
					}
					continue;
				}
			}
			/*else if (c != '\t' && c != ' ')
			{
				nStart = 0;
			}*/
		}

		sString += c;
	}

	m_nCurPos = i;

	return sString;
}

/**
* IsDate
* ����������:
* ��������� �������� �� ��������� ����� �������� (��� ����� ��������) �����-����������, ����������� � ���������
* ������������ ��������:
* true,false
*/
bool CTranslateModule::IsDate()
{
	return IsByte('\'');
}

/**
* GetDate
* ����������:
* �������� �� ������� ����, ����������� � ���������
* ���� ����� ���� ���, �� ��������� ����������
* ������������ ��������:
* ���� �� ������, ����������� � �������
*/
wxString CTranslateModule::GetDate()
{
	if (!IsDate()) SetError(ERROR_TRANSLATE_DATE, m_nCurPos);

	unsigned int nCount = 0;

	SkipSpaces();

	if (m_nCurPos >= m_nSizeText) SetError(ERROR_TRANSLATE_WORD, m_nCurPos);

	int nNext = m_nCurPos;

	for (unsigned int i = m_nCurPos; i < m_nSizeText; i++)
	{
		if (nCount < 2)
		{
			if (m_sBUFFER[i] == '\'')
				nCount++;
			nNext = i + 1;
		}
		else break;
	}

	int nFirst = m_nCurPos;
	m_nCurPos = nNext;
	if (nFirst == nNext) SetError(ERROR_TRANSLATE_DATE, nFirst);
	return m_sBuffer.Mid(nFirst + 1, nNext - nFirst - 2);
}

/**
 * IsEnd
 * ����������:
 * ��������� ������� ��������� �������������� (�.�. ����� �� ����� ������)
 * ������������ ��������:
 * true,false
 */
bool CTranslateModule::IsEnd()
{
	SkipSpaces();

	if (m_nCurPos < m_nSizeText) return false;
	return true;
}

/**
 * IsKeyWord
 * ����������:
 * ���������� �������� �� �������� ����� ��������� ����������
 * ������������ ��������,����:
 * -1: ���
 * ������ ��� ����� 0: ����� � ������ ��������� ����
*/
#if defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(_WIN64)
long long CTranslateModule::IsKeyWord(const wxString &sKeyWord)
#else 
int CTranslateModule::IsKeyWord(const wxString &sKeyWord)
#endif
{
	auto itHashKeyWords = m_aHashKeyWords.find(StringUtils::MakeUpper(sKeyWord));
#if defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(_WIN64)
	if (itHashKeyWords != m_aHashKeyWords.end()) return ((long long)itHashKeyWords->second) - 1;
#else 
	if (itHashKeyWords != m_aHashKeyWords.end()) return ((int)itHashKeyWords->second) - 1;
#endif 
	return wxNOT_FOUND;
}

/**
 * PrepareLexem
 * ������1 - �������� ������ ��� ������������ �������� ������� ��� �������������
 */
bool CTranslateModule::PrepareLexem()
{
	if (!m_aDefList)
	{
		m_aDefList = new CDefList();
		m_aDefList->SetParent(&glDefList);
		m_bAutoDeleteDefList = true;//�������, ��� ������ � ������������� ������� �� (� �� ������� � �������� ���������� �����������)
	}

	wxString s;

	m_aLexemList.clear();

	while (!IsEnd())
	{
		CLexem bytecode;
		bytecode.m_nNumberLine = m_nCurLine;
		bytecode.m_nNumberString = m_nCurPos;//���� � ���������� ���������� ������, �� ������ ��� ������ ����� ������ ������������
		bytecode.m_sModuleName = m_sModuleName;

		if (IsWord())
		{
			wxString sOrig;
			s = GetWord(false, false, &sOrig);

			//��������� ����������� ������������ (#define)
			if (m_aDefList->HasDef(s))
			{
				CLexemList *pDef = m_aDefList->GetDef(s);

				for (unsigned int i = 0; i < pDef->size(); i++)
				{
					CLexem *lex = pDef[i].data();

					lex->m_nNumberString = m_nCurPos;
					lex->m_nNumberLine = m_nCurLine;//��� ����� ��������
					lex->m_sModuleName = m_sModuleName;

					m_aLexemList.push_back(*lex);
				}

				continue;
			}

			//undefined
			if (s.Lower() == wxT("undefined"))
			{
				bytecode.m_nType = CONSTANT;
				bytecode.m_vData.SetType(eValueTypes::TYPE_EMPTY);
			}
			//boolean
			else if (s.Lower() == wxT("true") || s.Lower() == wxT("false"))
			{
				bytecode.m_nType = CONSTANT;
				bytecode.m_vData.SetBoolean(s);
			}
			//null
			else if (s.Lower() == wxT("null"))
			{
				bytecode.m_nType = CONSTANT;
				bytecode.m_vData.SetType(eValueTypes::TYPE_NULL);
			}

			if (bytecode.m_nType != CONSTANT)
			{
				int n = IsKeyWord(s);

				bytecode.m_vData = sOrig;

				if (n >= 0)
				{
					bytecode.m_nType = KEYWORD;
					bytecode.m_nData = n;
				}
				else
				{
					bytecode.m_nType = IDENTIFIER;
				}
			}
		}
		else if (IsNumber() || IsString() || IsDate())
		{
			bytecode.m_nType = CONSTANT;

			if (IsNumber())
			{
				bytecode.m_vData.SetNumber(GetNumber());

				int n = m_aLexemList.size() - 1;

				if (n >= 0)
				{
					if (m_aLexemList[n].m_nType == DELIMITER && (m_aLexemList[n].m_nData == '-' || m_aLexemList[n].m_nData == '+'))
					{
						n--;
						if (n >= 0)
						{
							if (m_aLexemList[n].m_nType == DELIMITER && (m_aLexemList[n].m_nData == '[' || m_aLexemList[n].m_nData == '(' || m_aLexemList[n].m_nData == ',' || m_aLexemList[n].m_nData == '<' || m_aLexemList[n].m_nData == '>' || m_aLexemList[n].m_nData == '='))
							{
								n++;
								if (m_aLexemList[n].m_nData == '-')
									bytecode.m_vData.m_fData = -bytecode.m_vData.m_fData;
								m_aLexemList[n] = bytecode;
								continue;
							}
						}
					}
				}
			}
			else
			{
				if (IsString())
				{
					bytecode.m_vData.SetString(GetString());
				}
				else if (IsDate())
				{
					bytecode.m_vData.SetDate(GetDate());
				}
			}

			m_aLexemList.push_back(bytecode);
			continue;
		}
		else if (IsByte('~'))
		{
			s.clear();

			GetByte();//���������� ����������� � �������. ������ ����� (��� ������)
			continue;
		}
		else
		{
			s.clear();

			bytecode.m_nType = DELIMITER;
			bytecode.m_nData = GetByte();

			if (bytecode.m_nData <= 13)
			{
				continue;
			}
		}

		bytecode.m_sData = s;

		if (bytecode.m_nType == KEYWORD)
		{
			if (bytecode.m_nData == KEY_DEFINE && m_nModePreparing != LEXEM_ADDDEF)//������� ������������� ��������������
			{
				if (!IsWord())
					SetError(ERROR_IDENTIFIER_DEFINE, m_nCurPos);

				wxString sName = GetWord();

				//��������� �������������� ��������� � ������ �����������
				if (LEXEM_ADD == m_nModePreparing)
					PrepareFromCurrent(LEXEM_ADDDEF, sName);
				else
					PrepareFromCurrent(LEXEM_IGNORE, sName);

				continue;
			}
			else if (bytecode.m_nData == KEY_UNDEF)//�������� ��������������
			{
				if (!IsWord())
					SetError(ERROR_IDENTIFIER_DEFINE, m_nCurPos);

				wxString sName = GetWord();
				m_aDefList->RemoveDef(sName);
				continue;
			}
			else if (bytecode.m_nData == KEY_IFDEF || bytecode.m_nData == KEY_IFNDEF)//�������� ��������������
			{
				if (!IsWord())
					SetError(ERROR_IDENTIFIER_DEFINE, m_nCurPos);
				wxString sName = GetWord();
				bool bHasDef = m_aDefList->HasDef(sName);
				if (bytecode.m_nData == KEY_IFNDEF)
					bHasDef = !bHasDef;

				//����������� ���� ���� ���� �� ���������� #else ��� #endif
				int nMode = 0;

				if (bHasDef)
					nMode = LEXEM_ADD;//��������� �������������� ��������� � ������ ������
				else
					nMode = LEXEM_IGNORE;//����� ����������
				PrepareFromCurrent(nMode);

				if (!IsWord())
					SetError(ERROR_USE_ENDDEF, m_nCurPos);

				wxString sWord = GetWord();
				if (IsKeyWord(sWord) == KEY_ELSEDEF)//����� #else
				{
					//��� ��� �����������
					if (!bHasDef)
						nMode = LEXEM_ADD;//��������� �������������� ��������� � ������ ������
					else
						nMode = LEXEM_IGNORE;//����� ����������
					PrepareFromCurrent(nMode);

					if (!IsWord())
						SetError(ERROR_USE_ENDDEF, m_nCurPos);
					sWord = GetWord();
				}

				//������� #endif
				if (IsKeyWord(sWord) != KEY_ENDIFDEF)
					SetError(ERROR_USE_ENDDEF, m_nCurPos);

				continue;
			}
			else if (bytecode.m_nData == KEY_ENDIFDEF)//����� ��������� ��������������
			{
				m_nCurPos = bytecode.m_nNumberString;//����� �� ��������� ���������� ��������
				break;
			}
			else if (bytecode.m_nData == KEY_ELSEDEF)//"�����" ��������� ��������������
			{
				//������������ �� ������ ��������� ���������
				m_nCurPos = bytecode.m_nNumberString;//����� �� ��������� ���������� ��������
				break;
			}
			else if (bytecode.m_nData == KEY_REGION)
			{
				if (!IsWord())
					SetError(ERROR_IDENTIFIER_REGION, m_nCurPos);

				/*wxString sName = */GetWord();

				PrepareFromCurrent(LEXEM_ADD);

				if (!IsWord())
					SetError(ERROR_USE_ENDREGION, m_nCurPos);

				wxString sWord = GetWord();

				//������� #endregion
				if (IsKeyWord(sWord) != KEY_ENDREGION)
					SetError(ERROR_USE_ENDREGION, m_nCurPos);

				continue;
			}
			else if (bytecode.m_nData == KEY_ENDREGION)
			{
				m_nCurPos = bytecode.m_nNumberString;//����� �� ��������� ���������� ��������
				break;
			}
		}

		m_aLexemList.push_back(bytecode);
	}

	CLexem bytecode;
	bytecode.m_nType = ENDPROGRAM;
	bytecode.m_nData = 0;
	bytecode.m_nNumberString = m_nCurPos;
	m_aLexemList.push_back(bytecode);

	return true;
}

CDefList::~CDefList()
{
	DefList.clear();
}

void CDefList::RemoveDef(const wxString &sName)
{
	DefList.erase(StringUtils::MakeUpper(sName));
}

bool CDefList::HasDef(const wxString &sName)
{
	wxString m_csName = StringUtils::MakeUpper(sName);
	auto m_find_val = DefList.find(m_csName);
	if (m_find_val != DefList.end()) return true;

	static int nLevel = 0;

	nLevel++;

	if (nLevel > MAX_OBJECTS_LEVEL)
	{
		CTranslateError::Error("����������� ����� ������� (#3)");
	}

	//���� � ���������
	bool bRes = false;
	if (pParent) bRes = pParent->HasDef(sName);
	nLevel--;
	return bRes;
}

CLexemList *CDefList::GetDef(const wxString &sName)
{
	auto itDefList = DefList.find(StringUtils::MakeUpper(sName));
	if (itDefList != DefList.end()) return itDefList->second;

	//���� � ���������
	if (pParent && pParent->HasDef(sName))
		return pParent->GetDef(sName);

	CLexemList *m_lexList = new CLexemList();
	DefList[StringUtils::MakeUpper(sName)] = m_lexList;
	return m_lexList;
}

void CDefList::SetDef(const wxString &sName, CLexemList *pDef)
{
	CLexemList *pList = GetDef(sName);

	pList->clear();

	if (pDef)
	{
		for (unsigned int i = 0; i < pDef->size(); i++)
		{
			pList->push_back(*pDef[i].data());
		}
	}
}

void CDefList::SetDef(const wxString &sName, wxString sValue)
{
	CLexemList List;
	if (sValue.length() > 0)
	{
		CLexem Lex;
		Lex.m_nType = CONSTANT;
		if (sValue[0] == '-' || sValue[0] == '+' || (sValue[0] >= '0'&&sValue[0] <= '9'))//�����
			Lex.m_vData.SetNumber(sValue);
		else
			Lex.m_vData.SetString(sValue);
		List.push_back(Lex);
		SetDef(StringUtils::MakeUpper(sName), &List);
	}
	else
	{
		SetDef(StringUtils::MakeUpper(sName), NULL);
	}
}

void CDefList::SetParent(CDefList *p)
{
	pParent = p;
}

/**
 * �������� ������ ������� � ������� �������
 */
void CTranslateModule::PrepareFromCurrent(int nMode, const wxString &sName)
{
	CTranslateModule translate;
	translate.m_aDefList = m_aDefList;
	translate.m_nModePreparing = nMode;
	translate.m_sModuleName = m_sModuleName;
	translate.Load(m_sBuffer);

	//��������� ����� ������
	translate.m_nCurLine = m_nCurLine;
	translate.m_nCurPos = m_nCurPos;

	//�������� ����� ������
	if (nMode == LEXEM_ADDDEF)
	{
		GetStrToEndLine();
		translate.m_nSizeText = m_nCurPos;
	}

	translate.PrepareLexem();

	if (nMode == LEXEM_ADDDEF)
	{
		m_aDefList->SetDef(sName, &translate.m_aLexemList);
		m_nCurLine = translate.m_nCurLine;
	}
	else if (nMode == LEXEM_ADD)
	{
		for (unsigned int i = 0; i < translate.m_aLexemList.size() - 1; i++) {//��� ����� ENDPROGRAM
			m_aLexemList.push_back(translate.m_aLexemList[i]);
		}

		m_nCurPos = translate.m_nCurPos;
		m_nCurLine = translate.m_nCurLine;
	}
	else
	{
		m_nCurPos = translate.m_nCurPos;
		m_nCurLine = translate.m_nCurLine;
	}
}

void CTranslateModule::OnSetParent(CTranslateModule *pSetParent)
{
	if (!m_aDefList)
	{
		m_aDefList = new CDefList();
		m_aDefList->SetParent(&glDefList);

		m_bAutoDeleteDefList = true;//������� ������������
	}

	if (pSetParent) m_aDefList->pParent = pSetParent->m_aDefList;
	else m_aDefList->pParent = &glDefList;
}

class CAutoLoader
{
public:
	//����� ���������
	CAutoLoader() { CTranslateModule::LoadKeyWords(); };

	//���������� ���������
	~CAutoLoader() {}

} m_autoLoader;