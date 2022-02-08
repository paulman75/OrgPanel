#include <windows.h>
#include "textf.h"
#include "fltext.h"
#define _MYDLL_
#include "../Headers/general.h"
#include "math.h"

extern HINSTANCE hInstance;
static byte TextCount=0;

LPString nst,mst;

CONST LOGFONT fllf = {
    -72,                        //int   lfHeight;
    0,                         //int   lfWidth;
    0,                         //int   lfEscapement;
    0,                         //int   lfOrientation;
    FW_DONTCARE,               //int   lfWeight;
    0,                         //BYTE  lfItalic;
    0,                         //BYTE  lfUnderline;
    0,                         //BYTE  lfStrikeOut;
    RUSSIAN_CHARSET,              //BYTE  lfCharSet;
    OUT_DEFAULT_PRECIS,        //BYTE  lfOutPrecision;
    CLIP_DEFAULT_PRECIS,       //BYTE  lfClipPrecision;
    DEFAULT_QUALITY,           //BYTE  lfQuality;
    DEFAULT_PITCH | FF_SWISS,  //BYTE  lfPitchAndFamily;
    "Comic Sans MS",           //BYTE  lfFaceName[LF_FACESIZE];
    };

BOOL CFloatText::ThreadFunc()
{
	Work=TRUE;
	while (!NeedStop)
	{
		if (bSpin)
		{
			if (!wFrames)
			{
				sw1=sw2;
				sh1=sh2;
				Angle=0;
				break;
			}
			else
			{
				sw1+=dw;
				sh1+=dh;
				Angle+=dl;
				wFrames--;
				RePaint();
			}
		}
		else
		{
			FirstY+=FSmallStep;
			RePaint();
		}
		Paint(NULL);
		Sleep(FTimerInterval);
	}
	Work=FALSE;
	if (!bSpin) RePaint();
	Paint(NULL);
	PostMessage(GetParent(hwnd),WM_FLOATTEXTEVENT,bSpin,0);
	return 0;
}

DWORD WINAPI ChildThread(LPVOID lpvThreadParm)
{
	CFloatText* self=(CFloatText*) lpvThreadParm;
	self->ThreadFunc();
	return 0;
}

BOOL CFloatText::Start()
{
	DWORD Id;
	NeedStop=FALSE;
	hHandle=CreateThread(NULL,0,ChildThread,this,0,&Id);
	return TRUE;
}

BOOL CFloatText::Go(WORD TimerInterval)
{
	if (Work) return FALSE;
	FTimerInterval=TimerInterval;
	FirstY=-30;
	LPString ps=St;
	while(ps)
	{
		FirstY-=ps->FontH+ps->YDelta;
		ps=ps->Next;
	}
	RePaint();
	Paint(NULL);
	bSpin=FALSE;
	return Start();
}

void CFloatText::ClearPoints()
{
	if (dwTop) GlobalFree(lpTop);
	if (dwBot) GlobalFree(lpBot);
	lpTop=NULL;
	lpBot=NULL;
	// Allocate top guide line
	dwTop = 2;
	dwBot = 2;
	lpTop = (LPPOINT)GlobalAlloc(GPTR, sizeof(POINT) * 2);
	if (!lpTop) return;
			  
	// Allocate bottom guide line
	lpBot = (LPPOINT)GlobalAlloc(GPTR, sizeof(POINT) * 2);
	if (!lpBot)
	{
		GlobalFree(lpTop);
		return;
	}
}

void CFloatText::RePaint()
{
	RECT rc;
	GetClientRect(hwnd,&rc);

	if (hFlTextBack) FillBackGround(hMemDC,&rc,hFlTextBack);
	else
	{
		HBRUSH hbr=CreateSolidBrush(BackCol);
		FillRect(hMemDC,&rc,hbr);
		DeleteObject(hbr);
	}

	if (Work && !bSpin)
	{
		LPString ps=St;
		int h;
		int y=FirstY;
		while(ps)
		{
			ClearPoints();
			float kw=(float)Ftopw/rc.bottom;
			h=(int)((float)ps->FontH*(1-(float)y/rc.bottom));
			if (h<0) h=0;
			if (y<0) h=ps->FontH;
			lpBot[0].x=(int)(kw*y);
			lpBot[1].x=(int)(rc.right-kw*y);
			lpBot[0].y=lpBot[1].y=rc.bottom-y;
			y+=h;
			lpTop[0].x=(int)(kw*y);
			lpTop[1].x=(int)(rc.right-kw*y);
			lpTop[0].y=lpTop[1].y=rc.bottom-y;
	
			if (y>0)
			{
				if (!FillOut(&lpTop, &dwTop)) return;
				if (!FillOut(&lpBot, &dwBot)) return;

				SelectObject(hMemDC,hFont);
				TextEffect(hMemDC,lpTop,lpBot,dwTop,dwBot,ps->St,ps->OColor, ps->Color);
			}
			y+=(int)((float)ps->YDelta*(1-(float)y/rc.bottom));
			ps=ps->Next;
		}
		if (h==0)	NeedStop=TRUE;
	}
	if (Work && bSpin)
	{
		ClearPoints();
		double r=sqrt(sh1*sh1+sw1*sw1);
		double a1=atan2(sw1,sh1);
		double a2=atan2(-sw1,sh1);
		double a3=atan2(sw1,-sh1);
		double a4=atan2(-sw1,-sh1);
		lpBot[0].x=(int)(r*sin(a2+Angle))+xs;
		lpBot[1].x=(int)(r*sin(a1+Angle))+xs;
		lpBot[0].y=(int)(r*cos(a2+Angle))+ys;
		lpBot[1].y=(int)(r*cos(a1+Angle))+ys;

		lpTop[0].x=(int)(r*sin(a4+Angle))+xs;
		lpTop[1].x=(int)(r*sin(a3+Angle))+xs;
		lpTop[0].y=(int)(r*cos(a4+Angle))+ys;
		lpTop[1].y=(int)(r*cos(a3+Angle))+ys;
		if (!FillOut(&lpTop, &dwTop)) return;
		if (!FillOut(&lpBot, &dwBot)) return;
		SelectObject(hMemDC,hFont);
		TextEffect(hMemDC,lpTop,lpBot,dwTop,dwBot,SpinSt->St,SpinSt->OColor, SpinSt->Color);
	}
	if (BitVisible && hBitmap)
	{
		HDC dc2=CreateCompatibleDC(hMemDC);
		SelectObject(dc2,hBitmap);
		BITMAPINFOHEADER bi;
		GetObject(hBitmap,sizeof(bi),(LPVOID)&bi);
		BitBlt(hMemDC,xBit,yBit,bi.biWidth,bi.biHeight,dc2,0,0,SRCCOPY);
		DeleteDC(dc2);
	}
}

void CFloatText::Paint(HDC dc)
{
	HDC dc1=dc;
	if (!dc) dc1=GetDC(hwnd);
	RECT rc;
	GetClientRect(hwnd,&rc);
	BitBlt(dc1,0,0,rc.right,rc.bottom,hMemDC,0,0,SRCCOPY);
	if (!dc) ReleaseDC(hwnd,dc1);
}

CFloatText::CFloatText(HWND Owner, int x, int y, WORD w, WORD h, LPCTSTR BackBitmap, COLORREF BackColor)
{
         
	hFont = CreateFontIndirect(&fllf); 
	St=NULL;
	SpinSt=NULL;
	BackCol=BackColor;
	BitVisible=FALSE;
	Work=FALSE;
	hBitmap=NULL;
	lpTop=NULL;
	lpBot=NULL;
	if (!TextCount++)
	{
		WNDCLASS   wc;

		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= "FloatTextWindow";
		wc.hInstance        = hInstance;
		wc.hIcon			= NULL;
		wc.hCursor          = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground	= CreateSolidBrush(BackColor);
		wc.style            = 0;
		wc.lpfnWndProc		= WndProc;
		wc.cbClsExtra       = 0;
		wc.cbWndExtra       = 0;
		if (!RegisterClass(&wc)) return;
	}
	if (BackBitmap)	hFlTextBack=LoadBitmap(hInstance,BackBitmap);
	else hFlTextBack=NULL;
	hHandle=NULL;
    hwnd=CreateWindowEx(0,"FloatTextWindow",
		"", WS_CHILD, x, y, w, h, Owner, NULL, hInstance, this);
	HDC dc=GetDC(0);
	hMemDC=CreateCompatibleDC(dc);
	hMemBit=CreateCompatibleBitmap(dc,w,h);
	ReleaseDC(0,dc);
	SelectObject(hMemDC,hMemBit);
	RePaint();

	ShowWindow(hwnd,SW_SHOW);
	UpdateWindow(hwnd);

}

LRESULT CALLBACK CFloatText::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
CFloatText         *text = (CFloatText *)GetWindowLong(hwnd, GWL_USERDATA);
LPCREATESTRUCT pcs;

switch(msg)
   {

   case WM_NCCREATE:
      pcs = (LPCREATESTRUCT)lParam;
      text = (CFloatText *)pcs->lpCreateParams;
      SetWindowLong(hwnd, GWL_USERDATA, (long)text);
      break;
   case WM_DESTROY:
      text->Free();
      break;
   case WM_PAINT:
	   PAINTSTRUCT ps;
	   BeginPaint(hwnd,&ps);
	   SelectObject(ps.hdc,text->hFont);
	   text->Paint(ps.hdc);
	   EndPaint(hwnd,&ps);
	case WM_ERASEBKGND:
		return 0;
		break;
      }
return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CFloatText::AddString(WORD YDelta, WORD FontHeight, COLORREF Col, COLORREF OCol, LPCTSTR st)
{
	mst=(LPString)malloc(sizeof(String));
	mst->Next=St;
	St=mst;
	mst->FontH=FontHeight;
	mst->YDelta=YDelta;
	mst->Color=Col;
	mst->OColor=OCol;
	mst->St=(char*)malloc(lstrlen(st)+1);
	lstrcpy(mst->St,st);
}

void CFloatText::SetParam(WORD topw, WORD SmallStep)
{
	FreeSt(&St);
	Ftopw=topw;
	FSmallStep=SmallStep;
}

void CFloatText::FreeSt(LPString* lpst)
{
	while (*lpst)
	{
		mst=(*lpst)->Next;
		free ((*lpst)->St);
		free (*lpst);
		*lpst=mst;
	}
}

BOOL CFloatText::Stop()
{
	NeedStop=TRUE;
	if (hHandle)
	{
		DWORD dw;
		GetExitCodeThread(hHandle,&dw);
		if (dw==STILL_ACTIVE) WaitForSingleObject(hHandle,INFINITE);
		hHandle=0;
	}
	return TRUE;
}

void CFloatText::Free()
{
	Stop();
	DeleteObject(hFont);
	FreeSt(&St);
	FreeSt(&SpinSt);
}

CFloatText::~CFloatText()
{
	Free();
	DestroyWindow(hwnd);
	if (!--TextCount) UnregisterClass("FloatTextWindow",hInstance);
	if (hFlTextBack) DeleteObject(hFlTextBack);
	if (hBitmap) DeleteObject(hBitmap);
	hBitmap=NULL;
	if (hMemBit) DeleteObject(hMemBit);
	hMemBit=NULL;
	if (hMemDC) DeleteDC(hMemDC);
	hMemDC=NULL;
	if (dwTop) GlobalFree(lpTop);
	if (dwBot) GlobalFree(lpBot);
}

void CFloatText::LoadBitmap2(LPCTSTR Name, int xb, int yb)
{
	if (hBitmap) DeleteObject(hBitmap);
	hBitmap=LoadBitmap(hInstance,Name);
	xBit=xb;
	yBit=yb;
}

void CFloatText::ShowBitmap(BOOL Show)
{
	if (!hBitmap) return;
	BitVisible=Show;
	InvalidateRect(hwnd,NULL,FALSE);
}

BOOL CFloatText::MakeSpin(int x, int y, WORD w1, WORD w2, WORD h1, WORD h2, COLORREF Col, COLORREF oCol, LPCTSTR st, byte spins, WORD Frames, WORD TimerInterval)
{
	if (Work) return FALSE;
	FTimerInterval=TimerInterval;
	FreeSt(&SpinSt);
	SpinSt=(LPString)malloc(sizeof(String));
	SpinSt->Next=NULL;
	SpinSt->Color=Col;
	SpinSt->OColor=oCol;
	SpinSt->St=(char*)malloc(lstrlen(st)+1);
	lstrcpy(SpinSt->St,st);
	xs=x;
	ys=y;
	sw1=w1;
	sw2=w2;
	sh1=h1;
	sh2=h2;
	dl=(float)((float)spins*2*3.1415)/Frames;
	dh=((float)h2-h1)/Frames;
	dw=((float)w2-w1)/Frames;
	Angle=0;
	wFrames=Frames;
	bSpin=TRUE;
	Start();
	return TRUE;
}
