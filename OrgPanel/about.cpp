#include "windows.h"
#include "Classes.h"
#include "fltext.h"
#include "ToolBar.h"
#include <commctrl.h>
#define _MYDLL_
#include "../Headers/general.h"

HBITMAP	hAboutBack;
extern HINSTANCE hInstance;

HANDLE	hBitLogo;
HDC		hLogoDC;
BOOL bAni=FALSE;
CFloatText* cc;
CToolBar* abar;
LPCTSTR		Caption;
HINSTANCE	hProcInst;

#define IDC_EMAIL	10
#define IDC_HOME	11
#define IDC_UPDATE  12
#define IDC_EXIT	13

void CloseAbout(HWND hDlg)
{
	delete cc;
	cc=NULL;
	delete abar;
	EndDialog(hDlg, TRUE);
}

void InitAbout(HWND hDlg)
{
	static HWND	hWndToolTip=NULL;
	RECT rc;
	MoveWindow(hDlg,0,0,345,256,FALSE);
	char* mm=(char*)malloc(100);
	WORD i=GetBuild(hProcInst);
	wsprintf(mm,"%s Сборка: %d",Caption,i);
	SetWindowText(hDlg,mm);
	free(mm);
	PutOnCenter(hDlg);
	GetClientRect(hDlg,&rc);
	hWndToolTip=CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_CHILD | WS_POPUP | TTS_ALWAYSTIP, 
          CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, 
          hDlg, NULL, hInstance, NULL);
	abar=new CToolBar(hDlg,0,201,24,TRUE,hWndToolTip);
	abar->AddButton("Написать письмо",	IDC_EMAIL, "T_EMAIL",	TBSTYLE_BUTTON);
	abar->AddButton("Домашняя страничка",	IDC_HOME,"T_HOME",	TBSTYLE_BUTTON);
	abar->AddButton("Проверить обновления",	IDC_UPDATE,"T_UPDATE",	TBSTYLE_BUTTON);
	abar->AddButton("",	IDC_EXIT+1,"",	TBSTYLE_SEP);
	abar->AddButton("Выход",	IDC_EXIT,"T_EXIT",	TBSTYLE_BUTTON);

	cc=new CFloatText(hDlg,0,0,340,200,"OBLAKOBACK",0xffff00);
	cc->SetParam(130,2);
	cc->AddString(10,40,0xff000,0xaa50a,"OrgPanel 3.3");
	cc->AddString(15,30,0xaba2fd,0x3d02aa,"Программист:");
	cc->AddString(10,28,0xffff,0xaa3a2,"Манаенков Павел");
	cc->AddString(10,33,0x76fc45,0xa3a2,"CopyRight (c) 2011 PM_SOFT");
	cc->LoadBitmap2("BLOGO",0,0);
	cc->Go(30);
}

BOOL FAR PASCAL AboutWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	static BOOL b;
    switch (message) {
	case WM_INITDIALOG:
		InitAbout(hDlg);
		return TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam)==IDOK || LOWORD(wParam)==IDCANCEL) 
		{
			CloseAbout(hDlg);
			return TRUE;
	    }
		if (HIWORD(wParam)==BN_CLICKED)
		switch (LOWORD(wParam))
		{
			case IDC_EMAIL:
				ShellExecute(hDlg,"open","mailto:orgpanel@mail.ru",NULL,NULL,SW_SHOW);
				CloseAbout(hDlg);
				return TRUE;
			case IDC_HOME:
				ShellExecute(hDlg,NULL,"http://orgpanel.narod2.ru",NULL,NULL,SW_NORMAL);
			case IDC_EXIT:
				CloseAbout(hDlg);
				return TRUE;
			case IDC_UPDATE:
				CloseAbout(hDlg);
				CheckUpdate(GetParent(hDlg));
				return TRUE;
			break;
		}
		case WM_FLOATTEXTEVENT:
			if (wParam)
			{
				if (b)
				{
					Sleep(100);
					if (!cc) return TRUE;
					cc->ShowBitmap(FALSE);
					cc->Go(30);
				}
				else
				{
				Sleep(500);
				if (!cc) return TRUE;
				cc->MakeSpin(235,120,100,3800,75,3400,0x0ff00,0x09900,"OrgPanel",1,40,30);
				b=TRUE;
				}
			}
			else
			{
				Sleep(500);
				if (!cc) return TRUE;
				cc->ShowBitmap(TRUE);
				cc->MakeSpin(235,120,5,100,5,75,0x0ff00,0x09900,"OrgPanel",1,40,30);
				b=FALSE;
			}
 	}
	return FALSE;
}

void __stdcall ShowAbout(HWND Owner, LPCTSTR s, HINSTANCE hInst)
{
	hProcInst=hInst;
	Caption=s;
	DialogBox(hInstance, "ABOUTBOX", Owner, (DLGPROC)AboutWndProc);
}

