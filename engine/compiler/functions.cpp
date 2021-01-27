////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, 2С-team
//	Description : translate error and exception handler 
////////////////////////////////////////////////////////////////////////////

#include "functions.h"
#include "definition.h"
#include "systemObjects.h"
#include "compileModule.h"
#include "valueArray.h"
#include "valueMap.h"
#include "procUnit.h"

#include "metadata/metadata.h"
#include "window/mainFrame.h"
#include "window/errorDialog.h"
#include "debugger/debugClient.h"
#include "utils/stringutils.h"

wxString CTranslateError::sCurError;

//////////////////////////////////////////////////////////////////////
//Constant:Список сообщений об ошибках
//////////////////////////////////////////////////////////////////////
wxString aErrors[] =
{
	"Usage: %s <filename>",
	"Error reading file %s",
	"Error opening file %s",
	"ASSERT: Module system error %s in line %d",
	"ASSERT_VALID: Module system error %s in line %d",
	"System error(out of array) when trying to process an error with a number %d",
	"Symbol expected :\n%s",
	"Word expected :\n%s",
	"Constant boolean expected :\n%s",
	"Constant number expected  :\n%s",
	"Constant string expected :\n%s",
	"Constant date expected :\n%s",
	"%s\nDuplicate identifier %s",//ERROR_IDENTIFIER_DUPLICATE
	"%s\nLabel '%s' not defined",
	"%s\nOne of the keywords is expected!",
	"%s\nModule code expected",
	"%s\nIdentifier expected",//ERROR_IDENTIFIER_DEFINE
	"%s\nRegion name expected",//ERROR_IDENTIFIER_REGION
	"%s\nKeyword or identifier expected",//ERROR_CODE
	"%s\nSymbol expected '%s'",//ERROR_DELIMETER
	"%s\nClosing parenthesis or comma expected",//ERROR_FUNC_DELIMETER
	"%s\nFunction or procedure declaration keyword expected",//ERROR_FUNC_DEFINE
	"%s\nModule cannot have a return statement",//ERROR_RETURN
	"%s\nExpected constant",//ERROR_CONST_DEFINE
	"%s\nAn operator is expected to complete a procedure or function!",//ERROR_ENDFUNC_DEFINE
	"%s\nError writing file: %s",//ERROR_FILE_WRITE
	"%s\nError in expression:\n%s",//ERROR_EXPRESSION
	"%s\nKeyword expected %s",//ERROR_KEYWORD
	"%s\nIdentifier '%s' not found",//ERROR_IDENTIFIER_NOT_FOUND
	"%s\nOperator Break can be used only inside the cycle",//ERROR_USE_BREAK
	"%s\nOperator Continue can be used only inside the cycle",//ERROR_USE_CONTINUE
	"%s\nOperator Return cannot be used outside the procedure or function",//ERROR_USE_RETURN
	"%s\nExpected program operators",//ERROR_USE_BLOCK
	"%s\nExpected expression",//ERROR_EXPRESSION_REQUIRE
	"%s\nProcedure or function not detected (%s)",//ERROR_CALL_FUNCTION
	"%s\nVariable with the specified name is already defined (%s)",//ERROR_DEF_VARIABLE
	"%s\nA procedure or function with the specified name is already defined (%s)",//ERROR_DEF_FUNCTION
	"%s\nToo many parameters",//ERROR_MANY_PARAMS
	"%s\nNot enough parameters",//ERROR_FEW_PARAMS
	"%s\nVar is not found (%s)",//ERROR_VAR_NOT_FOUND
	"%s\nUnexpected program code termination",//ERROR_END_PROGRAM
	"%s\nThis module may contain only definitions of procedures and functions", //ERROR_ONLY_FUNCTION
	"%s\nUse procedure as function (%s)", //ERROR_USE_PROCEDURE_AS_FUNCTION
	"%s\nExpected integer positive sign constant",//ERROR_ARRAY_SIZE_CONST
	"%s\nRe-import the parent module",//ERROR_DUBLICATE_IMPORT
	"%s\nModule not found",//ERROR_MODULE_NOT_FOUND
	"%s\nThe import statement must be at the beginning of the module",//ERROR_USE_IMPORT
	"%s\nFinal conditional compilation statement expected",//ERROR_USE_ENDDEF
	"%s\nA pending region statement is expected",//ERROR_USE_ENDREGION

	"%s\nConstructor not found (%s)",//ERROR_CALL_CONSTRUCTOR

	"%s\nType error define",//ERROR_TYPE_DEF
	"%s\nBad variable type",//ERROR_BAD_TYPE
	"%s\nBad value type",//ERROR_BAD_TYPE_EXPRESSION
	"%s\nVariable must be a numeric type",//ERROR_NUMBER_TYPE

	"%s\nBolean value expected",//ERROR_BAD_TYPE_EXPRESSION_B
	"%s\nNumeric value expected",//ERROR_BAD_TYPE_EXPRESSION_N
	"%s\nString value expected",//ERROR_BAD_TYPE_EXPRESSION_S
	"%s\nDate value expected",//ERROR_BAD_TYPE_EXPRESSION_D

	"%s\nVariable type does not support this operation",//ERROR_TYPE_OPERATION
};

bool CTranslateError::bSimpleMode = false;

//////////////////////////////////////////////////////////////////////
// Обработка ошибок
//////////////////////////////////////////////////////////////////////

CTranslateError::CTranslateError(const wxString &sErrorString) : std::exception(sErrorString) {}

#include <wx/evtloop.h>

void CTranslateError::ProcessError(const wxString &fileName, const wxString &moduleName, const wxString &moduleData, unsigned int line, unsigned int currPos)
{
	if (appData->IsEnterpriseMode())
	{
		wxString sErrorLine = FindErrorCodeLine(moduleData, currPos, 0);

		//open error dialog
		CErrorDialog *m_errDlg = new CErrorDialog(CMainFrame::Get(), wxID_ANY);
		m_errDlg->SetErrorMessage(moduleName + "\t" + sErrorLine + "\n" + sCurError);

		int retCode = m_errDlg->ShowModal();

		//send error to designer
		if (retCode > 0) { debugClient->SendErrorToDesigner(moduleName, line, moduleName + "\t" + sErrorLine + "\n" + sCurError); }
		//trying enter debug loop
		if (retCode > 1) { debugClient->DoDebugLoop(fileName, moduleName, line + 1, CProcUnit::GetCurrentRunContext()); }
		//close window
		if (retCode > 2) { mainFrameDestroy(); appDataDestroy(); std::exit(1); }
	}
}

void CTranslateError::Error(const wxString fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	ErrorV(fmt, list);
}

void CTranslateError::Error(int nErr, ...)
{
	va_list list;
	if (0 <= nErr && nErr < LastError)
	{
		va_start(list, nErr);
		ErrorV(aErrors[nErr], list);
	}
	else {
		list = (va_list)&nErr;
		ErrorV(aErrors[ERROR_SYS1], list);
	}
}

void CTranslateError::Error(int nErr, const wxString &s1, int d2)
{
	if (nErr >= 0 && nErr < LastError)
	{
		CTranslateError::Error(aErrors[nErr], s1.wc_str(), d2);
	}

	CTranslateError::Error(aErrors[ERROR_SYS1], nErr);
}

//служебные процедуры обработки ошибок
void CTranslateError::ErrorV(const wxString &fmt, va_list &list)
{
	wxString sErrorBuffer = wxString::FormatV(fmt, list); va_end(list);

	if (CTranslateError::IsSimpleMode()) sErrorBuffer.Replace("\n", "  ");

	sErrorBuffer.Trim(true);
	sErrorBuffer.Trim(false);

	sCurError = sErrorBuffer;

#ifdef _DEBUG
	wxLogDebug(sErrorBuffer);
#endif // !_DEBUG

	throw (new CTranslateError(sCurError));
}

wxString CTranslateError::FindErrorCodeLine(const wxString &sBuffer, int nCurPos, int nOnlyCodeLine)
{
	int nSizeText = sBuffer.length();

	if (nCurPos >= nSizeText) nCurPos = nSizeText - 1;
	if (nCurPos < 0) nCurPos = 0;

	int nStart = 0;

	for (int i = nCurPos; i > 0; i--)//ищем начало строки в которой выдается сообщение об ошибке трансляции
	{
		if (sBuffer[i] == '\n') { nStart = i + 1; break; };
	}

	int nEnd = nSizeText;

	for (int i = nCurPos + 1; i < nSizeText; i++)//ищем конец строки в которой выдается сообщение об ошибке трансляции
	{
		if (sBuffer[i] == '\n') { nEnd = i; break; };
	}

	//определяем номер строки
	unsigned int nNumber = 1 + sBuffer.Left(nStart).Replace('\n', '\n');

	wxString strError;

	if (nOnlyCodeLine)//это режим поиска текущей строки программы
	{
		if (nOnlyCodeLine == 1)
		{
			strError = wxString::Format("(#line %d)::%s", nNumber, sBuffer.Mid(nStart, nEnd - nStart));
			strError.Replace("\r", "");
			strError.Replace("\t", " ");
		}
		else if (nOnlyCodeLine == 2)
		{
			strError = sBuffer.Mid(nStart, nEnd - nStart);
			strError.Replace("\r", "");
			strError.Replace("\t", " ");
		}
		else if (nOnlyCodeLine == 3)
		{
			strError = wxString::Format("(#line %d):     %s", nNumber, sBuffer.Mid(nStart, nEnd - nStart));
			strError.Replace("\r", "");
			strError.Replace("\t", " ");
		}
		else
		{
			strError = wxString::Format("%s <<?>> %s", sBuffer.Mid(nStart, nCurPos - nStart), sBuffer.Mid(nCurPos, nEnd - nCurPos));
		}
	}
	else
	{
		strError = wxString::Format("(#line %d):\n%s <<?>> %s", nNumber, sBuffer.Mid(nStart, nCurPos - nStart), sBuffer.Mid(nCurPos, nEnd - nCurPos));
	}

	strError.Trim(true);
	strError.Trim(false);

	return strError;
}

/////////////////////////////////////////////////////////////////////////////
//                         Функции работы с паролями                       //
/////////////////////////////////////////////////////////////////////////////

wxString GetHash(const wxString &sPassword)
{
	wxString sHash = "", sHash2;
	for (unsigned int i = 0; i < sPassword.size(); i++)
	{
		srand(i);
		sHash2 = wxString::Format("%s%3d", sHash, (rand() % 256) ^ (char)sPassword[i] ^ 0xFF);
		sHash = sHash2;
	}
	sHash.Replace(" ", "0");
	return sHash;
}

wxString GetPassword(wxString sHash)
{
	wxString sPassword, sPassword2;
	for (unsigned int i = 0; i < sHash.size() / 3; i++)
	{
		srand(i);
		wxString str = sHash.Mid(i * 3, 3);
		byte c = wxAtoi(str);
		sPassword2 = wxString::Format("%s%c", sPassword, (rand() % 256) ^ c ^ 0xFF);
		sPassword = sPassword2;
	}
	sHash.Replace(" ", "0");
	return sPassword;
}

/////////////////////////////////////////////////////////////////////////////
//                             Разные функции                              //
/////////////////////////////////////////////////////////////////////////////

wxString GetGetLastStringError()
{
	wxString sRes;
	DWORD dwLastError = GetLastError();

	HMODULE hModule = NULL; // default to system source
	LPWSTR MessageBuffer;
	DWORD dwBufferLength;

	DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_FROM_SYSTEM;

	//
	// If dwLastError is in the network range, 
	//  load the message source.
	//

	//
	// Call FormatMessage() to allow for message 
	//  text to be acquired from the system 
	//  or from the supplied module handle.
	//

	if (dwBufferLength = FormatMessage(
		dwFormatFlags,
		hModule, // module to get message from (NULL == system)
		dwLastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
		(LPWSTR)&MessageBuffer,
		0,
		NULL
	))
	{
		sRes = MessageBuffer;

		//
		// Free the buffer allocated by the system.
		//
		LocalFree(MessageBuffer);
	}
	return sRes;
}
