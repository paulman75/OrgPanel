#include "Windows.h"
#include "TrayBar.h"
#include "../Headers/const.h"
#include "../Headers/general.h"

extern HWND hwndMain;
extern HINSTANCE hInstance;		  // Global instance handle for application
int IconMode=ID_MAIN;
BOOL bCicle;
int IconTimerID=0;

NOTIFYICONDATA nd;

HICON hIcon[ID_MAIN+1];
char cEvTit[ID_MAIN+1][20]={"Дело","День рождения","Именины","","","Событие","Дневник"};

void ModifIcon()
{
    nd.cbSize = sizeof(NOTIFYICONDATA);
    nd.hWnd   = hwndMain;
    nd.uID    = 1;
    nd.hIcon  = hIcon[bCicle ? IconMode : ID_MAIN];
    nd.uFlags = NIF_ICON | NIF_TIP;
    lstrcpy(nd.szTip,cEvTit[IconMode]);
    Shell_NotifyIcon(NIM_MODIFY, &nd);
}

void __stdcall OnIconTimer(HWND hwnd,unsigned int msg, unsigned int id, DWORD tim)
{
	bCicle=!bCicle;
	ModifIcon();
}

void SetIconMode(int NewMode)
{
	if (IconTimerID) KillTimer(NULL,IconTimerID);
	IconTimerID=0;
	if (NewMode==IconMode) return;
	bCicle=TRUE;
	IconMode=NewMode;
	ModifIcon();
	if (IconMode!=ID_MAIN)
		IconTimerID=SetTimer(NULL,NULL,500,OnIconTimer);
}

void SetTrayBar()
{
	hIcon[ID_MAIN]=LoadIcon(hInstance,"1MAINICON");
	hIcon[ID_DELA]=LoadIcon(hInstance,"DELAICON");
	hIcon[ID_ROV]=LoadIcon(hInstance,"ROVICON");
	hIcon[ID_EVENT]=LoadIcon(hInstance,"EVENTICON");
	hIcon[ID_IMEN]=LoadIcon(hInstance,"IMENICON");

    nd.cbSize = sizeof(NOTIFYICONDATA);
    nd.hWnd   = hwndMain;
    nd.uID    = 1;
    nd.hIcon  = hIcon[ID_MAIN];
    nd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nd.uCallbackMessage = WM_TRAYBARCALLBACK;
	TDate d=Now();
	DateToString(&d,nd.szTip);
	char b[20];
	WeekDayToString(b,DayofWeek(&d));
	lstrcat(nd.szTip," ");
	lstrcat(nd.szTip,b);
    //lstrcpy(nd.szTip,"Дневник");
    Shell_NotifyIcon(NIM_ADD, &nd);
}

void RemoveIcon()
{
    nd.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, &nd);
}
