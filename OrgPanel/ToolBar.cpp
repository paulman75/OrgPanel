#include "windows.h"
#include "ToolBar.h"
#include <commctrl.h>
#define _MYDLL_
#include "../Headers/general.h"

extern HINSTANCE hInstance;
PToolButton nBut,mBut;
static int ToolBarCounts=0;
extern HPEN hGrayPen, hWhitePen;

BOOL CToolBar::FAddButton(LPCTSTR Tips, WORD ID, HBITMAP hBit, char ButType, BOOL Visible,BOOL Enable, BOOL NeedDel)
{
	FindButton(ID,&nBut,&mBut);
	if (nBut) return FALSE;
	nBut=(PToolButton)malloc(sizeof(CToolButton));
	nBut->hBit=hBit;
	nBut->NeedDelete=NeedDel;
	nBut->ID=ID;
	nBut->Visible=Visible;
	nBut->Enable=Enable;
	nBut->Next=NULL;
	if (mBut)	mBut->Next=nBut;
	else FirstButton=nBut;
	nBut->State=2;
	nBut->ButType=ButType;
	nBut->Tips=(char*)malloc(lstrlen(Tips)+1);
	lstrcpy(nBut->Tips,Tips);
	UpdateBar();
	return TRUE;
}

BOOL CToolBar::AddButton(LPCTSTR Tips, WORD ID, LPCTSTR BitRes, char ButType, BOOL Visible, BOOL Enable)
{
	return FAddButton(Tips,ID,LoadBitmap(hInstance,BitRes),ButType,Visible,Enable,TRUE);
}

BOOL CToolBar::AddButton(LPCTSTR Tips, WORD ID, HBITMAP hBit, char ButType, BOOL Visible, BOOL Enable)
{
	return FAddButton(Tips,ID,hBit,ButType,Visible,Enable,FALSE);
}

BOOL  CToolBar::DeleteButton(WORD ID)
{
	FindButton(ID,&nBut,&mBut);
	if (!nBut) return FALSE;
	if (mBut) mBut->Next=nBut->Next;
	else FirstButton=nBut->Next;
	if (ActiveButton==nBut) ActiveButton=NULL;
	if (ClickButton==nBut) ClickButton=NULL;
	free(nBut->Tips);
	if (nBut->NeedDelete) DeleteObject(nBut->hBit);
	free(nBut);
	UpdateBar();
	return TRUE;
}

LPCTSTR CToolBar::GetButtonTips(WORD ID)
{
	FindButton(ID,&nBut,&mBut);
	if (!nBut) return NULL;
	return nBut->Tips;
}

BOOL CToolBar::ShowButton(WORD ID,BOOL Visible)
{
	FindButton(ID,&nBut,&mBut);
	if (!nBut) return FALSE;
	if (nBut->Visible==Visible) return TRUE;
	nBut->Visible=Visible;
	UpdateBar();
	return TRUE;
}

BOOL CToolBar::EnableButton(WORD ID, BOOL Enable)
{
	FindButton(ID,&nBut,&mBut);
	if (!nBut) return FALSE;
	if (nBut->Enable==Enable) return TRUE;
	nBut->Enable=Enable;
	DrawButton(nBut,NULL);
	return TRUE;
}

BOOL CToolBar::GetEnableButton(WORD ID)
{
	FindButton(ID,&nBut,&mBut);
	if (!nBut) return FALSE;
	return nBut->Enable;
}

BOOL CToolBar::UpdateParam(WORD ID, HBITMAP hBit, LPCTSTR Tips)
{
	FindButton(ID,&nBut,&mBut);
	if (!nBut) return FALSE;
	if (hBit)  nBut->hBit=hBit;
	if (Tips)
	{
		free(nBut->Tips);
		nBut->Tips=(char*)malloc(lstrlen(Tips)+1);
		lstrcpy(nBut->Tips,Tips);
	}
	UpdateBar();
	return TRUE;
}

void CToolBar::FindButton(WORD ID,PToolButton *ppBut, PToolButton *ppPrev)
{
	*ppPrev=NULL;
	*ppBut=FirstButton;
	while (*ppBut && (*ppBut)->ID!=ID)
	{
		*ppPrev=*ppBut;
		*ppBut=(*ppBut)->Next;
	}
}

CToolBar::CToolBar(HWND Owner, WORD x, WORD y,char ButSize, BOOL Horz, HWND hToolTip)
{
FButSize=ButSize;
FHorz=Horz;
FirstPos=0;
KolTips=0;
FirstButton=NULL;
TimerID=0;
Pressed=FALSE;
hToolTipWnd=hToolTip;
if (!ToolBarCounts++)
{
	WNDCLASS   wc;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= "MyToolBar";
	wc.hInstance        = hInstance;
	wc.hIcon			= NULL;
	wc.hCursor          = (HCURSOR)LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wc.style            = 0;
	wc.lpfnWndProc		= ObjectProc;
	wc.cbClsExtra       = 0;
	wc.cbWndExtra       = 0;
	if (!RegisterClass(&wc)) return;
    }
WORD w,h;
if (FHorz)
{
	w=1240;			h=FButSize+8;
}
else
{
	w=FButSize+8;	h=1200;
}
hWnd=CreateWindow("MyToolBar","",WS_CHILD,x,y,w,h,Owner,NULL,hInstance,NULL);
SetWindowLong(hWnd, GWL_USERDATA, (long)this);
UpdateBar();
ShowWindow(hWnd,SW_SHOW);
};

CToolBar::~CToolBar()
{
	DeleteToolTips();
	while (FirstButton)
	{
		nBut=FirstButton->Next;
		free(FirstButton->Tips);
		free(FirstButton);
		FirstButton=nBut;
	}
	DestroyWindow(hWnd);
	if (!--ToolBarCounts)
	{
		UnregisterClass("MyToolBar", hInstance);
	}
};

void CToolBar::DeleteToolTips()
{
	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize=sizeof(ti);
	ti.hwnd=hWnd;
	while (KolTips)
	{
		ti.uId=KolTips--;
		SendMessage(hToolTipWnd,TTM_DELTOOL,0,(LPARAM)&ti);
	}
}

void CToolBar::UpdateBar()
{
	char dx,dy;
	WORD x,y;
	DeleteToolTips();
	if (FHorz)
	{
		dx=1;			dy=0;
		x=FirstPos+4;		y=4;
	}
	else
	{
		dx=0;			dy=1;
		x=4;			y=FirstPos+4;
	}
	nBut=FirstButton;
	while (nBut)
	{
		nBut->x=x;
		nBut->y=y;
		char dd=(nBut->ButType==TBSTYLE_SEP) ? FButSize/2+8 : FButSize+8;
		if (!nBut->Visible) dd=0;
		x+=dd*dx;
		y+=dd*dy;
		if (nBut->ButType==TBSTYLE_BUTTON)
		{
			RECT rc;
			GetButtonRect(nBut,&rc);
			TOOLINFO ti;
			ZeroMemory(&ti, sizeof(ti));
			ti.cbSize=sizeof(ti);
			ti.uFlags = TTF_SUBCLASS;  //TTF_SUBCLASS causes the tooltip to automatically subclass the window and look for the messages it is interested in.
			ti.hwnd = hWnd;
			ti.uId = KolTips++;
			ti.lpszText=nBut->Tips;
			ti.rect=rc;
			SendMessage(hToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
		}
		nBut=nBut->Next;
	}
	InvalidateRect(hWnd,NULL,TRUE);
	ActiveButton=NULL;
};

void DrawLine(HDC dc,LRESULT State,HPEN NoPen,HPEN YesPen,long x0,long y0,long x1,long y1,long x2,long y2)
{
if (State) SelectObject(dc,YesPen);
else SelectObject(dc,NoPen);
MoveToEx(dc,x0,y0,NULL);
LineTo(dc,x1,y1);
LineTo(dc,x2,y2);
}

PToolButton CToolBar::DetectButton(int x,int y)
{
	nBut=FirstButton;
	while (nBut)
	{
		RECT rc;
		GetButtonRect(nBut,&rc);
		if (nBut->ButType==TBSTYLE_BUTTON && nBut->Visible && nBut->Enable && x>=rc.left && x<=rc.right && y>=rc.top && y<=rc.bottom) return nBut;
		nBut=nBut->Next;
	}
	return NULL;
}

void CToolBar::GetButtonRect(PToolButton b, LPRECT rc)
{
	rc->left=b->x;
	rc->top=b->y;
	rc->bottom=rc->top+FButSize;
	rc->right=rc->left+FButSize;
}

void CToolBar::DrawButton(PToolButton b, HDC hdc)
{
	BOOL c=!hdc;
	if (c) hdc=GetDC(hWnd);
	RECT rc;
	GetButtonRect(b,&rc);
	FillRect(hdc,&rc,(HBRUSH)COLOR_WINDOW);
	if (b->State!=2)
	{
		DrawLine(hdc,b->State,hWhitePen,hGrayPen,rc.left,rc.bottom-1,rc.left,rc.top,rc.right-1,rc.top);
		DrawLine(hdc,b->State,hGrayPen,hWhitePen,rc.left,rc.bottom-1,rc.right-1,rc.bottom-1,rc.right-1,rc.top);
	}
	BITMAPINFOHEADER bit;
	GetObject(b->hBit,sizeof(bit),&bit);
	char ddx=(FButSize-bit.biWidth/2)/2;
	char ddy=(FButSize-bit.biHeight)/2;
	char dd=b->State==1 ? 1:0;
	if (b->Enable) DrawBitmap(hdc,b->hBit,rc.left+ddx+dd,rc.top+ddy+dd);
	else DrawState(hdc, NULL, NULL, (LPARAM)b->hBit, NULL, rc.left+dd+ddx, rc.top+dd+ddy, bit.biWidth/2, bit.biHeight, DST_BITMAP |DSS_DISABLED);
	if (c) ReleaseDC(hWnd,hdc);
}

void CToolBar::DrawSepLine(HDC hdc,HPEN hPen,PToolButton b,char d)
{
	char dx=FHorz ? 1:0;
	WORD x,y;
	x=b->x+dx*(FButSize/4+d-1);
	y=b->y+(1-dx)*(FButSize/4+d-1);
	SelectObject(hdc,hPen);
	MoveToEx(hdc,x,y,NULL);
	LineTo(hdc,x+(1-dx)*FButSize,y+dx*FButSize);
};

void CToolBar::DrawToolBar(PAINTSTRUCT *ps)
{
	HPEN hOldPen=(HPEN)SelectObject(ps->hdc,hWhitePen);
	MoveToEx(ps->hdc,0,ps->rcPaint.bottom-1,NULL);
	LineTo(ps->hdc,0,0);
	LineTo(ps->hdc,ps->rcPaint.right,0);
	SelectObject(ps->hdc,hGrayPen);
	MoveToEx(ps->hdc,0,ps->rcPaint.bottom-1,NULL);
	LineTo(ps->hdc,ps->rcPaint.right,ps->rcPaint.bottom-1);
	nBut=FirstButton;
	while (nBut)
	{
		if (nBut->Visible)
		if (nBut->ButType==TBSTYLE_BUTTON) DrawButton(nBut,ps->hdc);
		else 
		{
			DrawSepLine(ps->hdc,hGrayPen,nBut,0);
			DrawSepLine(ps->hdc,hWhitePen,nBut,1);
		}
		nBut=nBut->Next;
	}
	SelectObject(ps->hdc,hOldPen);
};

void CToolBar::MouseMove(PToolButton b, BOOL Always)
{
	if (!b && !ActiveButton) return;
	if (b && ActiveButton && b==ActiveButton && !Always) return;
	if (ActiveButton)
	{
		ActiveButton->State=2;
		DrawButton(ActiveButton,NULL);
	}
	ActiveButton=b;
	if (b)
	{
		if (Pressed)
		{
			if (ActiveButton==ClickButton) ActiveButton->State=1;
		}
		else ActiveButton->State=0;
		DrawButton(ActiveButton,NULL);
	}
	if (ActiveButton && !TimerID) TimerID=SetTimer(hWnd,1,200,NULL);
}

void CToolBar::TimerOn()
{
	PToolButton b;
	RECT rc;
	GetWindowRect(hWnd,&rc);
	POINT p;
	GetCursorPos(&p);
	if (p.x<rc.left || p.x>rc.right || p.y<rc.top || p.y>rc.bottom)
	{
		b=NULL;
		KillTimer(hWnd,TimerID);
		TimerID=0;
	}
	else b=DetectButton(p.x-rc.left,p.y-rc.top);
	MouseMove(b);
}

LRESULT CALLBACK CToolBar::ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
CToolBar         *bar = (CToolBar *)GetWindowLong(hwnd, GWL_USERDATA);
	switch (msg)
	{
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hwnd,&ps);
		bar->DrawToolBar(&ps);
		EndPaint(hwnd,&ps);
		break;
	case WM_MOUSEMOVE:
		bar->MouseMove(bar->DetectButton(LOWORD(lParam),HIWORD(lParam)));
		break;
	case WM_TIMER:
		if (wParam==bar->TimerID) bar->TimerOn();
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		bar->Pressed=TRUE;
		bar->ClickButton=bar->DetectButton(LOWORD(lParam),HIWORD(lParam));
		bar->MouseMove(bar->ClickButton,TRUE);
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		mBut=bar->DetectButton(LOWORD(lParam),HIWORD(lParam));
		bar->Pressed=FALSE;
		bar->MouseMove(mBut,TRUE);
		if (mBut==bar->ClickButton && mBut)
		{
			MySound();
			SendMessage(GetParent(hwnd),WM_COMMAND,mBut->ID+(BN_CLICKED<<16),(LPARAM)hwnd);
		}
		break;
    default: 
		return(DefWindowProc(hwnd, msg, wParam, lParam));
	}
	return 0;
}
