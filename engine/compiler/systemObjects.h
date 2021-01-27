#ifndef _SYSTEMOBJECTS_H__
#define _SYSTEMOBJECTS_H__

#include "value.h"

//--Константы:
#define PageBreak wxT("\n\n")
#define LineBreak wxT("\n")
#define TabSymbol wxT("\t")

#include "systemEnums.h"

class CSystemObjects : public CValue
{
	static wxDateTime m_workDate;

public:

	//--- Базовые:
	static bool Boolean(CValue &cValue);
	static number_t Number(CValue &cValue);
	static wxLongLong_t Date(CValue &cValue);
	static wxString String(CValue &cValue);

	//--- Математические:
	static number_t Round(CValue &cValue, int precision = 0, eRoundMode mode = eRoundMode::eRoundMode_Round15as20);
	static CValue Int(CValue &cNumber);
	static number_t Log10(CValue &cValue);
	static number_t Ln(CValue &cValue);
	static CValue Max(CValue **p);
	static CValue Min(CValue **p);

	//--- Строковые:
	static int StrLen(CValue &cValue);
	static bool IsBlankString(CValue &cValue);
	static wxString TrimL(CValue &cValue);
	static wxString TrimR(CValue &cValue);
	static wxString TrimAll(CValue &cValue);
	static wxString Left(CValue &cValue, unsigned int nCount);
	static wxString Right(CValue &cValue, unsigned int nCount);
	static wxString Mid(CValue &cValue, unsigned int nFirst, unsigned int nCount);
	static unsigned int Find(CValue &cValue, CValue &cValue2, unsigned int nStart);
	static wxString StrReplace(CValue &cSource, CValue &cValue1, CValue &cValue2);
	static int StrCountOccur(CValue &cSource, CValue &cValue1);
	static int StrLineCount(CValue &cSource);
	static wxString StrGetLine(CValue &cValue, unsigned int nLine);
	static wxString Upper(CValue &cSource);
	static wxString Lower(CValue &cSource);
	static wxString Chr(short nCode);
	static short Asc(CValue &cSource);

	//--- Работа с датой и временем:
	static CValue CurrentDate();
	static CValue CurrentTime();
	static CValue WorkingDate();
	static CValue AddMonth(CValue &cData, int nMonthAdd = 1);
	static CValue BegOfMonth(CValue &cData);
	static CValue EndOfMonth(CValue &cData);
	static CValue BegOfQuart(CValue &cData);
	static CValue EndOfQuart(CValue &cData);
	static CValue BegOfYear(CValue &cData);
	static CValue EndOfYear(CValue &cData);
	static CValue BegOfWeek(CValue &cData);
	static CValue EndOfWeek(CValue &cData);
	static int GetYear(CValue &cData);
	static int GetMonth(CValue &cData);
	static int GetDay(CValue &cData);
	static int GetHour(CValue &cData);
	static int GetMinute(CValue &cData);
	static int GetSecond(CValue &cData);
	static int GetWeekOfYear(CValue &cData);
	static int GetDayOfYear(CValue &cData);
	static int GetDayOfWeek(CValue &cData);
	static int GetQuartOfYear(CValue &cData);
	static wxString PeriodStr(CValue &cData1, CValue &cData2);

	//--- Специальные:
	static void Message(const wxString &sMessage, eStatusMessage status = eStatusMessage::eStatusMessage_Information);
	static void Alert(const wxString &sMessage);
	static CValue Question(const wxString &sMessage, eQuestionMode mode = eQuestionMode::eQuestionMode_OK);
	static void SetStatus(const wxString &sStatus);
	static void ClearMessages();
	static void SetError(const wxString &sError);
	static void Raise(const wxString &sError);
	static wxString ErrorDescription();
	static bool IsEmptyValue(CValue &cData);
	static CValue Evaluate(const wxString &sExpression);
	static void Execute(const wxString &sCode);
	static wxString Format(CValue &cData, wxString &fmt);
	static CValue Type(const CValue &cTypeName);
	static CValue TypeOf(const CValue &cData);
	static int Rand();
	static int ArgCount();
	static wxString ArgValue(int n);
	static wxString ComputerName();
	static void RunApp(const wxString &sCommand);
	static void SetAppTitle(const wxString &sTitle);
	static wxString UserDir();
	static wxString UserName();
	static wxString UserPassword();
	static bool SingleMode();
	static wxString TempFilesDir();
	static int GeneralLanguage();
	static void EndJob(bool force = false);

	static void UserInterruptProcessing(); 

	static CValue GetCommonForm(const wxString &sFormName);
	static void ShowCommonForm(const wxString &sFormName);

	static void BeginTransaction();
	static void CommitTransaction();
	static void RollBackTransaction();

public:

	CSystemObjects();

	//****************************************************************************
	//*                              Support methods                             *
	//****************************************************************************

	static CMethods m_methods;

	virtual CMethods* GetPMethods() const { PrepareNames(); return &m_methods; } 	// получить ссылку на класс помощник разбора имен атрибутов и методов
	virtual void PrepareNames() const;                             // этот метод автоматически вызывается для инициализации имен атрибутов и методов
	virtual CValue Method(CMethodParameters &aParams);       // вызов метода

	//check is empty
	virtual inline bool IsEmpty() const override { return false; }
};

#endif 