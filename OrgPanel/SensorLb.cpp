#include "windows.h"
#define _MYDLL_
#include "../Headers/general.h"
#include "SensorLb.h"
#include "../Headers/const.h"

extern HINSTANCE hInstance;
extern HFONT hMainFont;
static int iSensorCount=0;

CSensorLabel* FirstSenLab=NULL;
CSensorLabel* slab;

void __stdcall CreateSensorLabel(HWND hwnd, int x, int y, int Col1, int Col2, LPCTSTR Cap, byte Cur, int ID)
{
	if (FirstSenLab)
	{
		slab=FirstSenLab;
		while (slab->next) slab=slab->next;
		slab->next=new CSensorLabel(hwnd,x,y,Col1,Col2,Cap,Cur,ID);
		slab->next->next=NULL;
	}
	else
	{
		FirstSenLab=new CSensorLabel(hwnd,x,y,Col1,Col2,Cap,Cur,ID);
		FirstSenLab->next=NULL;
	}
}

CSensorLabel* FindSenLabel(byte ID)
{
	slab=FirstSenLab;
	while (slab)
	{
		if (GetWindowLong(slab->hWnd,GWL_ID)==ID) return slab;
		slab=slab->next;
	}
	return NULL;
}

void __stdcall ShowSensorLabel(byte ID, BOOL bShow)
{
	slab=FindSenLabel(ID);
	if (!slab) return;
	ShowWindow(slab->hWnd,bShow ? SW_SHOW:SW_HIDE);
}

BOOL __stdcall SetSensorCaption(LPCTSTR Cap, byte ID)
{
	slab=FindSenLabel(ID);
	if (!slab) return FALSE;
	lstrcpy(slab->FCaption,Cap);
	slab->UpdateSize();
	return TRUE;
}

void __stdcall DeleteAllSensorLabels()
{
	while (FirstSenLab)
	{
		slab=FirstSenLab->next;
		delete FirstSenLab;
		FirstSenLab=slab;
	}
}

LONG CALLBACK CSensorLabel::StaticSensorProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CSensorLabel* obj = (CSensorLabel*)GetWindowLong(hwnd, GWL_USERDATA);
    if (obj) obj->SensorProc(msg, wParam, lParam);
    return(DefWindowProc(hwnd, msg, wParam, lParam));
}

CSensorLabel::CSensorLabel(HWND Owner, int XCenter, int YCenter, int Color1, int Color2, LPCTSTR Caption,byte Cursor,int ID)
{
	ColorOff=Color1;
	ColorOn=Color2;
	FXCenter=XCenter;
	FYCenter=YCenter;
	if (Caption) lstrcpy(FCaption,Caption);
	else FCaption[0]=0;
	if (!iSensorCount++)
    {
		WNDCLASS   wc;
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= "SensorLabel";
        wc.hInstance        = hInstance;
		wc.hIcon			= NULL;
		if (Cursor==0) wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		else wc.hCursor=LoadCursor(hInstance,"HANDCUR");
		wc.hbrBackground	= NULL;
        wc.style            = 0;
		wc.lpfnWndProc		= StaticSensorProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        RegisterClass(&wc);
    }
	hWnd=CreateWindow("SensorLabel","",WS_CHILD,0,0,0,0,Owner,NULL,hInstance,NULL);
	SetWindowLong(hWnd, GWL_USERDATA, (long)this);
	SetWindowLong(hWnd,GWL_ID,ID);
	if (hWnd==NULL) return;
	TimerId=0;
	UpdateSize();
	ShowWindow(hWnd,SW_SHOW);
	bDownClick=FALSE;
}

void CSensorLabel::UpdateSize()
{
	HDC hdc=GetDC(hWnd);
	SIZE siz;
	SelectObject(hdc,hMainFont);
	GetTextExtentPoint32(hdc,FCaption,lstrlen(FCaption),&siz);
	ReleaseDC(hWnd,hdc);
	RECT rc;
	GetWindowRect(hWnd,&rc);
	POINT pt={0,0};
	ClientToScreen(GetParent(hWnd),&pt);
	MoveWindow(hWnd,FXCenter-siz.cx/2,FYCenter-siz.cy/2,siz.cx,siz.cy,FALSE);
	rc.left-=pt.x;
	rc.right-=pt.x;
	rc.bottom-=pt.y;
	rc.top-=pt.y;
	InvalidateRect(GetParent(hWnd),&rc,TRUE);
	InvalidateRect(hWnd,NULL,FALSE);
}

CSensorLabel::~CSensorLabel()
{
	if (TimerId!=0) KillTimer(hWnd,TimerId);
	DestroyWindow(hWnd);
	if (!(--iSensorCount)) 
	{
		UnregisterClass("SensorName", hInstance);
	}
}

LONG FAR PASCAL CSensorLabel::SensorProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (((msg==WM_MOUSEMOVE) || (msg==WM_NCHITTEST)) && (TimerId==0))
		{
			TimerId=SetTimer(hWnd,1,100,NULL);
			InvalidateRect(hWnd,NULL,FALSE);
		}
	if (msg==WM_MOUSELEAVE)  
	{
		KillTimer(hWnd,(UINT)TimerId);
		TimerId=0;
		InvalidateRect(hWnd,NULL,FALSE);
		bDownClick=FALSE;
	}
	if (msg==WM_LBUTTONDOWN) bDownClick=TRUE;
	if (msg==WM_LBUTTONUP)
	{
		int ID;
		ID=GetWindowLong(hWnd,GWL_ID);
		if (bDownClick) SendMessage(GetParent(hWnd),WM_SENSORLABELCLICK, (WPARAM)hWnd,ID);
	}
	if (msg==WM_TIMER)
	{
        POINT pt;
		GetCursorPos(&pt);
        if (WindowFromPoint(pt)!=hWnd) PostMessage(hWnd,WM_MOUSELEAVE,0,0);
    }
	if (msg==WM_PAINT)
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd,&ps);
		if (TimerId==0) SetTextColor(ps.hdc,ColorOff);
		else			SetTextColor(ps.hdc,ColorOn);
		SetBkMode(ps.hdc,TRANSPARENT);
		SelectObject(ps.hdc,hMainFont);
		TextOut(ps.hdc,0,0,FCaption,lstrlen(FCaption));
		EndPaint(hWnd,&ps);
	}
	return 0;
}
