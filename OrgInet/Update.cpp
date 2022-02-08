#include "windows.h"
#include "../Headers/general.h"
#include "Main.h"


const char* AppName[8]={"Блокнот","Всплывающая панелька",
"Записная книжка","Основная библиотека","Дополнительная библиотека",
"Дневник","Хранитель паролей","Конвертер"};

const char* Server[UPDATESERVER_COUNT]={
	"orgpanel.narod2.ru",
		""
	};
const char* File[UPDATESERVER_COUNT]={
	"http://orgpanel.narod2.ru/OrgPanel.upd",
		""
	};

UPDATEINFO UpdateInfo;
char* pInfo;
int recsize;

extern int CurrentServer;
extern HWND g_hDlg;
extern HINSTANCE hInstance;
extern int Progress;
extern HANDLE hDNS;
extern DWORD IP;
extern char hostentBuf[MAXGETHOSTSTRUCT];
extern SOCKET s;
extern int Port;
extern BOOL bTime;
extern HWND hOKBut;
extern BOOL bAutoShow;

int GetBuildFromFile(char* name)
{
	HMODULE hm=LoadLibrary(name);
	if (!hm) return -1;
	int res=GetBuild(hm);
	FreeLibrary(hm);
	return res;
}

void OnEndUpdateRequest(BOOL bOK)
{
	if (bOK)
	{
		int lBuild[6];
		lBuild[A_NOTEBOOK]=GetBuildFromFile("NoteBook.exe");
		lBuild[A_NOTEPAD]=GetBuildFromFile("NotePad.exe");
		lBuild[A_APPBAR]=GetBuildFromFile("OrgPanel.exe");
		lBuild[A_DIARY]=GetBuildFromFile("Diary.exe");
		lBuild[A_ORGPANEL]=GetBuildFromFile("OrgPanel.dll");
		lBuild[A_ORGINET]=GetBuildFromFile("OrgInet.dll");
		lBuild[A_PASSKEAPER]=GetBuildFromFile("PassKeeper.exe");
		lBuild[A_INITCONV]=GetBuildFromFile("UnitConv.exe");

		AddTextToRichEdit(ID_RICH,"\r\nФайл информации успешно получен\r\n");
		BOOL bOld=FALSE;
		for (int i=0; i<8; i++)
		if (lBuild[i]<UpdateInfo.Build[i])
		{
			bOld=TRUE;
			AddTextToRichEdit(ID_RICH,"Модуль \"");
			AddTextToRichEdit(ID_RICH,(char*)AppName[i]);
			AddTextToRichEdit(ID_RICH,"\" обновился\r\n");
		}
		if (bOld)
		{
			AddTextToRichEdit(ID_RICH,"\r\n");
			AddTextToRichEdit(ID_RICH,UpdateInfo.Text);
			AddTextToRichEdit(ID_RICH,"\r\n\r\nСсылки для скачивания:\r\n");
			if (lstrlen(UpdateInfo.Url[0]))
			{
				AddTextToRichEdit(ID_RICH,UpdateInfo.Url[0]);
				AddTextToRichEdit(ID_RICH,"\r\n");
			}
			if (lstrlen(UpdateInfo.Url[1]))
			{
				AddTextToRichEdit(ID_RICH,UpdateInfo.Url[1]);
				AddTextToRichEdit(ID_RICH,"\r\n");
			}
			if (lstrlen(UpdateInfo.Url[2]))
			{
				AddTextToRichEdit(ID_RICH,UpdateInfo.Url[2]);
				AddTextToRichEdit(ID_RICH,"\r\n");
			}
			AddTextToRichEdit(ID_RICH,"\r\n");
			AddTextToRichEdit(ID_RICH,UpdateInfo.Prim);
		}
		else AddTextToRichEdit(ID_RICH,"Обновлений не найдено\r\n");
		SetWindowText(hOKBut,"OK");
	}
	else
	{
		CurrentServer++;
		if (CurrentServer==UPDATESERVER_COUNT)
		{
			AddTextToRichEdit(ID_RICH,"Невозможно проверить обновления\r\n");
			SetWindowText(hOKBut,"OK");

		}
		else ServerStart((char*)Server[CurrentServer]);
	}
}

void ServerUpdateRead(int err)
{
	if (!g_hDlg) return;
	if (err)
	{
		ServerStop(eStatusNotResponding);
	}
	else
	{
		char *ss=(char*)malloc(6000);
		FillMemory(ss,6000,0);
		err = recv(s, (char FAR *)ss, 6000, 0);
		if (err)
		{
			if (err>sizeof(UPDATEINFO)) err=sizeof(UPDATEINFO);
			memcpy(pInfo,ss,err);
			pInfo+=err;
			recsize+=err;
			free(ss);
			if (UpdateInfo.Version!=31)
			{
				AddTextToRichEdit(ID_RICH,"Неверная версия информационного файла\r\n");
				ServerStop(eLogic);
				return;
			}
			if (recsize==sizeof(UPDATEINFO))
			{
				ServerStop(eStatusUsed);
				return;
			}
		}
		else
		{
			free(ss);
			ServerStop(eStatusNotResponding);
		}
	}
}

void MakeUpdateStart()
{
	pInfo=(char*)&UpdateInfo;
	recsize=0;
	ServerStart((char*)Server[CurrentServer]);
}

BOOL __stdcall InetCheckUpdate(HWND hwnd)
{
    WSADATA WSAData;

	if (WSAStartup(0x101, &WSAData)) return TRUE;

	Progress=eProgressNot;
	CurrentServer=0;
	bAutoShow=TRUE;
	Port=80;
	bTime=FALSE;
	return InitUpdate(hwnd);
}
