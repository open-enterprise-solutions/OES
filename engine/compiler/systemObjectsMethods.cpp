////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : system objects 
////////////////////////////////////////////////////////////////////////////

#include "systemObjects.h"
#include "methods.h"
#include "appData.h"

CMethods CSystemObjects::m_methods;

enum
{
	//--- �������:
	enBoolean = 0,
	enNumber,
	enDate,
	enString,
	//--- ��������������:
	enRound,
	enInt,
	enLog10,
	enLn,
	enMax,
	enMin,
	//--- ���������:
	enStrLen,
	enIsBlankString,
	enTrimL,
	enTrimR,
	enTrimAll,
	enLeft,
	enRight,
	enMid,
	enFind,
	enStrReplace,
	enStrCountOccur,
	enStrLineCount,
	enStrGetLine,
	enUpper,
	enLower,
	enChr,
	enAsc,
	//--- ������ � ����� � ��������:
	enCurrentDate,
	enCurrentTime,
	enWorkingDate,
	enAddMonth,
	enBegOfMonth,
	enEndOfMonth,
	enBegOfQuart,
	enEndOfQuart,
	enBegOfYear,
	enEndOfYear,
	enBegOfWeek,
	enEndOfWeek,
	enGetYear,
	enGetMonth,
	enGetDay,
	enGetHour,
	enGetMinute,
	enGetSecond,
	enGetWeekOfYear,
	enGetDayOfYear,
	enGetDayOfWeek,
	enGetQuartOfYear,
	enPeriodStr,
	//--- �����������:
	enMessage,
	enAlert,
	enQuestion,
	enSetStatus,
	enClearMessages,
	enSetError,
	enRaise,
	enErrorDescription,
	enIsEmptyValue,
	enEvaluate,
	enExecute,
	enFormat,
	enType,
	enTypeOf,
	enRand,
	enArgCount,
	enArgValue,
	enComputerName,
	enRunApp,
	enSetAppTitle,
	enUserDir,
	enUserName,
	enUserPassword,
	enSingleMode,
	enTempFilesDir,
	enGeneralLanguage,
	enEndJob,
	enUserInterruptProcessing, 
	enGetCommonForm,
	enShowCommonForm,
	enBeginTransaction,
	enCommitTransaction,
	enRollBackTransaction
};

void CSystemObjects::PrepareNames() const
{
	std::vector<SEng> aMethods = {
		//--- �������:
		{"boolean", "boolean(value)"},
		{"number", "number(value)"},
		{"date", "date(value)"},
		{"string", "string(value)"},
		//--- ��������������:
		{"round", "round(number, number, roundMode)"},
		{"int", "int(number)"},
		{"log10", "log10(number)"},
		{"ln", "ln(number)"},
		{"max", "max(number, ...)"},
		{"min", "min(number, ...)"},
		//--- ���������:
		{"strLen", "strLen(string)"},
		{"isBlankString", "isBlankString(string)"},
		{"trimL", "trimL(string)"},
		{"trimR", "trimR(string)"},
		{"trimAll", "trimAll(string)"},
		{"left", "left(string, number)"},
		{"right", "right(string, number)"},
		{"mid", "mid(string, number, number)"},
		{"find", "find(string, string, number)"},
		{"strReplace", "strReplace(string, string, string)"},
		{"strCountOccur", "strCountOccur(string, string)"},
		{"strLineCount", "strLineCount(string)"},
		{"strGetLine", "strGetLine(string)"},
		{"upper", "upper(string)"},
		{"lower", "lower(string)"},
		{"chr", "chr(number)"},
		{"asc", "asc(string)"},
		//--- ������ � ����� � ��������:
		{"currentDate", "currentDate()"},
		{"currentTime", "currentTime()"},
		{"workingDate", "workingDate(date)"},
		{"addMonth", "addMonth(date, number)"},
		{"begOfMonth", "begOfMonth(date)"},
		{"endOfMonth", "endOfMonth(date)"},
		{"begOfQuart", "begOfQuart(date)"},
		{"endOfQuart", "endOfQuart(date)"},
		{"begOfYear", "begOfYear(date)"},
		{"endOfYear", "endOfYear(date)"},
		{"begOfWeek", "begOfWeek(date)"},
		{"endOfWeek", "endOfWeek(date)"},
		{"getYear", "getYear(date)"},
		{"getMonth", "getMonth(date)"},
		{"getDay", "getDay(date)"},
		{"getHour", "getHour(date)"},
		{"getMinute", "getMinute(date)"},
		{"getSecond", "getSecond(date)"},
		{"getWeekOfYear", "getWeekOfYear(date)"},
		{"getDayOfYear", "getDayOfYear(date)"},
		{"getDayOfWeek", "getDayOfWeek(date)"},
		{"getQuartOfYear", "getQuartOfYear(date)"},
		{"periodStr", "periodStr(date, date)"},
		//--- �����������:
		{"message", "message(string, statusMessage)"},
		{"alert", "alert(string)"},
		{"question", "question(string, questionMode)"},
		{"setStatus", "setStatus(string)"},
		{"clearMessages", "clearMessages()"},
		{"setError", "setError(string)"},
		{"raise", "raise(string)"},
		{"errorDescription", "errorDescription()"},
		{"isEmptyValue", "isEmptyValue(value)"},
		{"evaluate", "evaluate(string)"},
		{"execute", "execute(string)"},
		{"format", "format(value, string)"},
		{"type", "type(string)"},
		{"typeOf", "typeOf(value)"},
		{"rand", "rand()"},
		{"argCount", "argCount()"},
		{"argValue", "argValue()"},
		{"computerName", "computerName()"},
		{"runApp", "runApp(string)"},
		{"setAppTitle", "setAppTitle(string)"},
		{"userDir", "userDir()"},
		{"userName", "userName()"},
		{"userPassword", "userPassword()"},
		{"singleMode", "singleMode()"},
		{"tempFilesDir", "tempFilesDir()"},
		{"generalLanguage", "generalLanguage()"},
		{"endJob", "endJob(boolean)"},

		{"userInterruptProcessing", "userInterruptProcessing()"},

		{"getCommonForm", "getCommonForm(string)"},
		{"showCommonForm", "showCommonForm(string)"},

		{"beginTransaction", "beginTransaction()"},
		{"commitTransaction", "commitTransaction()"},
		{"rollBackTransaction", "rollBackTransaction()"}
	};

	m_methods.PrepareMethods(aMethods.data(), aMethods.size());
};

#include "enum.h"

CValue CSystemObjects::Method(CMethodParameters &aParams)
{
	if (!appData->IsDesignerMode())
	{
		switch (aParams.GetMethodIndex())
		{
			//--- �������:
		case enBoolean: return Boolean(aParams[0]);
		case enNumber: return Number(aParams[0]);
		case enDate: return Date(aParams[0]);
		case enString: return String(aParams[0]);
			//--- ��������������:
		case enRound: return Round(aParams[0], aParams.GetParamCount() > 1 ? aParams[1].ToInt() : 0, aParams.GetParamCount() > 2 ? aParams[2].ConvertToEnumValue<eRoundMode>() : eRoundMode::eRoundMode_Round15as20);
		case enInt: return Int(aParams[0]);
		case enLog10: return Log10(aParams[0]);
		case enLn: return Ln(aParams[0]);
		case enMax: return Max(aParams.GetParams());
		case enMin: return Min(aParams.GetParams());
			//--- ���������:
		case enStrLen: return StrLen(aParams[0]);
		case enIsBlankString: return IsBlankString(aParams[0]);
		case enTrimL: return TrimL(aParams[0]);
		case enTrimR: return TrimR(aParams[0]);
		case enTrimAll: return TrimAll(aParams[0]);
		case enLeft: return Left(aParams[0], aParams[1].ToInt());
		case enRight: return Right(aParams[0], aParams[1].ToInt());
		case enMid: return Mid(aParams[0], aParams[1].ToInt(), aParams.GetParamCount() > 1 ? aParams[2].ToInt() : 1);
		case enFind: return Find(aParams[0], aParams[1], aParams.GetParamCount() > 1 ? aParams[2].ToInt() : 0);
		case enStrReplace: return StrReplace(aParams[0], aParams[1], aParams[2]);
		case enStrCountOccur: return StrCountOccur(aParams[0], aParams[1]);
		case enStrLineCount: return StrLineCount(aParams[0]);
		case enStrGetLine: return StrGetLine(aParams[0], aParams[1].ToInt());
		case enUpper: return Upper(aParams[0]);
		case enLower: return Lower(aParams[0]);
		case enChr: return Chr(aParams[0].ToInt());
		case enAsc: return Asc(aParams[0]);
			//--- ������ � ����� � ��������:
		case enCurrentDate: return CurrentDate();
		case enCurrentTime: return CurrentTime();
		case enWorkingDate: return WorkingDate();
		case enAddMonth: return AddMonth(aParams[0], aParams[1].ToInt());
		case enBegOfMonth: return BegOfMonth(aParams[0]);
		case enEndOfMonth: return EndOfMonth(aParams[0]);
		case enBegOfQuart: return BegOfQuart(aParams[0]);
		case enEndOfQuart: return EndOfQuart(aParams[0]);
		case enBegOfYear: return BegOfYear(aParams[0]);
		case enEndOfYear: return EndOfYear(aParams[0]);
		case enBegOfWeek: return BegOfWeek(aParams[0]);
		case enEndOfWeek: return EndOfWeek(aParams[0]);
		case enGetYear: return GetYear(aParams[0]);
		case enGetMonth: return GetMonth(aParams[0]);
		case enGetDay: return GetDay(aParams[0]);
		case enGetHour: return GetHour(aParams[0]);
		case enGetMinute: return GetMinute(aParams[0]);
		case enGetSecond: return GetSecond(aParams[0]);
		case enGetWeekOfYear: return GetWeekOfYear(aParams[0]);
		case enGetDayOfYear: return GetDayOfYear(aParams[0]);
		case enGetDayOfWeek: return GetDayOfWeek(aParams[0]);
		case enGetQuartOfYear: return GetQuartOfYear(aParams[0]);
		case enPeriodStr: return PeriodStr(aParams[0], aParams[1]);
			//--- �����������:
		case enMessage: Message(aParams[0].ToString(), aParams.GetParamCount() > 1 ? aParams[1].ConvertToEnumValue<eStatusMessage>() : eStatusMessage::eStatusMessage_Information); break;
		case enAlert: Alert(aParams[0].ToString()); break;
		case enQuestion: return Question(aParams[0].ToString(), aParams[1].ConvertToEnumValue<eQuestionMode>());
		case enSetStatus: SetStatus(aParams[0].ToString()); break;
		case enClearMessages: ClearMessages(); break;
		case enSetError: SetError(aParams[0].ToString()); break;
		case enRaise: Raise(aParams[0].ToString()); break;
		case enErrorDescription: return ErrorDescription();
		case enIsEmptyValue: return IsEmptyValue(aParams[0]);
		case enEvaluate: return Evaluate(aParams[0].ToString());
		case enExecute: Execute(aParams[0].ToString()); break;
		case enFormat: return Format(aParams[0], aParams[1].ToString());
		case enType: return Type(aParams[0]);
		case enTypeOf: return TypeOf(aParams[0]);
		case enRand: return Rand();
		case enArgCount: return ArgCount();
		case enArgValue: return ArgValue(aParams[0].ToInt());
		case enComputerName: return ComputerName();
		case enRunApp: RunApp(aParams[0].ToString()); break;
		case enSetAppTitle: SetAppTitle(aParams[0].ToString());
		case enUserDir: return UserDir();
		case enUserName: return UserName();
		case enUserPassword: return UserPassword();
		case enSingleMode: return SingleMode();
		case enTempFilesDir: return TempFilesDir();
		case enGeneralLanguage: return GeneralLanguage();
		case enEndJob: EndJob(aParams[0].ToInt()); break;
		case enUserInterruptProcessing: UserInterruptProcessing(); break;
		case enGetCommonForm: return GetCommonForm(aParams[0].ToString());
		case enShowCommonForm: ShowCommonForm(aParams[0].ToString()); break;
		case enBeginTransaction: BeginTransaction(); break;
		case enCommitTransaction: CommitTransaction(); break;
		case enRollBackTransaction: RollBackTransaction(); break;
		}
	}
	else
	{
		switch (aParams.GetMethodIndex())
		{
			//--- �����������:
			case enType: return Type(aParams[0]);
			case enTypeOf: return TypeOf(aParams[0]);

		    case enGetCommonForm: return GetCommonForm(aParams[0].ToString());
			case enShowCommonForm: ShowCommonForm(aParams[0].ToString()); break;
		}
	}

	return new CValueNoRet(aParams.GetMethodName());
}