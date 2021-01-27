#ifndef _FUNCTIONS_H__
#define _FUNCTIONS_H__

#include <wx/wx.h>

class CTranslateError : public std::exception
{
	static wxString sCurError;

	wxString sModuleName;
	unsigned int nLine;

	static bool bSimpleMode;

public:

	CTranslateError(const wxString &errorString = wxEmptyString);

	//error from proc unit 
	static void ProcessError(const wxString &fileName, const wxString &moduleName, const wxString &moduleData, unsigned int line, unsigned int currPos);

	//special error functions
	static void Error(const wxString fmt, ...);
	static void Error(int nErr, ...);
	static void Error(int nErr, const wxString &s1, int d2);

	//служебные процедуры обработки ошибок
	static void ErrorV(const wxString &fmt, va_list &list);

	static wxString FindErrorCodeLine(const wxString &sBuffer, int nCurPos, int nOnlyCodeLine = 0);
	static wxString GetLastError() { return sCurError; }

	static void ActivateSimpleMode() { bSimpleMode = true; }
	static void DeativateSimpleMode() { bSimpleMode = false; }

	static bool IsSimpleMode() { return bSimpleMode; }
};

class CInterruptBreak : public CTranslateError
{
public:
	CInterruptBreak() : CTranslateError(_("The program was stopped by the user!")) {}
};

#endif 