#include "windows.h"
#include "../../Headers/Bitmap.h"
#include "ToolBar2.h"
#include <commctrl.h>
#include "../../Headers/const.h"

extern HINSTANCE hInstance;
extern HPEN	hpnBlack,hpnColor1,hWhitePen,hGrayPen;
PToolButton pBut,pBut2;
static int ToolBar2Counts=0;
WORD ButID=1000;
#define SPLITSIZE 10

void CToolBar2::SetButSize(byte NewSize)
{
	if (NewSize==FButSize) return;
	FButSize=NewSize;
	UpdateBar();
}

void CToolBar2::BeginUpdate()
{
	CanUpdate=FALSE;
}

void CToolBar2::EndUpdate()
{
	CanUpdate=TRUE;
	UpdateBar();
}

void CToolBar2::SetNewPosition(WORD x, WORD y)
{
	SetWindowPos(hWnd,0,x,y,0,0,SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
}

void CToolBar2::SetEdge(byte NewValue)
{
	if (FEdge==NewValue) return;
	FEdge=NewValue;
	UpdateBar();
}

BOOL CToolBar2::AddButton(LPCTSTR Tips, CMyBitmap* hBit, LPVOID Data, BOOL bSplitter)
{
	FindButton(999,&pBut,&pBut2);
	pBut=(PToolButton)malloc(sizeof(CToolButton));
	pBut->hBit=hBit;
	pBut->ID=ButID++;
	pBut->lpvData=Data;
	pBut->Splitter=bSplitter;
	pBut->Next=NULL;
	if (pBut2)	pBut2->Next=pBut;
	else FirstButton=pBut;
	pBut->Pressed=FALSE;
	pBut->Tips=(char*)malloc(lstrlen(Tips)+1);
	lstrcpy(pBut->Tips,Tips);
	UpdateBar();
	return TRUE;
}

LPVOID CToolBar2::GetPrevUnit(WORD x, WORD y)
{
	PToolButton res=NULL;
	PToolButton but=FirstButton;
	while (but)
	{
		WORD lx;
		WORD ly;
		GetButtonSize(but,&lx, &ly);
		if (IsHorizontal())
		{
			if (x<but->x+lx/2) break;
		}
		else
		{
			if (y<but->y+ly/2) break;
		}
		res=but;
		but=but->Next;
	}
	if (res) return res->lpvData;
	return NULL;
}

void CToolBar2::SetAboveButton(PToolButton b)
{
	if (AboveButton==b) return;
	SendMessage(GetParent(hWnd),WM_TOOLBARBUTTONABOVE,0,0);
	if (b) SendMessage(GetParent(hWnd),WM_TOOLBARBUTTONABOVE,(WPARAM)b->lpvData,FToolBarID);
	AboveButton=b;
}

/*void CToolBar2::GetButtonCenter(LPVOID Data, LPPOINT pt)
{
	pBut=FirstButton;
	pt->x=pt->y=0;
	if (!pBut) return;
	byte i=0;
	while (pBut && pBut->lpvData!=Data)
	{
		pBut=pBut->Next;
		i++;
	}
	RECT rc;
	GetWindowRect(hWnd,&rc);
	byte Horz=1;
	if ((FEdge==ABE_LEFT)||(FEdge==ABE_RIGHT)) Horz=0;

	pt->x=rc.left+Horz*i*(FButSize+3)+FButSize/2-3;
	pt->y=rc.top+(1-Horz)*i*(FButSize+3)+FButSize/2-3;
}*/

void CToolBar2::FindButton(WORD ID,PToolButton *ppBut, PToolButton *ppPrev)
{
	*ppPrev=NULL;
	*ppBut=FirstButton;
	if (!*ppBut) return;
	while (*ppBut && (*ppBut)->ID!=ID)
	{
		*ppPrev=*ppBut;
		*ppBut=(*ppBut)->Next;
	}
}

CToolBar2::CToolBar2(HWND Owner, WORD x, WORD y,char ButSize, byte iEdge, HWND hToolTip, byte ToolBarID)
{
FButSize=ButSize;
FEdge=iEdge;
KolTips=0;
FirstButton=NULL;
Pressed=FALSE;
CanUpdate=TRUE;
TimerID=0;
AboveButton=NULL;
ClickButton=NULL;
bCanDrag=FALSE;
bDraged=FALSE;
bMoved=FALSE;
hToolTipWnd=hToolTip;
FToolBarID=ToolBarID;
hDragCursor=LoadCursor(GetModuleHandle("OrgPanel.dll"),"DRAG");
if (!ToolBar2Counts++)
{
	WNDCLASS   wc;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= "MyToolBar2";
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
hWnd=CreateWindow("MyToolBar2","",WS_CHILD,x,y,0,0,Owner,NULL,hInstance,NULL);
SetWindowLong(hWnd, GWL_USERDATA, (long)this);
UpdateBar();
ShowWindow(hWnd,SW_SHOW);
};

void CToolBar2::FreeAll()
{
	DeleteToolTips();
	while (FirstButton)
	{
		pBut=FirstButton->Next;
		free(FirstButton->Tips);
		free(FirstButton);
		FirstButton=pBut;
	}
	DeleteObject(hDragCursor);
}

BOOL CToolBar2::IsHorizontal()
{
	return !((FEdge==ABE_LEFT)||(FEdge==ABE_RIGHT));
}

CToolBar2::~CToolBar2()
{
	FreeAll();	
	DestroyWindow(hWnd);
	if (!--ToolBar2Counts)
	{
		UnregisterClass("MyToolBar2", hInstance);
	}
};

void CToolBar2::DeleteToolTips()
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

void CToolBar2::GetButtonSize(PToolButton pBut, WORD *px, WORD *py)
{
	char dx,dy;
	if (!IsHorizontal())	{		dx=0;			dy=1;}
	else					{		dx=1;			dy=0;}
	
		if (pBut->Splitter)
		{
			*px=(IsHorizontal()? SPLITSIZE+3 : FButSize+3)*dx;
			*py=(IsHorizontal()? FButSize+3 : SPLITSIZE+3)*dy;
		}
		else
		{
			*px=(FButSize+3)*dx;
			*py=(FButSize+3)*dy;
		}
}

void CToolBar2::UpdateBar()
{
	if (!CanUpdate) return;
	WORD x,y;

	DeleteToolTips();

	x=0;		y=0;

	pBut=FirstButton;
	while (pBut)
	{
		pBut->x=x;
		pBut->y=y;
		WORD lx;
		WORD ly;
		GetButtonSize(pBut,&lx,&ly);
		x+=lx;
		y+=ly;

		if (!pBut->Splitter)
		{
			RECT rc;
			GetButtonRect(pBut,&rc);
			TOOLINFO ti;
			ZeroMemory(&ti, sizeof(ti));
			ti.cbSize=sizeof(ti);
			ti.uFlags = TTF_SUBCLASS;  //TTF_SUBCLASS causes the tooltip to automatically subclass the window and look for the messages it is interested in.
			ti.hwnd = hWnd;
			ti.uId = KolTips++;
			ti.lpszText=pBut->Tips;
			ti.rect=rc;
			SendMessage(hToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
		}
		pBut=pBut->Next;
	}
	InvalidateRect(hWnd,NULL,TRUE);
	BOOL Horz=IsHorizontal();
	SetWindowPos(hWnd,0,0,0,Horz ? x-2 : FButSize+1, Horz ? FButSize+1 : y-2,SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
};

void DrawLine2(HDC dc,LRESULT State,HPEN NoPen,HPEN YesPen,long x0,long y0,long x1,long y1,long x2,long y2)
{
if (State) SelectObject(dc,YesPen);
else SelectObject(dc,NoPen);
MoveToEx(dc,x0,y0,NULL);
LineTo(dc,x1,y1);
LineTo(dc,x2,y2);
}

PToolButton CToolBar2::DetectButton(int x,int y)
{
	pBut=FirstButton;
	while (pBut)
	{
		RECT rc;
		GetButtonRect(pBut,&rc);
		if (x>=rc.left && x<=rc.right && y>=rc.top && y<=rc.bottom) return pBut;
		pBut=pBut->Next;
	}
	return NULL;
}

void CToolBar2::GetButtonRect(PToolButton b, LPRECT rc)
{
	rc->left=b->x;
	rc->top=b->y;
	if (b->Splitter)
	{
		rc->bottom=rc->top+(IsHorizontal()? FButSize : SPLITSIZE);
		rc->right=rc->left+(IsHorizontal()? SPLITSIZE : FButSize);
	}
	else
	{
		rc->bottom=rc->top+FButSize;
		rc->right=rc->left+FButSize;
	}
}

void CToolBar2::DrawButton(PToolButton b, HDC hdc)
{
	BOOL c=!hdc;
	if (c) hdc=GetDC(hWnd);
	RECT rc;
	GetButtonRect(b,&rc);

	if (b->Splitter)
	{
		if (IsHorizontal())
		{
			SelectObject(hdc,hGrayPen);
			MoveToEx(hdc,rc.left+SPLITSIZE/2,rc.top+3,NULL);
			LineTo(hdc,rc.left+SPLITSIZE/2,rc.bottom-3);
			SelectObject(hdc,hpnColor1);
			MoveToEx(hdc,rc.left+SPLITSIZE/2+1,rc.top+3,NULL);
			LineTo(hdc,rc.left+SPLITSIZE/2+1,rc.bottom-3);
		}
		else
		{
			SelectObject(hdc,hGrayPen);
			MoveToEx(hdc,rc.left+3,rc.top+SPLITSIZE/2,NULL);
			LineTo(hdc,rc.right-3,rc.bottom-SPLITSIZE/2);
			SelectObject(hdc,hpnColor1);
			MoveToEx(hdc,rc.left+3,rc.top+SPLITSIZE/2+1,NULL);
			LineTo(hdc,rc.right-3,rc.bottom-SPLITSIZE/2+1);
		}
	}
	else
	{
		FillRect(hdc,&rc,(HBRUSH)COLOR_WINDOW);
		DrawLine2(hdc,b->Pressed,hWhitePen,hpnBlack,rc.left,rc.bottom,rc.left,rc.top,rc.right,rc.top);
		DrawLine2(hdc,b->Pressed,hpnColor1,hGrayPen,rc.left+1,rc.bottom-1,rc.left+1,rc.top+1,rc.right-1,rc.top+1);
		DrawLine2(hdc,b->Pressed,hpnBlack,hWhitePen,rc.left,rc.bottom,rc.right,rc.bottom,rc.right,rc.top);
		DrawLine2(hdc,b->Pressed,hGrayPen,hpnColor1,rc.left+1,rc.bottom-1,rc.right-1,rc.bottom-1,rc.right-1,rc.top+1);

		BITMAPINFOHEADER bit;
		GetObject(b->hBit->GetBitmap(),sizeof(bit),&bit);
		char ddx=(FButSize-bit.biWidth)/2;
		char ddy=(FButSize-bit.biHeight)/2;
		char dd=b->Pressed ? 1:0;
		BitBlt(hdc,rc.left+ddx+dd,rc.top+ddy+dd,bit.biWidth,bit.biHeight,b->hBit->hdc,0,0,SRCCOPY);
	}
	if (c) ReleaseDC(hWnd,hdc);
}

void CToolBar2::DrawToolBar(PAINTSTRUCT *ps)
{
	HPEN hOldPen=(HPEN)SelectObject(ps->hdc,hWhitePen);
	pBut=FirstButton;
	while (pBut)
	{
		DrawButton(pBut,ps->hdc);
		pBut=pBut->Next;
	}
	SelectObject(ps->hdc,hOldPen);
};

void CToolBar2::MouseMove(PToolButton b, BOOL Always)
{
	if (!b && !AboveButton) return;
	if (b && AboveButton && b==AboveButton && !Always) return;
	SetAboveButton(b);
	if (AboveButton && !TimerID) TimerID=SetTimer(hWnd,1,200,NULL);
	if (!ClickButton || !Pressed) return;
	BOOL pr=ClickButton->Pressed;
	ClickButton->Pressed=(b==ClickButton);
	if (pr!=ClickButton->Pressed) DrawButton(ClickButton,NULL);
}

void CToolBar2::TimerOn()
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

void CToolBar2::OnLButDown(WORD x, WORD y)
{
	StartX=x;
	StartY=y;
	ClickButton=DetectButton(x, y);

	if (!ClickButton||!bCanDrag) return;
		bMoved=TRUE;
}

void CToolBar2::OnMouseMove(WORD x, WORD y)
{
		if (bDraged)
		{
//			OnDraged(x,y,FALSE);
			return;
		}
		if (bMoved && !bDraged)
		{
			int xx=StartX-x;
			int yy=StartY-y;
			if ((xx*xx+yy*yy)>100)
			{
				bDraged=TRUE;
//				bOldOkDragCursor=TRUE;
				hOldCursor=SetCursor(hDragCursor);
			}
			return;
		}
}

void CToolBar2::OnLButUp(WORD x, WORD y)
{
		pBut2=DetectButton(x,y);
		if (bDraged)
		{
		//	OnDraged(lParam,TRUE);
			bDraged=FALSE;
			SetCursor(hOldCursor);
		}
		if (bMoved)
		{
			bMoved=FALSE;
		}
		ReleaseCapture();
		MouseMove(NULL,TRUE);
		Pressed=FALSE;
		if (pBut2==ClickButton && pBut2)
		{
			SendMessage(GetParent(hWnd),WM_TOOLBARBUTTONCLICK,(WPARAM)pBut2->lpvData,FToolBarID);
			DrawButton(pBut2,NULL);
		}
		else
		{
			POINT pt;
			RECT rc;
			GetWindowRect(hWnd,&rc);
			pt.x=x;
			pt.y=y;
			if (pt.x>30000) pt.x-=65536;
			if (pt.y>30000) pt.y-=65536;

			pt.x+=rc.left;
			pt.y+=rc.top;
			if (ClickButton)
			SendMessage(GetParent(hWnd),WM_DROPTOOLBUT,(WPARAM)ClickButton->lpvData,(LPARAM)&pt );
		}
}

LRESULT CALLBACK CToolBar2::ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
CToolBar2         *bar = (CToolBar2 *)GetWindowLong(hwnd, GWL_USERDATA);
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
		bar->OnMouseMove(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_TIMER:
		if (wParam==bar->TimerID) bar->TimerOn();
		break;
	case WM_RBUTTONUP:
		pBut2=bar->DetectButton(LOWORD(lParam),HIWORD(lParam));
		if (pBut2) SendMessage(GetParent(bar->hWnd),WM_TOOLBARBUTTONRCLICK,(WPARAM)pBut2->lpvData,bar->FToolBarID);
		break;
	case WM_LBUTTONDOWN:
		bar->OnLButDown(LOWORD(lParam),HIWORD(lParam));
		SetCapture(hwnd);
		bar->Pressed=TRUE;
		bar->MouseMove(bar->ClickButton,TRUE);
		break;
	case WM_LBUTTONUP:
		bar->OnLButUp(LOWORD(lParam),HIWORD(lParam));
		break;
    default: 
		return(DefWindowProc(hwnd, msg, wParam, lParam));
	}
	return 0;
}
