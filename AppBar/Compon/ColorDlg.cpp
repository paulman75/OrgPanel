#include "windows.h"
#include "ColorDlg.h"
#include "..\..\Headers\const.h"

extern HINSTANCE hInstance;
static int ColorDlgCounts=0;

CColorDlg::CColorDlg(HWND Parent, int x, int y, COLORREF col)
{
FColor=col;
if (!ColorDlgCounts++)
{
	WNDCLASS   wc;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= "MyColorDlg";
	wc.hInstance        = hInstance;
	wc.hIcon			= NULL;
	wc.hCursor          = (HCURSOR)LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.style            = 0;
	wc.lpfnWndProc		= ObjectProc;
	wc.cbClsExtra       = 0;
	wc.cbWndExtra       = 0;
	if (!RegisterClass(&wc)) return;
    }
hWnd=CreateWindow("MyColorDlg","",WS_DLGFRAME | WS_CHILD,x,y,20,20,Parent,NULL,hInstance,NULL);
SetWindowLong(hWnd, GWL_USERDATA, (long)this);
ShowWindow(hWnd,SW_SHOW);
};

CColorDlg::~CColorDlg()
{
	DestroyWindow(hWnd);
	if (!--ColorDlgCounts)
	{
		UnregisterClass("MyColorDlg", hInstance);
	}
}

void CColorDlg::Execute()
{
	CHOOSECOLOR cc;
	cc.lStructSize=sizeof(cc);
	cc.hInstance=0;
	cc.hwndOwner=hWnd;
	cc.Flags=CC_FULLOPEN | CC_RGBINIT;
	cc.rgbResult=FColor;
	COLORREF cr[16];
	for (byte i=0; i<16; i++) cr[i]=0xffffff;
	cc.lpCustColors=&cr[0];
	if (ChooseColor(&cc))
	{
		FColor=cc.rgbResult;
		InvalidateRect(hWnd,NULL,TRUE);
		SendMessage(GetParent(hWnd),WM_COLORDIALOGCHANGE,0,0);
	}
}

LRESULT CALLBACK CColorDlg::ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
CColorDlg         *dlg = (CColorDlg *)GetWindowLong(hwnd, GWL_USERDATA);
	switch (msg)
	{
	case WM_ERASEBKGND:
		HBRUSH hb;
		hb=CreateSolidBrush(dlg->FColor);
		RECT rc;
		GetClientRect(hwnd,&rc);
		FillRect((HDC)wParam,&rc,hb);
		DeleteObject(hb);
		break;
	case WM_LBUTTONUP:
		dlg->Execute();
		break;
    default: 
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 1;
}

void CColorDlg::SetColor(COLORREF col)
{
	FColor=col;
	InvalidateRect(hWnd,NULL,TRUE);
}
