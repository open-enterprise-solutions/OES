#include "autoComplectionCompile.h"
#include "compiler/definition.h"
#include "compiler/systemObjects.h"
#include "utils/stringutils.h"

//////////////////////////////////////////////////////////////////////
// CPrecompileContext CPrecompileContext CPrecompileContext CPrecompileContext  //
//////////////////////////////////////////////////////////////////////

/**
 * ����� ���������� � ��� �������
 * ���������� 1 - ���� ���������� �������
 */
bool CPrecompileContext::FindVariable(const wxString &sName, CValue &vContext, bool bContext)
{
	if (bContext)
	{
		auto itFounded = cVariables.find(StringUtils::MakeUpper(sName));
		if (itFounded != cVariables.end())
		{
			vContext = itFounded->second.vContext;
			return itFounded->second.bContext;
		}
		return false;
	}
	else
	{
		return cVariables.find(StringUtils::MakeUpper(sName)) != cVariables.end();
	}
}

/**
 * ����� ���������� � ��� �������
 * ���������� 1 - ���� ���������� �������
 */
bool CPrecompileContext::FindFunction(const wxString &sName, CValue &vContext, bool bContext)
{
	if (bContext)
	{
		auto itFounded = cFunctions.find(StringUtils::MakeUpper(sName));
		if (itFounded != cFunctions.end() && itFounded->second)
		{
			vContext = itFounded->second->vContext;
			return itFounded->second->bContext;
		}
		return false;
	}
	else
	{
		return cFunctions.find(StringUtils::MakeUpper(sName)) != cFunctions.end();
	}
}

 void CPrecompileContext::RemoveVariable(const wxString &sName)
 {
	 auto itFounded = cVariables.find(StringUtils::MakeUpper(sName));
	 if (itFounded != cVariables.end())
	 {
		 cVariables.erase(itFounded);
	 }
 }

/**
 * ��������� ����� ���������� � ������
 * ���������� ����������� ���������� � ���� SParamValue
 */
SParamValue CPrecompileContext::AddVariable(const wxString &sName, const wxString &sType, bool bExport, bool bTempVar, CValue Value)
{
	if (FindVariable(sName))//���� ���������� + ��������� ���������� = ������
		return SParamValue();

	CPrecompileVariable CurrentVar;
	CurrentVar.bContext = false;
	CurrentVar.sName = StringUtils::MakeUpper(sName);
	CurrentVar.sRealName = sName;
	CurrentVar.bExport = bExport;
	CurrentVar.bTempVar = bTempVar;
	CurrentVar.sType = sType;
	CurrentVar.vObject = Value;
	CurrentVar.nNumber = cVariables.size();

	cVariables[StringUtils::MakeUpper(sName)] = CurrentVar;

	SParamValue Ret;
	Ret.sType = sType;
	Ret.vObject = Value;
	return Ret;
}

/**
 * ������� ���������� ����� ���������� �� ���������� �����
 * ����� ����������� ����������, ������� � �������� ��������� �� ���� ������������
 * ���� ��������� ���������� ���, �� ��������� ����� ����������� ����������
 */
SParamValue CPrecompileContext::GetVariable(const wxString &sName, bool bFindInParent, bool bCheckError, CValue Value)
{
	int nCanUseLocalInParent = nFindLocalInParent;
	
	SParamValue Variable;
	Variable.sName = StringUtils::MakeUpper(sName);

	if (!FindVariable(sName))
	{
		if (bFindInParent)//���� � ������������ ����������(�������)
		{
			int nParentNumber = 0;
			CPrecompileContext *pCurContext = pParent;
			CPrecompileContext *pNotParent = pStopParent;
			while (pCurContext)
			{
				nParentNumber++;
				if (nParentNumber > MAX_OBJECTS_LEVEL)
				{
					CSystemObjects::Message(pCurContext->pModule->GetModuleName());
					if (nParentNumber > 2 * MAX_OBJECTS_LEVEL) break;
				}

				if (pCurContext == pNotParent)//������� ������ != ����������� �����������
				{
					//��������� ��������� ��������
					pNotParent = pCurContext->pParent;
					if (pNotParent == pContinueParent)//������� �� ���������� - ��� ����������� ���������
						pNotParent = NULL;
				}
				else
				{
					if (pCurContext->FindVariable(sName))//�����
					{
						CPrecompileVariable CurrentVar = pCurContext->cVariables[StringUtils::MakeUpper(sName)];
						//������� ��� ���������� ���������� ��� ��� (���� nFindLocalInParent=true, �� ����� ����� ��������� ���������� ��������)
						if (nCanUseLocalInParent > 0 || CurrentVar.bExport)
						{
							//���������� ����� ����������
							Variable.sType = CurrentVar.sType;
							Variable.vObject = CurrentVar.vObject;

							return Variable;
						}
					}
				}
				nCanUseLocalInParent--;
				pCurContext = pCurContext->pParent;
			}
		}

		if (bCheckError) return Variable;

		bool bTempVar = sName.Left(1) == "@";
		//�� ���� ��� ���������� ���������� - ���������
		AddVariable(sName, wxEmptyString, false, bTempVar, Value);
	}

	//���������� ����� � ��� ����������
	CPrecompileVariable CurrentVar = cVariables[StringUtils::MakeUpper(sName)];

	Variable.sType = CurrentVar.sType;
	Variable.vObject = CurrentVar.vObject;

	return Variable;
}

CPrecompileContext::~CPrecompileContext()
{
	for (auto it = cFunctions.begin(); it != cFunctions.end(); it++)
	{
		CPrecompileFunction *pFunction = (CPrecompileFunction *)it->second;
		if (pFunction)
			delete pFunction;
	}
	cFunctions.clear();
}