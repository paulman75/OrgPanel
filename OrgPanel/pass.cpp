#include "windows.h"
#define _MYDLL_
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "res/resource.h"
#include "RegUnit.h"

#define NOBAR "Эту программу можно использовать только совместно с программой OrgPanel."
extern HINSTANCE	hInstance;

BOOL	bUsePass;
char	pas[20];

void __stdcall UsePassword()
{
	HWND hBar=FindWindow("BarWindow",NULL);
	if (hBar) SendMessage(hBar,WM_PASSWORDUSED,0,0);
}

BOOL FAR PASCAL PasswordWndProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	HWND Win;
    switch (message)
	{
	case WM_INITDIALOG:
		Win=GetDlgItem(hDlg,IDC_PASSEDIT1);
		SendMessage(Win,WM_SETTEXT,0,0);
		PutOnCenter(hDlg);
		return (TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam)==IDOK)
		{
			Win=GetDlgItem(hDlg,IDC_PASSEDIT1);
			char buf[20];
			SendMessage(Win,WM_GETTEXT,20,(LPARAM)&buf);
			BOOL b=(lstrcmp((LPCTSTR)&buf,pas)==0);
			if (!b) MessageBox(hDlg,"Неверный пароль","Ошибка",MB_ICONSTOP);
			EndDialog(hDlg,b);
			return (TRUE);
		}
		if (LOWORD(wParam)==IDCANCEL) 
		{
			EndDialog(hDlg, FALSE);
			return (TRUE);
	    }
	}
	return FALSE;
}

BOOL __stdcall CheckPassword(BOOL bFromBar, HWND Owner)
{
	if (!bFromBar)
	{
		HWND hBar=FindWindow("BarWindow",NULL);
		if (hBar)
		{
			if (SendMessage(hBar,WM_CHECKPASSWORD,0,0)==OK_PASS) return TRUE;
		}
	}
	if (!ReadRegistry("UsedPassword",(LPBYTE)&bUsePass,4))
	{
		MessageBox(0,NOBAR,"Ошибка",MB_ICONSTOP);
		return FALSE;
	}
	ReadRegistry("Password",(LPBYTE)pas,20);
	byte i=0;
	while (pas[i]) 
	{
		pas[i]+=20;
		i++;
	}
	if (!bUsePass || !pas[0]) return TRUE;
	BOOL res=DialogBox(hInstance, "PASSWORDDIALOG", Owner, (DLGPROC)PasswordWndProc);
	if (res) UsePassword();
	return res;
}
