#include "windows.h"
#include "Dialogs.h"
#include "Res\Resource.h"

extern HINSTANCE	hInstance;		  // Global instance handle for application

void PutOnCenter(HWND hh)
{
	RECT rc;
	GetWindowRect(hh,&rc);
	int sw=GetSystemMetrics(SM_CXFULLSCREEN);
	int sh=GetSystemMetrics(SM_CYFULLSCREEN);
	int w=rc.right-rc.left;
	int h=rc.bottom-rc.top;
	MoveWindow(hh,(sw-w)/2,(sh-h)/2,w,h,FALSE);
}

char bb[100];
LPCTSTR Cap;

BOOL FAR PASCAL InputWndProc(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
{
	HWND	hw;
    switch (message) {
	case WM_INITDIALOG:	
		SendMessage(hDlg,WM_SETTEXT,0,(LPARAM)Cap);
		hw=GetDlgItem(hDlg,IDC_MESSAGELABEL);
		SendMessage(hw,WM_SETTEXT,0,(LPARAM)Cap);
		hw=GetDlgItem(hDlg,IDC_INPEDIT1);
		SendMessage(hw,WM_SETTEXT,0,(LPARAM)bb);
		PutOnCenter(hDlg);
		return (TRUE);
	case WM_COMMAND:
		if ( LOWORD(wParam)==IDOK || LOWORD(wParam)==IDCANCEL) 
		{
			hw=GetDlgItem(hDlg,IDC_INPEDIT1);
			SendMessage(hw,WM_GETTEXT,100,(LPARAM)bb);
			EndDialog(hDlg, LOWORD(wParam));
			return (TRUE);
	    }
	    break;
    }
    return (FALSE);
}

BOOL InputQuery(HWND Owner,LPCTSTR Caption,LPCTSTR Text,LPTSTR buf)
{
	lstrcpy(bb,Text);
	Cap=Caption;
	byte res=DialogBox(hInstance, "INPUTDIALOG", Owner, (DLGPROC)InputWndProc);
	if (res) lstrcpy(buf,bb);
	return (res==IDOK);
}


