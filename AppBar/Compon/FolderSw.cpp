#include "windows.h"
#include "FolderSw.h"
#include "../../Headers/general.h"
#include <commctrl.h>
#include "../../Headers/const.h"

extern HINSTANCE hInstance;
extern HFONT hMainFont;
extern HPEN hpnColor1,hWhitePen, hGrayPen,hpnBlack;
HBRUSH		hGreenBrushFS;
extern CFolderSwitch*	Switch;
extern HWND MenuWnd;

void CFolderSwitch::SetValue(byte New,byte Max)
{
	FValue=New;
	FMax=Max;
	UpdateValue();
}

LONG FAR PASCAL SwitchObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (Switch) Switch->WindowProc(hwnd,msg, wParam, lParam);
    return(DefWindowProc(hwnd, msg, wParam, lParam));
}

void CFolderSwitch::UpdatePosition(WORD Left,WORD Top,byte Width,byte Height,byte Max,BOOL Small)
{
	FMax=Max;
	byte ButW;
	if (Width>35) ButW=15;
	else ButW=12;
	MoveWindow(hWnd,Left,Top,Width,Height,TRUE);
	MoveWindow(LBut,0,0,ButW,Height,TRUE);
	MoveWindow(RBut,Width-ButW,0,ButW,Height,TRUE);
	ShowWindow(hWnd,SW_SHOW);
}

CFolderSwitch::CFolderSwitch(HWND Owner)
{
	FValue=1;
	FMax=5;
	IncPause = 0;
	LBut = NULL;
	RBut = NULL;
	LPressed = false;
	RPressed = false;
	TimerId = 0;
	hMenu = NULL;
	hToolTip = NULL;
	hWnd = NULL;
	WNDCLASS   wc;
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= "FolderSwitch";
        wc.hInstance        = hInstance;
		wc.hIcon			= NULL;
        wc.hCursor          = (HCURSOR)LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)COLOR_WINDOW;
        wc.style            = 0;
		wc.lpfnWndProc		= SwitchObjectProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        if (!RegisterClass(&wc)) return;
		hGreenBrushFS=CreateSolidBrush(0xc0c000);
	hWnd=CreateWindow("FolderSwitch","",WS_CHILD,0,0,32,32,Owner,NULL,hInstance,NULL);
	LBut=CreateWindow("FolderSwitch","",WS_VISIBLE | WS_CHILD,0,0,32,32,hWnd,NULL,hInstance,NULL);
	RBut=CreateWindow("FolderSwitch","",WS_VISIBLE | WS_CHILD,0,0,32,32,hWnd,NULL,hInstance,NULL);
	Switch=this;
	TimerId=0;
	LPressed=FALSE;
	RPressed=FALSE;
	hMenu=NULL;
InitCommonControls();
	hToolTip=CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_CHILD | WS_POPUP | TTS_ALWAYSTIP, 
            CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, 
             hWnd, NULL, hInstance, NULL);
	SendMessage(hToolTip,TTM_SETDELAYTIME, TTDT_INITIAL,80);
	UpdateToolTip();
}

void CFolderSwitch::DrawLine(BOOL Pressed,HDC dc,HPEN NoPen,HPEN YesPen,int x0,int y0,int x1,int y1,int x2,int y2)
{
if (Pressed) SelectObject(dc,YesPen);
else SelectObject(dc,NoPen);
MoveToEx(dc,x0,y0,NULL);
LineTo(dc,x1,y1);
LineTo(dc,x2,y2);
}

void CFolderSwitch::DrawNumber(HDC hdc)
{
	RECT rect;
	GetClientRect(hWnd,&rect);
	SIZE siz;
	HFONT hf=(HFONT)SelectObject(hdc,hMainFont);
	SetBkMode(hdc,TRANSPARENT);
	char ch;
	if (FValue>9)
	{
		ch=FValue/10+48;
		GetTextExtentPoint32(hdc,&ch,1,&siz);
		TextOut(hdc,1+(rect.right-siz.cx)/2,rect.bottom/2-siz.cy+1,&ch,1);
		ch=mod(FValue,10)+48;
		GetTextExtentPoint32(hdc,&ch,1,&siz);
		TextOut(hdc,1+(rect.right-siz.cx)/2,rect.bottom/2,&ch,1);
	}
	else
	{
		ch=FValue+48;
		GetTextExtentPoint32(hdc,&ch,1,&siz);
		TextOut(hdc,1+(rect.right-siz.cx)/2,(rect.bottom-siz.cy)/2,&ch,1);
	}
	SelectObject(hdc,hf);
}

void CFolderSwitch::DrawButton(HWND hwnd,HDC hdc, BOOL Left)
{
RECT rc;
GetClientRect(hwnd,&rc);
BOOL Press=(Left&&LPressed || !Left&&RPressed);
DrawLine(Press,hdc,hWhitePen,hpnBlack,0,rc.bottom,0,0,rc.right,0);
DrawLine(Press,hdc,hpnColor1,hGrayPen,1,rc.bottom,1,1,rc.right,1);
DrawLine(Press,hdc,hpnBlack,hWhitePen,0,rc.bottom-1,rc.right-1,rc.bottom-1,rc.right-1,0);
DrawLine(Press,hdc,hGrayPen,hpnColor1,1,rc.bottom-2,rc.right-2,rc.bottom-2,rc.right-2,1);
POINT poi[7]; 
LONG delta;
if (Press) delta=2;
else delta=0;
if (Left)
{
	poi[0].x=2+delta;   
	poi[1].x=7+delta;	  
	poi[2].x=7+delta;
	poi[3].x=11+delta;
	poi[4].x=11+delta;
	poi[5].x=7+delta;
	poi[6].x=7+delta;
} else {
	poi[0].x=12+delta;   
	poi[1].x=7+delta;	  
	poi[2].x=7+delta;
	poi[3].x=3+delta;
	poi[4].x=3+delta;
	poi[5].x=7+delta;
	poi[6].x=7+delta;
}
delta=delta+rc.bottom/2;
poi[0].y=delta;
poi[1].y=5+delta;
poi[2].y=2+delta;
poi[3].y=2+delta;
poi[4].y=-2+delta;
poi[5].y=-2+delta;
poi[6].y=-5+delta;
if (rc.bottom<38) 
	for (delta=0; delta<7;delta++)	poi[delta].x=(WORD)(poi[delta].x/1.3);
SelectObject(hdc,hpnBlack);
SelectObject(hdc,hGreenBrushFS);
Polygon(hdc,poi,7);
}

void CFolderSwitch::DeleteToolTip()
{
	if (!SendMessage(hToolTip,TTM_GETTOOLCOUNT,0,0)) return;
	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize=sizeof(ti);
	ti.hwnd=hWnd;
	ti.uId=0;
	SendMessage(hToolTip,TTM_DELTOOL,0,(LPARAM)&ti);
}

void CFolderSwitch::UpdateToolTip()
{
	DeleteToolTip();
	char* ch=(char*)SendMessage(GetParent(hWnd),WM_SWITCHGETFOLDERNAME,FValue,0);
	if (!ch) return;
	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize=sizeof(ti);
	ti.uFlags = TTF_SUBCLASS;  //TTF_SUBCLASS causes the tooltip to automatically subclass the window and look for the messages it is interested in.
	ti.hwnd = hWnd;
	ti.uId = 0;
	ti.lpszText=ch;
	RECT rc;
	GetClientRect(hWnd,&rc);
	ti.rect=rc;
	SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void CFolderSwitch::UpdateValue()
{
	if ((FValue<1) || (FValue>60000)) FValue=FMax;
	if (FValue>FMax) FValue=1;
	InvalidateRect(hWnd,NULL,TRUE);
	SendMessage(GetParent(hWnd),WM_SWITCHVALUECHANGED,(WPARAM)FValue,0);
	UpdateToolTip();
}

CFolderSwitch::~CFolderSwitch()
{
	if (hMenu) DestroyMenu(hMenu);
	DeleteToolTip();
	DestroyWindow(hWnd);
	UnregisterClass("FolderSwitch", hInstance);
}

void CFolderSwitch::IncValue(int Offset)
{
	FValue=FValue+Offset;
	UpdateValue();
}

void CFolderSwitch::InitMenu()
{
	if (hMenu) DestroyMenu(hMenu);
	hMenu=CreatePopupMenu();
	char *temp=new char[200];
	for (int i=1; i<=FMax; i++)
	{
		char* ch=(char*)SendMessage(GetParent(hWnd),WM_SWITCHGETFOLDERNAME,i,0);
		if (ch)	wsprintf(temp,"%d - %s",i,ch);
		else wsprintf(temp,"%d",i);
		AppendMenu(hMenu,MF_STRING ,i,temp);
	}
	delete []temp;
}

LONG FAR PASCAL CFolderSwitch::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	switch (msg)
	{
	case WM_PAINT:
		BeginPaint(hwnd,&ps);
		if (hwnd!=hWnd) DrawButton(hwnd,ps.hdc,hwnd==LBut);
		else DrawNumber(ps.hdc);
		EndPaint(hwnd,&ps);
		break;
	case WM_LBUTTONDOWN:
		if (hwnd!=LBut && hwnd!=RBut) break;
		IncPause=0;
		if (TimerId==0) TimerId=SetTimer(hwnd,1,50,NULL);
		if (hwnd==LBut) 
		{
			LPressed=TRUE;
			IncValue(-1);
		}			
		else 
		{
			RPressed=TRUE;
			IncValue(1);
		}
		MySound();
		InvalidateRect(hwnd,NULL,TRUE);
		break;
	case WM_LBUTTONUP:
		if (hwnd==hWnd) break;
		LPressed=FALSE;
		RPressed=FALSE;
		InvalidateRect(hwnd,NULL,TRUE);
		if (TimerId!=0) 
		{
			if (KillTimer(hwnd,TimerId)) TimerId=0;
		}
		break;
	case WM_RBUTTONUP:
		POINT point;
		GetCursorPos(&point);
		if (MenuWnd) SendMessage(MenuWnd,WM_CANCELMODE,0,0);
		InitMenu();
		TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hWnd, NULL);
		MenuWnd=hWnd;
		break;
	case WM_TIMER:
		POINT	pt;
		GetCursorPos(&pt);
		HWND hh;
		hh=WindowFromPoint(pt);
		if (hh!=hwnd) 
		{
			KillTimer(hwnd,TimerId);
			TimerId=0;
			LPressed=FALSE;
			RPressed=FALSE;
			InvalidateRect(hwnd,NULL,TRUE);
		} 
		else if (IncPause++>1) 
		{
			IncPause=0;
			if (hwnd==LBut) IncValue(-1);
			else IncValue(1);
			MySound();
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam)>0 && LOWORD(wParam)<=(UINT)FMax)
		{
			FValue=LOWORD(wParam);
			UpdateValue();
		}
		break;
	}
return 0;
}
