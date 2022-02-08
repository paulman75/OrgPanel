#include "windows.h"
#include "Classes.h"
#include "Res\Resource.h"
#define _MYDLL_
#include "../Headers/general.h"

extern HINSTANCE	hInstance;		  // Global instance handle for application

CMyString*	sCaption;
CMyString*	sText1;
CMyString*	sText2;
HICON	hIcon;
LPCTSTR	Str;
LPTSTR Str2;
WORD StrCapab;

void __stdcall PutOnCenter(HWND hh)
{
	RECT rc;
	GetWindowRect(hh,&rc);
	int sw=GetSystemMetrics(SM_CXFULLSCREEN);
	int sh=GetSystemMetrics(SM_CYFULLSCREEN);
	int w=rc.right-rc.left;
	int h=rc.bottom-rc.top;
	MoveWindow(hh,(sw-w)/2,(sh-h)/2,w,h,FALSE);
}

BOOL FAR PASCAL DialogWndProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	PAINTSTRUCT ps;
    switch (message) {
	case WM_INITDIALOG:	
		SendMessage(hDlg,WM_SETTEXT,0,(LPARAM)sCaption->Text);
		HWND	hw;
		hw=GetDlgItem(hDlg,IDC_MESSAGELABEL);
		SendMessage(hw,WM_SETTEXT,0,(LPARAM)sText1->Text);
		hw=GetDlgItem(hDlg,IDC_PARAMLABEL);
		SendMessage(hw,WM_SETTEXT,0,(LPARAM)sText2->Text);
		PutOnCenter(hDlg);
		return (TRUE);
	case WM_COMMAND:
		if ( LOWORD(wParam)==IDOK || LOWORD(wParam)==IDNO) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (TRUE);
	    }
	    break;
	case WM_PAINT:
		BeginPaint(hDlg,&ps);
		DrawIcon(ps.hdc,20,20,hIcon);
		EndPaint(hDlg,&ps);
		break;
    }
    return (FALSE);
}

byte __stdcall MessageDlg(HWND Owner,LPCTSTR Text,LPCTSTR Param, LPCTSTR Caption)
{
sCaption=new CMyString(Caption);
sText1=new CMyString(Text);
sText2=new CMyString(Param);
*sText2+=" ?";
hIcon=LoadIcon(NULL,IDI_QUESTION);
	byte res=DialogBox(hInstance, "MESSAGEDIALOG", Owner, (DLGPROC)DialogWndProc);
DeleteObject(hIcon);
delete sText1;
delete sText2;
delete sCaption;
return res;
}

BOOL FAR PASCAL InputWndProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	HWND	hw;
    switch (message) {
	case WM_INITDIALOG:	
		SendMessage(hDlg,WM_SETTEXT,0,(LPARAM)sCaption->Text);
		hw=GetDlgItem(hDlg,IDC_MESSAGELABEL);
		SendMessage(hw,WM_SETTEXT,0,(LPARAM)sText1->Text);
		if (Str)
		{
			hw=GetDlgItem(hDlg,IDC_INPEDIT1);
			SendMessage(hw,WM_SETTEXT,0,(LPARAM)Str);
		}
		PutOnCenter(hDlg);
		return (TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam)==IDOK)
		{
			hw=GetDlgItem(hDlg,IDC_INPEDIT1);
			SendMessage(hw,WM_GETTEXT,StrCapab,(LPARAM)Str2);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (LOWORD(wParam)==IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
	    }
	    break;
    }
    return (FALSE);
}

BOOL __stdcall InputQuery(HWND Owner,LPCTSTR Caption,LPCTSTR Text,LPCTSTR From, LPTSTR ToBuf, WORD BufSize)
{
sCaption=new CMyString(Caption);
sText1=new CMyString(Text);
Str=From;
Str2=ToBuf;
StrCapab=BufSize;
	byte res=DialogBox(hInstance, "INPUTDIALOG", Owner, (DLGPROC)InputWndProc);
delete sText1;
delete sCaption;
return (res==IDOK);
}

