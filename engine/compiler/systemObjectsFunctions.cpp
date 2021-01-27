#include "systemObjects.h"
#include "spell.h"
#include "database/databaseLayer.h"
#include "window/output/outputWindow.h"
#include "forms/elements/frame.h"
#include "metadata/metaObjects/metaFormObject.h"
#include "metadata/metadata.h"
#include "utils/stringutils.h"
#include "window/mainFrame.h"
#include "translateModule.h"
#include "procUnit.h"
#include "appData.h"

#include "systemObjectsEnums.h"

//--- Базовые:
bool CSystemObjects::Boolean(CValue &cValue)
{
	return cValue.GetBoolean();
}

number_t CSystemObjects::Number(CValue &cValue)
{
	return cValue.GetNumber();
}

wxLongLong_t CSystemObjects::Date(CValue &cValue)
{
	return cValue.GetDate();
}

wxString CSystemObjects::String(CValue &cValue)
{
	return cValue.GetString();
}

//---Математические:
number_t CSystemObjects::Round(CValue &cValue, int precision, eRoundMode mode)
{
	number_t fNumber = cValue.GetNumber();

	if (precision > MAX_PRECISION) precision = MAX_PRECISION;

	ttmath::Int<TTMATH_BITS(128)> nDelta;
	if (!fNumber.ToInt(nDelta)) fNumber = fNumber - nDelta;

	number_t fTemp = 10;
	fTemp.Pow(precision + 1);
	fTemp = fTemp * fNumber;

	ttmath::Int<TTMATH_BITS(128)> N;
	fTemp.ToInt(N);

	//округление - в зависимости от метода
	if (mode == eRoundMode::eRoundMode_Round15as20)
	{
		if (N % 10 >= 5) N = N / 10 + 1;
		else N = N / 10;
	}
	else
	{
		if (N % 10 >= 6) N = N / 10 + 1;
		else N = N / 10;
	}

	number_t G = 10; G.Pow(precision);

	if (!fTemp.FromInt(N))
	{
		fTemp = fTemp / G;
		fTemp.Add(nDelta);

		return fTemp;
	}

	return 0;
}

CValue CSystemObjects::Int(CValue &cValue)
{
	ttmath::Int<TTMATH_BITS(128)> int128;
	number_t fNumber = cValue.GetNumber();
	if (!fNumber.ToInt(int128)) return int128;
	else return 0;
}

number_t CSystemObjects::Log10(CValue &cValue)
{
	number_t fNumber = cValue.GetNumber();
	return std::log10(fNumber.ToDouble());
}

number_t CSystemObjects::Ln(CValue &cValue)
{
	number_t fNumber = cValue.GetNumber();
	return std::log(fNumber.ToDouble());
}

CValue CSystemObjects::Max(CValue **aParams)
{
	int i = 1;
	CValue *pRes = aParams[0];
	while (aParams[i]->GetType() != TYPE_EMPTY) { if (aParams[i]->GetNumber() > pRes->GetNumber()) pRes = aParams[i]; i++; }
	return *pRes;
}

CValue CSystemObjects::Min(CValue **aParams)
{
	int i = 1;
	CValue *pRes = aParams[0];
	while (aParams[i]->GetType() != TYPE_EMPTY) { if (aParams[i]->GetNumber() < pRes->GetNumber()) pRes = aParams[i]; i++; }
	return *pRes;
}

//---Строковые:
int CSystemObjects::StrLen(CValue &cValue)
{
	wxString csString = cValue.GetString();
	return csString.Length();
}

bool CSystemObjects::IsBlankString(CValue &cValue)
{
	wxString csString = cValue.GetString();
	csString.Trim(true);
	csString.Trim(false);
	return csString.IsEmpty();
}

wxString CSystemObjects::TrimL(CValue &cValue)
{
	wxString csString = cValue.GetString();
	csString.Trim(false);
	return csString;
}

wxString CSystemObjects::TrimR(CValue &cValue)
{
	wxString csString = cValue.GetString();
	csString.Trim(true);
	return csString;
}

wxString CSystemObjects::TrimAll(CValue &cValue)
{
	wxString csString = cValue.GetString();
	csString.Trim(true);
	csString.Trim(false);
	return csString;
}

wxString CSystemObjects::Left(CValue &cValue, unsigned int nCount)
{
	wxString csString = cValue.GetString();
	return csString.Left(nCount);
}

wxString CSystemObjects::Right(CValue &cValue, unsigned int nCount)
{
	wxString csString = cValue.GetString();
	return csString.Right(nCount);
}

wxString CSystemObjects::Mid(CValue &cValue, unsigned int nFirst, unsigned int nCount)
{
	wxString csString = cValue.GetString();
	return csString.Mid(nFirst, nCount);
}

unsigned int CSystemObjects::Find(CValue &cValue, CValue &cValue2, unsigned int nStart)
{
	if (nStart < 1) nStart = 1;
	wxString csStr = cValue.GetString();
	return csStr.find(cValue2.GetString(), nStart - 1) + 1;
}

wxString CSystemObjects::StrReplace(CValue &cSource, CValue &cValue1, CValue &cValue2)
{
	wxString csSource = cSource.GetString();
	csSource.Replace(cValue1.GetString(), cValue2.GetString());
	return csSource;
}

int CSystemObjects::StrCountOccur(CValue &cSource, CValue &cValue1)
{
	wxString csSource = cSource.GetString();
	return csSource.find(cValue1.GetString());
}

int CSystemObjects::StrLineCount(CValue &cSource)
{
	wxString csSource = cSource.GetString();
	return csSource.find('\n') + 1;
}

wxString CSystemObjects::StrGetLine(CValue &cValue, unsigned int nLine)
{
	wxString csSource = cValue.GetString() + wxT("\r\n");

	unsigned int nLast = 0;
	unsigned int nStartLine = 1;

	//********блок для ускорения
	static wxString _csStaticSource;

	static unsigned int _nStaticLast = 0;
	static unsigned int _nStaticLine = 0;

	if (_csStaticSource == csSource)
	{
		if (_nStaticLine <= nLine)
		{
			nLast = _nStaticLast;//т.е. начинаем поиск не с начала
			nStartLine = _nStaticLine;
		}
	}

	//перебираем строчки в тупую
	for (unsigned int i = nStartLine;; i++)
	{
		unsigned int nIndex = csSource.find(wxT("\r\n"), nLast);

		if (nIndex < 0) return wxEmptyString;

		if (i == nLine)
		{
			_csStaticSource = csSource;
			_nStaticLast = nIndex + 2;
			_nStaticLine = nLine + 1;

			return csSource.Mid(nLast, nIndex - nLast);
		}

		nLast = nIndex + 2;
	}

	return wxEmptyString;
}

wxString CSystemObjects::Upper(CValue &cSource)
{
	wxString csSource = cSource.GetString();
	csSource.MakeUpper();
	return csSource;
}

wxString CSystemObjects::Lower(CValue &cSource)
{
	wxString csSource = cSource.GetString();
	csSource.MakeLower();
	return csSource;
}

wxString CSystemObjects::Chr(short nCode)
{
	return wxString(static_cast<wchar_t>(nCode));
}

short CSystemObjects::Asc(CValue &cSource)
{
	wxString csSource = cSource.GetString();
	if (!csSource.Length()) return 0;
	return static_cast<wchar_t>(csSource[0]);
}

//---Работа с датой и временем
CValue CSystemObjects::CurrentDate()
{
	wxDateTime timeNow = wxDateTime::Now();

	timeNow.SetHour(0);
	timeNow.SetMinute(0);
	timeNow.SetSecond(0);

	wxLongLong m_llValue = timeNow.GetValue();

	CValue valueNow = eValueTypes::TYPE_DATE;
	valueNow.m_dData = m_llValue.GetValue();
	return valueNow;
}

CValue CSystemObjects::CurrentTime()
{
	wxDateTime timeNow = wxDateTime::Now();
	wxLongLong m_llValue = timeNow.GetValue();

	CValue valueNow = eValueTypes::TYPE_DATE;
	valueNow.m_dData = m_llValue.GetValue();
	return valueNow;
}

CValue CSystemObjects::WorkingDate()
{
	m_workDate.SetHour(0);
	m_workDate.SetMinute(0);
	m_workDate.SetSecond(0);

	return m_workDate;
}

CValue CSystemObjects::AddMonth(CValue &cData, int nMonthAdd)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);
	int SummaMonth = nYear * 12 + nMonth - 1;
	SummaMonth += nMonthAdd;
	nYear = SummaMonth / 12;
	nMonth = SummaMonth % 12 + 1;

	return CValue(nYear, nMonth, nDay);
}

CValue CSystemObjects::BegOfMonth(CValue &cData)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);
	return CValue(nYear, nMonth, 1);
}

CValue CSystemObjects::EndOfMonth(CValue &cData)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);

	CValue m_date = CValue(nYear, nMonth, 1, 23, 59, 59);
	AddMonth(m_date, -1);
	return AddMonth(m_date, -1);
}

CValue CSystemObjects::BegOfQuart(CValue &cData)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);
	return CValue(nYear, 1 + ((nMonth - 1) / 3) * 3, 1);
}

CValue CSystemObjects::EndOfQuart(CValue &cData)
{
	return AddMonth(BegOfQuart(cData), 3) - 1;
}

CValue CSystemObjects::BegOfYear(CValue &cData)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);
	return CValue(nYear, 1, 1);
}

CValue CSystemObjects::EndOfYear(CValue &cData)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);
	return CValue(nYear, 12, 31, 23, 59, 59);
}

CValue CSystemObjects::BegOfWeek(CValue &cData)
{
	int nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear;
	cData.FromDate(nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear);
	CValue Date1 = CValue(nYear, nMonth, nDay) - (DayOfWeek + 1);
	return Date1;
}

CValue CSystemObjects::EndOfWeek(CValue &cData)
{
	int nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear;
	cData.FromDate(nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear);
	CValue Date1 = CValue(nYear, nMonth, nDay) + (7 - DayOfWeek);
	return Date1;
}

int CSystemObjects::GetYear(CValue &cData)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);
	return nYear;
}

int CSystemObjects::GetMonth(CValue &cData)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);
	return nMonth;
}

int CSystemObjects::GetDay(CValue &cData)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);
	return nDay;
}

int CSystemObjects::GetHour(CValue &cData)
{
	int nYear, nMonth, nDay; unsigned short nHour, nMinutes, nSeconds;
	cData.FromDate(nYear, nMonth, nDay, nHour, nMinutes, nSeconds);
	return nHour;
}

int CSystemObjects::GetMinute(CValue &cData)
{
	int nYear, nMonth, nDay; unsigned short nHour, nMinutes, nSeconds;
	cData.FromDate(nYear, nMonth, nDay, nHour, nMinutes, nSeconds);
	return nMinutes;
}

int CSystemObjects::GetSecond(CValue &cData)
{
	int nYear, nMonth, nDay; unsigned short nHour, nMinutes, nSeconds;
	cData.FromDate(nYear, nMonth, nDay, nHour, nMinutes, nSeconds);
	return nSeconds;
}

int CSystemObjects::GetWeekOfYear(CValue &cData)
{
	int nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear;
	cData.FromDate(nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear);
	return WeekOfYear;
}

int CSystemObjects::GetDayOfYear(CValue &cData)
{
	int nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear;
	cData.FromDate(nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear);
	return DayOfYear;
}

int CSystemObjects::GetDayOfWeek(CValue &cData)
{
	int nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear;
	cData.FromDate(nYear, nMonth, nDay, DayOfWeek, DayOfYear, WeekOfYear);
	return DayOfWeek;
}

int CSystemObjects::GetQuartOfYear(CValue &cData)
{
	int nYear, nMonth, nDay;
	cData.FromDate(nYear, nMonth, nDay);
	return 1 + ((nMonth - 1) / 3);
}

static spell_date &sd = Speller::GetDefaultSpeller().DatePart;

wxString CSystemObjects::PeriodStr(CValue &cData1, CValue &cData2)
{
	cData1 = Date(cData1);
	cData2 = Date(cData2);

	wxString Str;
	int nYear1, nMonth1, nDay1;
	int nYear2, nMonth2, nDay2;
	cData1.FromDate(nYear1, nMonth1, nDay1);
	cData2.FromDate(nYear2, nMonth2, nDay2);
	//месяц
	if (cData1 == BegOfMonth(cData1) && cData2 == EndOfMonth(cData1))
		Str = wxString::Format("%s %d %s", sd[nMonth1], nYear1, sd.SmallYear);
	else if (cData1 == BegOfQuart(cData1) && cData2 == EndOfQuart(cData1)) //квартал
		Str = wxString::Format("%d %s %d %s", int((nMonth1 - 1) / 3) + 1, sd.Quart, nYear1, sd.SmallYear);
	else if (cData1 == BegOfYear(cData1))
	{
		CValue m_date_6(nYear1, 6, 1);
		CValue m_date_9(nYear1, 9, 1);
		CValue m_date_1(nYear1, 16, 1);

		// 6 месяцев
		if (cData2 == EndOfMonth(m_date_6))
			Str = wxString::Format("1 %s %d %s", sd.SemiYear, nYear1, sd.SmallYear);
		if (cData2 == EndOfMonth(m_date_9))
			Str = wxString::Format("%s %d %s", sd.Month_9, nYear1, sd.SmallYear);
		if (cData2 == EndOfYear(m_date_1))
			Str = wxString::Format("%d %s", nYear1, sd.SmallYear);
	}
	else
	{
		//остальное - просто диапазон
		Str = cData1.GetString() + wxT(" - ") + cData2.GetString();
	}

	return Str;
}

//--- Специальные:
void CSystemObjects::Message(const wxString &sMessage, eStatusMessage status)
{
	if (CTranslateError::IsSimpleMode()) return;
	if (status == eStatusMessage::eStatusMessage_Information) outputWindow->OutputMessage(sMessage);
	else if (status == eStatusMessage::eStatusMessage_Warning) outputWindow->OutputWarning(sMessage);
	else outputWindow->OutputError(sMessage);
}

void CSystemObjects::Alert(const wxString &sMessage)//Предупреждение
{
	if (CTranslateError::IsSimpleMode()) return;
	wxMessageBox(sMessage, wxT("Warning"), wxICON_WARNING, CMainFrame::Get());
}

CValue CSystemObjects::Question(const wxString &sMessage, eQuestionMode mode)//Вопрос
{
	if (CTranslateError::IsSimpleMode()) return CValue();

	int m_wndStyle = 0;

	if (mode == eQuestionMode::eQuestionMode_OK) m_wndStyle = wxOK;
	else if (mode == eQuestionMode::eQuestionMode_OKCancel) m_wndStyle = wxOK | wxCANCEL;
	else if (mode == eQuestionMode::eQuestionMode_YesNo) m_wndStyle = wxYES | wxNO;
	else if (mode == eQuestionMode::eQuestionMode_YesNoCancel) m_wndStyle = wxYES | wxNO | wxCANCEL;

	int retCode = wxMessageBox(sMessage, wxT("Question"), m_wndStyle | wxICON_QUESTION, CMainFrame::Get());

	if (retCode == wxOK) return new CValueQuestionReturnCode(eQuestionReturnCode::eQuestionReturnCode_OK);
	else if (retCode == wxCANCEL) return new CValueQuestionReturnCode(eQuestionReturnCode::eQuestionReturnCode_Cancel);
	else if (retCode == wxYES) return new CValueQuestionReturnCode(eQuestionReturnCode::eQuestionReturnCode_Yes);
	else if (retCode == wxNO) return new CValueQuestionReturnCode(eQuestionReturnCode::eQuestionReturnCode_No);

	return CValue();
}

void CSystemObjects::SetStatus(const wxString &sStatus)
{
	if (CTranslateError::IsSimpleMode()) return;
	mainFrame->SetStatusText(sStatus);
}

void CSystemObjects::ClearMessages()
{
	if (CTranslateError::IsSimpleMode()) return;
	outputWindow->Clear();
}

void CSystemObjects::SetError(const wxString &sError)
{
	if (CTranslateError::IsSimpleMode()) return;
	CTranslateError::Error(sError);
}

void CSystemObjects::Raise(const wxString &sError)
{
	if (CTranslateError::IsSimpleMode()) return;
	CProcUnit::Raise(); CTranslateError::Error(sError);
}

wxString CSystemObjects::ErrorDescription()
{
	if (CTranslateError::IsSimpleMode()) return wxEmptyString;
	return CTranslateError::GetLastError();
}

bool CSystemObjects::IsEmptyValue(CValue &cData)
{
	return cData.IsEmpty();
}

CValue CSystemObjects::Evaluate(const wxString &sExpression)
{
	return CProcUnit::Evaluate(sExpression, CProcUnit::GetCurrentRunContext(), false);
}

void CSystemObjects::Execute(const wxString &sExpression)
{
	if (CTranslateError::IsSimpleMode()) return;
	CProcUnit::Evaluate(sExpression, CProcUnit::GetCurrentRunContext(), true);
}

static Speller &m_gSpell = Speller::GetDefaultSpeller();

wxString CSystemObjects::Format(CValue &cData, wxString &fmt)
{
	if (fmt.IsEmpty()) { return cData.GetString(); }

	int LeadingZero;

	if ("(0)" == fmt.Mid(1, 3)) { LeadingZero = 1; fmt = fmt.Mid(4); }
	else { LeadingZero = 0; fmt = fmt.Mid(1); }

	wxString rez;

	switch (cData.GetType())
	{
	case eValueTypes::TYPE_DATE:
	{
		spell_date &sd = m_gSpell.DatePart;
		wxLongLong_t dta = cData.GetDate();
		int yy, mm, dd;

		wxLongLong m_llDate(dta);
		wxDateTime nDate(m_llDate);

		yy = nDate.GetYear();
		mm = nDate.GetMonth() - 1;
		dd = nDate.GetDay();

		if (!yy) { //Если год=0 - некорректна¤ (или пуста¤) дата
			rez = " . . ";
		}
		else
		{
			int octParam = 0;

			//ѕреобразуем формат в 8-ричное число дл¤ удобства сравнени¤
			for (LPCTSTR s = fmt; *s && !(octParam & 030000000000); s++)
			{
				octParam <<= 3;

				switch (*s) {
				case 'Y': case 'y': octParam |= 7; break;
				case 'D': case 'd': octParam |= 6; break;
				case 'M': case 'm': octParam |= 5; break;
					// 4 не используем, т.к. дл¤ контрол¤ переполнени¤ 
					// мы можем использовать только 2 последних бита, а у 4 там нули.
				case 'W': case 'w': octParam |= 2; break;
				case 'Q': case 'q': octParam |= 3; break;
				}
			}

			switch (octParam)
			{
			case 0665577: //DDMMYY
				rez = wxString::Format("%02d.%02d.%02d", dd, mm, yy % 100);
				break;
			case 066557777: //DDMMYYYY
			default: //он же формат по-умолчанию
				rez = wxString::Format("%02d.%02d.%04d", dd, mm, yy);
				break;
			case 066555577: //DDMMMMYY
				rez = wxString::Format((LeadingZero ? "%02d %s %02d %s" : "%d %s %02d %s"), dd, sd[spell_date::InM + mm], yy % 100, sd.SmallYear);
				Speller::FirstCharBig(rez);
				break;

			case 06655557777: //DDMMMMYYYY
				rez = wxString::Format((LeadingZero ? "%02d %s %04d %s" : "%d %s %04d %s"), dd, sd[spell_date::InM + mm], yy, sd.SmallYear);
				Speller::FirstCharBig(rez);
				break;

			case 05555: //MMMM
				rez = wxString::Format("%s", sd[mm]);
				Speller::FirstCharBig(rez);
				break;

			case 0555577: //MMMMYY
				rez.Format("%s %02d %s", sd[mm], yy % 100, sd.SmallYear);
				Speller::FirstCharBig(rez);
				break;

			case 055557777: //MMMMYYYY
				rez.Format("%s %04d %s", sd[mm], yy, sd.SmallYear);
				Speller::FirstCharBig(rez);
				break;

			case 03333: //QQQQ
				rez = wxString::Format((LeadingZero ? "%02d %s" : "%d %s")
					, mm / 3 + 1, sd.Quart);
				Speller::FirstCharBig(rez);
				break;

			case 0333377: //QQQQYY
				rez = wxString::Format((LeadingZero ? "%02d %s %02d %s" : "%d %s %02d %s")
					, mm / 3 + 1, sd.Quart, yy % 100, sd.SmallYear);
				Speller::FirstCharBig(rez);
				break;

			case 033337777: //QQQQYYYY
				rez = wxString::Format((LeadingZero ? "%02d %s %04d %s" : "%d %s %04d г.")
					, mm / 3 + 1, sd.Quart, yy, sd.SmallYear);
				Speller::FirstCharBig(rez);
				break;

			case 02222: //WWWW
				rez = sd.DayOfWeek[dta % 7];
				Speller::FirstCharBig(rez);
				break;

			case 077775566: //YYYYMMDD
				rez = wxString::Format("%04d%02d%02d", yy, mm, dd);
				break;
			}
		}
		break;
	}
	//Digits
	case eValueTypes::TYPE_NUMBER:
	{
		spell_num &sn = m_gSpell.num_Speller;

		if ('ѕ' == fmt[0]
			|| 'S' == fmt[0])
		{
			if (fmt.Length() > 1 && ('m' == fmt[1] || 'M' == fmt[1]))
			{
				if (fmt.Length() > 2 && ('h' == fmt[2] || 'H' == fmt[2]))
				{
					rez = sn(cData.ToDouble(), *m_gSpell.UnitBig, *m_gSpell.UnitSmall);
				}
				else
				{
					rez = sn(cData.ToDouble(), *m_gSpell.UnitBig);
				}
			}
			else
			{
				rez = m_gSpell.num_Speller(cData.ToDouble());
			}

			Speller::FirstCharBig(rez);
			break;
		}

		//Обычное число
		int width = -1, prec = 0, stat = 1, move = 0;
		char zero = 0, dot = '.', coma = 0;

		LPCTSTR s = fmt;

		if ('0' == *s) { zero = ' '; s++; }
		else if ('-' == *s) { zero = *s; s++; }

		for (; *s; s++)
		{
			if ('0' <= *s
				&& *s <= '9')
			{
				if (width < 0)
					width = 0;
				else
					width *= 10;
				width += (*s - '0');
			}
			else break;
		}

		if ('.' == *s)
		{
			for (s++; *s; s++)
			{
				if ('0' <= *s
					&& *s <= '9')
				{
					prec *= 10;
					prec += (*s - '0');
				}
				else break;
			}
		}

		if ('>' == *s)
		{
			for (s++; *s; s++)
			{
				if ('0' <= *s
					&& *s <= '9')
				{
					move *= 10;
					move += (*s - '0');
				}
				else
					break;
			}
		}

		if (*s) { dot = *s; s++; }
		if (*s) { coma = *s; }

		cData = CValue(cData.GetNumber() / pow(10, move));
		rez = Speller::FloatFormat(cData.ToDouble(), width, prec, LeadingZero, zero, dot, coma);

		break;
	}

	//Строки
	case eValueTypes::TYPE_STRING:
	{
		unsigned int len = wxAtoi(fmt); //” Format по умолчанию дополнение пробелами слева, у 1— - справа
		wxString csStr = cData.GetString();

		if (!len)
			rez = csStr;
		else if (len < csStr.Length())
			csStr = csStr.Left(len);

		rez.Format("%*s", len, csStr); break;
	}
	default: //Не распознан или не задан
		rez = cData.GetString();
	}
	return rez;
}

#include "valueType.h"

CValue CSystemObjects::Type(const CValue &cTypeName)
{
	if (cTypeName.GetType() != eValueTypes::TYPE_STRING)
	{
		Raise(wxString::Format("Cannot convert value"));
		return CValue();
	}

	wxString typeName = cTypeName.GetString();

	if (!CValue::IsRegisterObject(typeName))
		Raise(wxString::Format("Type not found '%s'", typeName));

	unsigned long long type_id = CValue::GetIDObjectFromString(typeName);
	return new CValueType(type_id);
}

CValue CSystemObjects::TypeOf(const CValue &cData)
{
	return new CValueType(cData);
}

int CSystemObjects::Rand()
{
	return rand();
}

int CSystemObjects::ArgCount()//КоличествоАргументовПрограммы
{
	return __argc;
}

wxString CSystemObjects::ArgValue(int n)//ЗначениеАргументаПрограммы
{
	if (n<0 || n> __argc) CTranslateError::Error(_("Invalid argument index"));
	return __wargv[n];
}

wxString CSystemObjects::ComputerName()//ИмяКомпьютера
{
	return wxGetHostName();
}

void CSystemObjects::RunApp(const wxString &sCommand)//ЗапуститьПриложение
{
	if (CTranslateError::IsSimpleMode()) return;
	wxExecute(sCommand);
}

void CSystemObjects::SetAppTitle(const wxString &sTitle)//ЗаголовокСистемы
{
	if (CTranslateError::IsSimpleMode()) return;
	mainFrame->SetTitle(sTitle);
}

wxString CSystemObjects::UserDir() { return appData->GetApplicationPath(); }

wxString CSystemObjects::UserName() { return wxEmptyString; }

wxString CSystemObjects::UserPassword() { return wxEmptyString; }

bool CSystemObjects::SingleMode() { return false; }

wxString CSystemObjects::TempFilesDir() { return wxEmptyString; }

int CSystemObjects::GeneralLanguage() { return 1; }

////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko
//	Description : system objects 
////////////////////////////////////////////////////////////////////////////

#include "metadata/metadata.h"

void CSystemObjects::EndJob(bool force)//ЗавершитьРаботуСистемы
{
	if (force)
	{
		appData->Destroy();
		std::exit(EXIT_SUCCESS);
	}
	else
	{
		IModuleManager *moduleManager = metadata->GetModuleManager();

		if (moduleManager->DestroyMainModule())
		{
			appDataDestroy();
			std::exit(EXIT_SUCCESS);
		}
	}
}

void CSystemObjects::UserInterruptProcessing()
{
	bool m_bNeedInterruptProcessing = wxGetKeyState(WXK_CONTROL)
		&& wxGetKeyState(WXK_CANCEL);
	if (m_bNeedInterruptProcessing) throw (new CInterruptBreak());
}

CValue CSystemObjects::GetCommonForm(const wxString &sFormName)
{
	for (auto commonForm : metadata->GetMetaObjects(g_metaCommonFormCLSID))
	{
		if (StringUtils::MakeUpper(sFormName) == StringUtils::MakeUpper(commonForm->GetName()))
		{
			CValueFrame *m_valueFrame = static_cast<CMetaFormObject *>(commonForm)->GenerateForm();
			if (m_valueFrame)
			{
				unsigned int nRefOld = m_valueFrame->GetRefCount();

				m_valueFrame->IncrRef();

				if (appData->IsEnterpriseMode())
					m_valueFrame->InitializeModule(true);

				m_valueFrame->SetRefCount(nRefOld);

				return m_valueFrame;
			}
		}
	}

	Raise("Сommon form not found '" + sFormName + "'");
	return CValue();
}

void CSystemObjects::ShowCommonForm(const wxString &sFormName)
{
	if (CTranslateError::IsSimpleMode()) return;

	for (auto commonForm : metadata->GetMetaObjects(g_metaCommonFormCLSID))
	{
		if (StringUtils::MakeUpper(sFormName) == StringUtils::MakeUpper(commonForm->GetName()))
		{
			CValueFrame *m_valueFrame = static_cast<CMetaFormObject *>(commonForm)->GenerateForm();
			if (m_valueFrame)
			{
				unsigned int nRefOld = m_valueFrame->GetRefCount();

				m_valueFrame->IncrRef();

				if (appData->IsEnterpriseMode())
					m_valueFrame->InitializeModule(true);

				m_valueFrame->SetRefCount(nRefOld);

				m_valueFrame->ShowForm(); return;
			}
		}
	}

	Raise("Сommon form not found '" + sFormName + "'");
}

void CSystemObjects::BeginTransaction()
{
	if (CTranslateError::IsSimpleMode()) return;
	objectDatabase->BeginTransaction();
}

void CSystemObjects::CommitTransaction()
{
	if (CTranslateError::IsSimpleMode()) return;
	objectDatabase->Commit();
}

void CSystemObjects::RollBackTransaction()
{
	if (CTranslateError::IsSimpleMode()) return;
	objectDatabase->RollBack();
}