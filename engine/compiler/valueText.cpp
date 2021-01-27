////////////////////////////////////////////////////////////////////////////
//	Author		: Maxim Kornienko, 2C- team
//	Description : value text
////////////////////////////////////////////////////////////////////////////

#include "valuetext.h"
#include "functions.h"
#include "methods.h"

CMethods CValueText::m_methods;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
wxIMPLEMENT_DYNAMIC_CLASS(CValueText, CValue);

CValueText::CValueText() : CValue(eValueTypes::TYPE_VALUE)
{
	bReadOnly = false;
	pDoc = NULL;
	sTitle = "Текст";
	nCodePage = 0;
	nTemplate = 0;
	nFixTemplate = 0;
}

CValueText::~CValueText()
{
}


//******************************
//Работа как агрегатного объекта
//******************************
void CValueText::PrepareNames() const
{
	SEng aMethods[] = {
		{"LinesCount","КоличествоСтрок"},
		{"GetLine","ПолучитьСтроку"},
		{"Open","Открыть"},
		{"Template","Шаблон"},
		{"FixTemplate","ФиксШаблон"},
		{"InsertLine","ВставитьСтроку"},
		{"AddLine","ДобавитьСтроку"},
		{"ReplaceLine","ЗаменитьСтроку"},
		{"DeleteLine","УдалитьСтроку"},
		{"ReadOnly","ТолькоПросмотр"},
		{"Show","Показать"},
		{"Clear","Очистить"},
		{"CodePage","КодоваяСтраница"},
		{"Write","Записать"},
		{"GetText","ПолучитьТекст"},
	};
	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods.PrepareMethods(aMethods, nCountM);
}

#define DEF_N_LINE()\
unsigned int n=aParams[0].ToInt();\
if(n<=0||n>aText.size())\
	CTranslateError::Error("Индекс выходит за границы количества строк текста");\
n--;

//extern wxString Template(wxString arg);  

CValue CValueText::Method(CMethodParameters &aParams)
{
	CValue Ret;
	switch (aParams.GetMethodIndex())
	{
	case enLinesCount:
		Ret = (int)aText.size();
		break;
	case enClear:
		aText.clear();
		break;
	case enGetLine:
	{
		DEF_N_LINE()
			Ret = aText[n];
		break;
	}
	case enAddLine:
		//if (nTemplate)
		//	aText.Add(Template(p[0]->GetString()));
		//else
		aText.Add(aParams[0].ToString());
		break;
	case enReplaceLine:
	{
		DEF_N_LINE()
			//if (nTemplate)
			//	aText[n] = Template(p[1]->GetString());
			//else
			aText[n] = aParams[1].ToString();
		break;
	}
	case enDeleteLine:
	{
		DEF_N_LINE()
			aText.RemoveAt(n);
		break;
	}
	case enInsertLine:
	{
		DEF_N_LINE()
			//if (nTemplate)
			//	aText.Insert(Template(p[1]->GetString()), n);
			//else
			aText.Insert(aParams[1].ToString(), n);
		break;
	}

	case enOpen:
	{
		wxString sName = aParams[0].ToString();
		//if (!FileExist(sName))
		//	CTranslateError::Error(wxString("Файл ") + sName + " не найден");
		aText.clear();
		sTitle = sName;

		//int nSize = 0;
		//char *buf = LoadFromFileBin(sName, nSize);
		//if (!nSize)
		//	break;
		//if (buf)
		//{
		//	wxString Str;
		//	Str.GetBuffer(10000);
		//	for (int i = 0; i < nSize; i++)
		//	{
		//		if (buf[i] == 0x0D && buf[i + 1] == 0x0A)
		//		{
		//			if (nTemplate)
		//				aText.Add(Template(Str));
		//			else
		//				aText.Add(Str);
		//			Str = "";
		//			i++;
		//			//i++;
		//		}
		//		else
		//		{
		//			Str += buf[i];
		//		}
		//	}
		//	if (nTemplate)
		//		aText.Add(Template(Str));
		//	else
		//		aText.Add(Str);
		//	Str.ReleaseBuffer();
		//	delete[]buf;
		//}
		break;
	}
	case enShow:
	{
		//if (!pDoc)
		//	pDoc = (CModuleDoc*)OpenFormMDI("Модуль", 1);
		//pDoc->SetPathName(sTitle, 0);
		//pDoc->SetTitle(sTitle);
		//pDoc->SetText(GetStr());
		//pDoc->m_xTextBuffer.SetReadOnly(bReadOnly);
		break;
	}
	case enWrite:
	{
		wxString sName = aParams[0].ToString();
		sTitle = sName;

		//extern void WritoToFileText(wxString csCFile, wxString Str);
		//WritoToFileText(sName, GetStr());

		break;
	}
	case enReadOnly:
	{
		Ret = bReadOnly;
		bReadOnly = aParams[0].ToBool();
		//if (pDoc)
		//	pDoc->m_xTextBuffer.SetReadOnly(bReadOnly);

		break;
	}
	case enCodePage:
	{
		Ret = nCodePage;
		if (aParams[0].GetType() == eValueTypes::TYPE_NUMBER)
			nCodePage = aParams[0].ToInt();
		break;
	}
	case enTemplate:
	{

		Ret = nTemplate;
		if (aParams[0].GetType() == eValueTypes::TYPE_NUMBER)
			nTemplate = aParams[0].ToInt();
		break;
	}
	case enFixTemplate:
	{
		Ret = nFixTemplate;
		if (aParams[0].GetType() == eValueTypes::TYPE_NUMBER)
			nFixTemplate = aParams[0].ToInt();
		break;
	}
	case enGetText:
		Ret = GetStr();
		break;
	}
	return Ret;
}

wxString CValueText::GetStr()
{
	wxString Str;
	int nDeltaSize = 100000;
	int nSize = nDeltaSize;
	//Str.GetBuffer(nSize);
	//for (int i = 0; i < aText.GetSize(); i++)
	//{
	//	if (i > 0)
	//		Str += "\r\n";
	//	Str += aText[i];
	//	if (Str.GetLength() > nSize)
	//	{
	//		nSize += nDeltaSize;
	//		Str.GetBuffer(nSize);
	//	}
	//}
	//Str.ReleaseBuffer();
	return Str;
}


//**********************************************************************
//*                       Runtime register                             *
//**********************************************************************

VALUE_REGISTER(CValueText, "textDocument", TEXT2CLSID("VL_TDOC"));
