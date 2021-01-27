////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : parser for autocomplete 
////////////////////////////////////////////////////////////////////////////

#include "autoComplectionParser.h"
#include "compiler/definition.h"

#pragma warning(disable : 4018)

CParserModule::CParserModule() : CTranslateModule(), m_nCurrentCompile(wxNOT_FOUND)
{
}

bool CParserModule::ParseModule(const wxString &sModule)
{
	m_aContentModule.clear();

	//1. ����� ��������� �� �������
	Load(sModule);

	try
	{
		PrepareLexem();
	}
	catch (const CTranslateError *)
	{
		return false;
	};

	CLexem lex;

	while ((lex = GETLexem()).m_nType != ERRORTYPE)
	{
		//���������� �������
		if (lex.m_nType == KEYWORD && lex.m_nData == KEY_IF)
		{
			while (m_nCurrentCompile + 1 < m_aLexemList.size())
			{
				lex = GETLexem();
				if (lex.m_nType == KEYWORD && lex.m_nData == KEY_THEN) break;
			}

			lex = GETLexem();
		}

		//���������� ��������� ������ WHILE
		if (lex.m_nType == KEYWORD && lex.m_nData == KEY_WHILE)
		{
			while (m_nCurrentCompile + 1 < m_aLexemList.size())
			{
				lex = GETLexem();
				if (lex.m_nType == KEYWORD && lex.m_nData == KEY_DO) break;
			}
			lex = GETLexem();
		}

		//���������� ��������� ���������
		if (lex.m_nType == DELIMITER && lex.m_nData == '?')
		{
			while (m_nCurrentCompile + 1 < m_aLexemList.size())
			{
				lex = GETLexem();
				if (lex.m_nType == DELIMITER && lex.m_nData == ';') break;
			}
			lex = GETLexem();
		}

		//���������� ����������
		if (lex.m_nType == KEYWORD && lex.m_nData == KEY_VAR)
		{
			while (m_nCurrentCompile + 1 < m_aLexemList.size())
			{
				wxString sName = GETIdentifier(true);

				int nArrayCount = -1;
				if (IsNextDelimeter('['))//��� ���������� �������
				{
					nArrayCount = 0;

					GETDelimeter('[');
					if (!IsNextDelimeter(']'))
					{
						CValue vConst = GETConstant();
						if (vConst.GetType() != eValueTypes::TYPE_NUMBER || vConst.GetNumber() < 0) continue;
						nArrayCount = vConst.ToInt();
					}
					GETDelimeter(']');
				}

				bool bExport = false;

				if (IsNextKeyWord(KEY_EXPORT))
				{
					GETKeyWord(KEY_EXPORT);
					bExport = true;
				}

				if (IsNextDelimeter('='))//��������� ������������� - �������� ������ ������ ������ ������� (�� �� ���� ������. �������� � �������)
				{
					if (nArrayCount >= 0) GETDelimeter(',');//Error!
					GETDelimeter('=');
				}

				CModuleElementInfo data;
				data.sName = sName;
				data.nLine = lex.m_nNumberLine;
				data.nImage = 358;

				if (bExport) data.eType = eContentType::eExportVariable;
				else data.eType = eContentType::eVariable;

				m_aContentModule.push_back(data);

				if (!IsNextDelimeter(',')) break;
				GETDelimeter(',');
			}
		}

		//���������� ������� � �������� 
		if (lex.m_nType == KEYWORD && (lex.m_nData == KEY_FUNCTION || lex.m_nData == KEY_PROCEDURE))
		{
			bool isFunction = lex.m_nData == KEY_FUNCTION;

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

			wxString sFuncName = GETIdentifier(true);

			//����������� ������ ���������� ���������� + ������������ �� ��� ���������
			GETDelimeter('(');
			if (!IsNextDelimeter(')'))
			{
				while (m_nCurrentCompile + 1 < m_aLexemList.size())
				{
					if (IsNextKeyWord(KEY_VAL))
					{
						GETKeyWord(KEY_VAL);
					}

					/*wxString sName =*/ (void)GETIdentifier(true);

					if (IsNextDelimeter('['))//��� ������
					{
						GETDelimeter('[');
						GETDelimeter(']');
					}
					else if (IsNextDelimeter('='))
					{
						GETDelimeter('=');
						CValue vConstant = GETConstant();
					}

					if (IsNextDelimeter(')')) break;

					GETDelimeter(',');
				}
			}

			GETDelimeter(')');

			bool bExport = false;

			if (IsNextKeyWord(KEY_EXPORT))
			{
				GETKeyWord(KEY_EXPORT); bExport = true;
			}

			CModuleElementInfo data;
			data.sName = sFuncName;
			data.sShortDescription = sShortDescription;
			data.nLine = lex.m_nNumberLine;

			if (isFunction)
			{
				data.nImage = 353;

				if (bExport) data.eType = eContentType::eExportFunction;
				else data.eType = eContentType::eFunction;
			}
			else
			{
				data.nImage = 352;

				if (bExport) data.eType = eContentType::eExportProcedure;
				else data.eType = eContentType::eProcedure;
			}

			m_aContentModule.push_back(data);

			lex = GETLexem();
		}
	}

	if (m_nCurrentCompile + 1 < m_aLexemList.size() - 1) return false;
	return true;
}

//variables
wxArrayString CParserModule::GetVariables(bool bOnlyExport)
{
	wxArrayString aVariables;

	for (auto code : m_aContentModule)
	{
		if (bOnlyExport && code.eType == eContentType::eExportVariable)
		{
			aVariables.push_back(code.sName);
		}
		else if (!bOnlyExport && (code.eType == eContentType::eExportVariable || code.eType == eContentType::eVariable))
		{
			aVariables.push_back(code.sName);
		}
	}

	return aVariables;
}

//functions & procedures 
wxArrayString CParserModule::GetFunctions(bool bOnlyExport)
{
	wxArrayString aFunctions;

	for (auto code : m_aContentModule)
	{
		if (bOnlyExport && code.eType == eContentType::eExportFunction)
		{
			aFunctions.push_back(code.sName);
		}
		else if (!bOnlyExport && (code.eType == eContentType::eExportFunction || code.eType == eContentType::eFunction))
		{
			aFunctions.push_back(code.sName);
		}
	}

	return aFunctions;
}

wxArrayString CParserModule::GetProcedures(bool bOnlyExport)
{
	wxArrayString aProcedures;

	for (auto code : m_aContentModule)
	{
		if (bOnlyExport && code.eType == eContentType::eExportProcedure)
		{
			aProcedures.push_back(code.sName);
		}
		else if (!bOnlyExport && (code.eType == eContentType::eExportProcedure || code.eType == eContentType::eProcedure))
		{
			aProcedures.push_back(code.sName);
		}
	}

	return aProcedures;
}

/**
 * GetLexem
 * ����������:
 * �������� ��������� ������� �� ������ ���� ���� � �������� ������� ������� ������� �� 1
 * ������������ ��������:
 * 0 ��� ��������� �� �������
 */
CLexem CParserModule::GetLexem()
{
	CLexem lex;
	if (m_nCurrentCompile + 1 < m_aLexemList.size())
	{
		lex = m_aLexemList[++m_nCurrentCompile];
	}
	return lex;
}

//�������� ��������� ������� �� ������ ���� ���� ��� ���������� �������� ������� �������
CLexem CParserModule::PreviewGetLexem()
{
	CLexem lex;
	while (true)
	{
		lex = GetLexem();
		if (!(lex.m_nType == DELIMITER && (lex.m_nData == ';' || lex.m_nData == '\n')))
			break;
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
CLexem CParserModule::GETLexem()
{
	CLexem lex = GetLexem();
	if (lex.m_nType == ERRORTYPE) {}
	return lex;
}
/**
 * GETDelimeter
 * ����������:
 * �������� ��������� ������� ��� �������� �����������
 * ������������ ��������:
 * ��� (� ������ ������� ��������� ����������)
 */
void CParserModule::GETDelimeter(char c)
{
	CLexem lex = GETLexem();
	while (!(lex.m_nType == DELIMITER && lex.m_nData == c)) {
		if (m_nCurrentCompile + 1 >= m_aLexemList.size()) break;
		lex = GETLexem();
	}
}
/**
 * IsNextDelimeter
 * ����������:
 * ��������� �������� �� ��������� ������� ����-���� �������� ������������
 * ������������ ��������:
 * true,false
 */
bool CParserModule::IsNextDelimeter(char c)
{
	if (m_nCurrentCompile + 1 < m_aLexemList.size())
	{
		CLexem lex = m_aLexemList[m_nCurrentCompile + 1];
		if (lex.m_nType == DELIMITER && lex.m_nData == c) return true;
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
bool CParserModule::IsNextKeyWord(int nKey)
{
	if (m_nCurrentCompile + 1 < m_aLexemList.size())
	{
		CLexem lex = m_aLexemList[m_nCurrentCompile + 1];
		if (lex.m_nType == KEYWORD && lex.m_nData == nKey) return true;
	}
	return false;
}

/**
 * GETKeyWord
 * �������� ��������� ������� ��� �������� �������� �����
 * ������������ ��������:
 * ��� (� ������ ������� ��������� ����������)
 */
void CParserModule::GETKeyWord(int nKey)
{
	CLexem lex = GETLexem();
	while (!(lex.m_nType == KEYWORD && lex.m_nData == nKey)) {
		if (m_nCurrentCompile + 1 >= m_aLexemList.size()) break;
		lex = GETLexem();
	}
}

/**
 * GETIdentifier
 * �������� ��������� ������� ��� �������� �������� �����
 * ������������ ��������:
 * ������-�������������
 */
wxString CParserModule::GETIdentifier(bool realName)
{
	CLexem lex = GETLexem();

	if (lex.m_nType != IDENTIFIER)
	{
		if (realName && lex.m_nType == KEYWORD) return lex.m_sData;
		return wxEmptyString;
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
CValue CParserModule::GETConstant()
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

	if (lex.m_nType != CONSTANT) return lex.m_vData;

	if (iNumRequire)
	{
		//�������� �� �� ����� ��������� ����� �������� ���	
		if (lex.m_vData.GetType() != eValueTypes::TYPE_NUMBER) { return lex.m_vData; }
		//������ ���� ��� ������
		if (iNumRequire == -1) lex.m_vData.m_fData = -lex.m_vData.m_fData;
	}
	return lex.m_vData;
}

