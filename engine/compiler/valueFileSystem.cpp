#include "valuefilesystem.h"
#include "methods.h"
#include "functions.h"

#include "utils/stringutils.h"

wxString GetGetLastStringError();

CMethods CValueFileSystem::m_methods;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxIMPLEMENT_DYNAMIC_CLASS(CValueFileSystem, CValue);


CValueFileSystem::CValueFileSystem() : CValue(eValueTypes::TYPE_VALUE), hFindFile(NULL), nTimer(0) {}

CValueFileSystem::~CValueFileSystem()
{
	if (hFindFile)
		FindClose(hFindFile);
	hFindFile = NULL;

	CloseAllHandle();
}

void CValueFileSystem::CloseAllHandle()
{
	for (auto it = aListHandle.begin(); it != aListHandle.end(); it++)
	{
		HANDLE hFile = (void*&)it->second;
		if (hFile)
			CloseHandle(hFile);
	}

	aListHandle.clear();
}

wxString GetStringFromTime(FILETIME &FileTime)
{
	SYSTEMTIME time;
	FileTimeToSystemTime(&FileTime, &time);
	wxString sRes;
	sRes = wxString::Format("%d.%d.%d %d:%d:%d", time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);
	return sRes;
}

//******************************
//–абота как агрегатного объекта
//******************************
void CValueFileSystem::PrepareNames() const
{
	SEng aMethods[] = {
		{"SelectFile","¬ыбрать‘айл(“ипƒиалога,»м€‘айла,»м€Ќач аталога,«аголовокќкна,‘ильтр,–асширение,“аймаут)"},
		{"SelectPictFile","¬ыбрать‘айл артинки(“ипƒиалога,»м€‘айла,»м€Ќач аталога,«аголовокќкна,‘ильтр,–асширение,“аймаут)"},
		{"SelectDirectory","¬ыбрать аталог(»м€ аталога,«аголовокќкна,“аймаут)"},
		{"ExistFile", "—уществует‘айл(»м€‘айла) - провер€ет существование файла"},
		{"CreateDirectory","—оздать аталог(»м€‘айла)"},
		{"RemoveDirectory","”далить аталог(»м€‘айла)"},
		{"SetCurrentDirectory","”ст“ек аталог(»м€‘айла) - устанавливает текущий каталог файлов"},
		{"GetCurrentDirectory","“ек аталог() - возвращает строку с именем текущего каталога файлов"},
		{"DeleteFile","”далить‘айл(»м€‘айла)"},
		{"FindFirstFile","Ќайтиѕервый‘айл(ћаска»мени)"},
		{"FindNextFile","Ќайти—ледующий‘айл() - находит следующий файл по открытой выборке"},
		{"GetFileAttr","јтрибуты‘айла(»м€‘айла,–азмер‘айла,јтрибуты‘айла,¬рем€—оздани€,¬рем€ѕоследнегоƒоступа,¬рем€ѕоследней«аписи,–асширенное»м€‘айла)"},
		{"FileCopy"," опировать‘айл(»м€‘айла»сточника,»м€‘айлаѕриемника,‘лагѕерезаписи)"},
		{"GetWindowsDirectory", "Windows аталог() - возвращает строку с именем Windows директории"},
		{"GetDiskFreeSpace", "—вободноећестоЌаƒиске(»м€ƒиска)"},
		{"MoveFile","ѕереименовать‘айл(»м€‘айла»сточника,»м€‘айлаѕриемника,‘лагѕерезаписи)"},
		{"LockFile ","«аблокировать‘айл(»м€‘айла,ѕозици€=0,ƒлина=1) /блокировка участка файла/"},
		{"UnLockFile ","–азблокировать‘айл(»м€‘айла,ѕозици€=0,ƒлина=1) /разблокировка участка файла/"},
		{"UnLockAllFile ", "–азблокировать¬се‘айлы() /разблокировка всех ранее заблокированных файлов/"},
		{"WriteData "," «аписатьƒанные(»м€‘айла,—трƒанные,ѕозици€) /записать данные в файл с указанной позиции"},
		{"ReadData ", "ѕрочитатьƒанные(»м€‘айла,ѕозици€,ƒлина) /прочитать данные из файла с указанной позиции"},
	};
	int nCountM = sizeof(aMethods) / sizeof(aMethods[0]);
	m_methods.PrepareMethods(aMethods, nCountM);
}

enum
{
	enSelectFile = 0,
	enSelectPictFile,
	enSelectDirectory,
	enExistFile,
	enCreateDirectory,
	enRemoveDirectory,
	enSetCurrentDirectory,
	enGetCurrentDirectory,
	enDeleteFile,
	enFindFirstFile,
	enFindNextFile,
	enGetFileAttr,
	enFileCopy,
	enGetWindowsDirectory,
	enGetDiskFreeSpace,
	enMoveFile,
	enLockFile,
	enUnLockFile,
	enUnLockAllFile,
	enWriteData,
	enReadData,
};

#include "definition.h"

CValue CValueFileSystem::Method(CMethodParameters &aParams)
{
	CValue Ret;

	switch (aParams.GetMethodIndex())
	{
	case enSelectFile:
	{
		wxString sFile = aParams[1].ToString();
		wxString sDir = aParams[2].ToString();
		wxString sTitle = aParams[3].ToString();
		wxString sFilter = aParams[4].GetString();

		wxFileDialog* dlg = new wxFileDialog(NULL, sTitle, sDir, sFile, sTitle,
			wxFD_OPEN, wxDefaultPosition);

		dlg->SetFilterIndexFromExt(sFilter);

		Ret = -1;

		if (dlg->ShowModal() == wxID_OK)
		{
			sFile = dlg->GetName();
			sDir = dlg->GetPath();
			sDir = sDir.Mid(0, sDir.Length() - sFile.Length());
			aParams[1] = (sFile);
			aParams[2] = (sDir);
			Ret = 1;
		}
		else
		{
			Ret = 0;
		}

		// Clean up after ourselves
		dlg->Destroy();
		break;
	}
	case enSelectPictFile:
	{
		wxString sFile = aParams[1].ToString();
		wxString sDir = aParams[2].ToString();
		wxString sTitle = aParams[3].ToString();
		wxString sFilter = aParams[4].GetString();

		wxFileDialog* dlg = new wxFileDialog(NULL, sTitle, sDir, sFile, sTitle,
			wxFD_OPEN, wxDefaultPosition);

		Ret = -1;

		dlg->SetFilterIndexFromExt(sFilter);

		if (dlg->ShowModal() == wxID_OK)
		{
			sFile = dlg->GetName();
			sDir = dlg->GetPath();
			sDir = sDir.Mid(0, sDir.Length() - sFile.Length());
			aParams[1] = (sFile);
			aParams[2] = (sDir);
			Ret = 1;
		}
		else
		{
			Ret = 0;
		}

		// Clean up after ourselves
		dlg->Destroy();
		break;
	}
	case enSelectDirectory:
	{
		wxFileDialog* dlg = new wxFileDialog(NULL, aParams[1].ToString());
		dlg->SetWindowStyle(wxFD_CHANGE_DIR);

		if (dlg->ShowModal() == wxID_OK)
		{
			Ret = 1;
			aParams[0] = (dlg->GetPath());
		}
		else
		{
			Ret = 0;
		}
		break;
	}
	case enGetCurrentDirectory:
	{
		wchar_t csCurrentDir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, csCurrentDir);
		Ret = (csCurrentDir);
		break;
	}
	case enSetCurrentDirectory:
	{
		SetCurrentDirectory(aParams[0].ToString());
		break;
	}
	case enCreateDirectory:
	{
		CreateDirectory(aParams[0].ToString(), 0);
		break;
	}
	case enDeleteFile:
	{
		DeleteFile(aParams[0].ToString());
		break;
	}
	case enFindFirstFile:
	{
		if (hFindFile)
			FindClose(hFindFile);
		hFindFile = 0;
		WIN32_FIND_DATA FindFileData;
		hFindFile = FindFirstFile(aParams[0].ToString(), &FindFileData);
		if (hFindFile != INVALID_HANDLE_VALUE)
			Ret = (FindFileData.cFileName);
		else
			Ret = ("");
		break;
	}
	case enFindNextFile:
	{
		WIN32_FIND_DATA FindFileData;
		if (!hFindFile)
		{
			Ret = ("");
		}
		else
			if (FindNextFile(hFindFile, &FindFileData))
				Ret = (FindFileData.cFileName);
			else
				if (hFindFile)
				{
					FindClose(hFindFile);
					hFindFile = 0;
					Ret = ("");
				}

		break;
	}
	case enGetFileAttr:
	{
		HANDLE hFindFile;
		WIN32_FIND_DATA FindFileData;
		hFindFile = FindFirstFile(aParams[0].ToString(), &FindFileData);
		if (hFindFile != INVALID_HANDLE_VALUE)
		{
			aParams[0] = (FindFileData.cFileName);
			aParams[1] = double(FindFileData.nFileSizeHigh) * 0x100000000 + double(FindFileData.nFileSizeLow);
			wxString csAttr;
			int nMask = 1;
			for (int i = 0; i < 9; i++)
			{
				if ((FindFileData.dwFileAttributes&nMask) == nMask)
					csAttr += "1";
				else
					csAttr += "0";
				nMask = nMask * 2;
			}
			aParams[2] = (csAttr);

			aParams[3] = (GetStringFromTime(FindFileData.ftCreationTime));
			aParams[4] = (GetStringFromTime(FindFileData.ftLastAccessTime));
			aParams[5] = (GetStringFromTime(FindFileData.ftLastWriteTime));


			aParams[6] = (FindFileData.cAlternateFileName);
		}

		break;
	}
	case enFileCopy:
	{
		Ret = CopyFile(aParams[0].ToString(), aParams[1].ToString(), aParams[2].ToInt());
		break;
	}

	case enExistFile:
	{
		WIN32_FIND_DATA data;
		HANDLE hFile = FindFirstFile(aParams[0].ToString(), &data);
		if (hFile == INVALID_HANDLE_VALUE || hFile == 0)
		{
			Ret = 0;
		}
		else
		{
			Ret = 1;
			FindClose(hFile);
		}
		break;
	}

	case enRemoveDirectory:
	{
		Ret = RemoveDirectory(aParams[0].ToString());
		break;
	}

	case enGetWindowsDirectory:
	{
		wchar_t csStr[MAX_PATH];

		GetWindowsDirectory(csStr, MAX_PATH);
		Ret = (csStr);

		break;
	}


	case enGetDiskFreeSpace:
	{
		ULARGE_INTEGER i64FreeBytesToCaller;
		ULARGE_INTEGER i64TotalBytes;
		ULARGE_INTEGER i64FreeBytes;

		GetDiskFreeSpaceEx(aParams[0].ToString(), &i64FreeBytesToCaller, &i64TotalBytes, &i64FreeBytes);
		Ret = double(i64FreeBytes.HighPart) * 0x100000000 + double(i64FreeBytes.LowPart);
		break;
	}
	case enMoveFile:
	{
		Ret = MoveFile(aParams[0].ToString(), aParams[1].ToString());
		break;
	}
	case enLockFile:
	case enUnLockFile:
	{
		wxString sFileName = aParams[0].ToString();
		int nStartPos = 0;
		
		if (aParams[1].GetType())
			nStartPos = aParams[1].ToInt();
		
		int nLength = 1;
		
		if (aParams[2].GetType())
			nLength = aParams[2].ToInt();

		HANDLE hFile = 0;
		hFile = aListHandle[StringUtils::MakeUpper(sFileName)];
		if (!hFile)
		{
			hFile = CreateFile(sFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
			aListHandle[StringUtils::MakeUpper(sFileName)] = hFile;
		}

		Ret = 0;
		if (hFile == INVALID_HANDLE_VALUE || hFile == 0)
		{
			CTranslateError::Error("‘айл %s не найден", sFileName);
		}
		else
		{
			bool bRes;
			if (enLockFile == aParams.GetMethodIndex())
			{
				bRes = LockFile(hFile, nStartPos, 0, nLength, 0);//лочим файлы до 4√...
			}
			else
			{
				bRes = UnlockFile(hFile, nStartPos, 0, nLength, 0);
			}

			Ret = 1;

			if (!bRes)
			{
				CTranslateError::Error("‘айлова€ ошибка: %s", GetGetLastStringError());
			}
		}

		break;
	}
	case enUnLockAllFile:
	{
		CloseAllHandle();
		Ret = 1;
		break;
	}
	case enWriteData:
	{
		wxString sFileName = aParams[0].ToString();
		wxString Str = aParams[1].ToString();
		int nPos = aParams[2].ToInt();
		HANDLE hFile = CreateFile(sFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL);
		if (hFile)
		{
			DWORD dwWrite = Str.size();
			SetFilePointer(hFile, nPos, NULL, FILE_BEGIN);
			WriteFile(hFile, Str, dwWrite, &dwWrite, NULL);
			CloseHandle(hFile);
		}
		break;
	}
	case enReadData:
	{
		wxString sFileName = aParams[0].ToString();
		int nPos = aParams[1].ToInt();
		int nLenght = aParams[2].ToInt();
		HANDLE hFile = CreateFile(sFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NULL, NULL);
		if (hFile)
		{
			SetFilePointer(hFile, nPos, NULL, FILE_BEGIN);
			DWORD dwRead;
			wchar_t *pBuf = new wchar_t[nLenght + 1];
			pBuf[nLenght] = 0;
			if (!ReadFile(hFile, pBuf, nLenght, &dwRead, NULL))
			{
				CloseHandle(hFile);
				CTranslateError::Error(ERROR_FILE_READ, sFileName);
			}
			CloseHandle(hFile);
			return pBuf;
		}
		break;
	}
	}
	return Ret;
}

//**********************************************************************
//*                       Runtime register                             *
//**********************************************************************

VALUE_REGISTER(CValueFileSystem, "fs", TEXT2CLSID("VL_FSYS"));