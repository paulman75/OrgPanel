#include "windows.h"
#include "ToolBar.h"
#include <commctrl.h>
#include "../Headers/const.h"
#include "../Headers/general.h"

extern HINSTANCE hInstance;
extern HPEN	hBlackPen,hpnColor1,hWhitePen,hGrayPen;
extern HFONT hMainFont;
PToolButton nBut,mBut;
static int ToolBarCounts=0;

BOOL CToolBar::AddButton(LPCTSTR Tips, WORD ID, LPCTSTR strName, char ButType, BOOL Visible,BOOL Enable, LPCTSTR sMenu)
{
	FindButton(ID,&nBut,&mBut);
	if (nBut) return FALSE;
	if (strName) if (strName[0]==0) strName=NULL;
	nBut=(PToolButton)malloc(sizeof(CToolButton));
	if (strName) nBut->hBit=new CMemoryBitmap(hInstance,strName);
	else nBut->hBit=NULL;
	nBut->hBit2=NULL;
	Update2Bitmap(nBut);
	nBut->ID=ID;
	nBut->Visible=Visible;
	nBut->MenuVis=FALSE;
	nBut->Enable=Enable;
	nBut->Next=NULL;
	if (mBut)	mBut->Next=nBut;
	else FirstButton=nBut;
	nBut->State=2;
	nBut->ButType=ButType;
	nBut->Tips=(char*)malloc(lstrlen(Tips)+1);
	lstrcpy(nBut->Tips,Tips);
	if (sMenu)
	{
		nBut->strMenu=(char*)malloc(lstrlen(sMenu)+1);
		lstrcpy(nBut->strMenu, sMenu);
	}
	else nBut->strMenu=NULL;
	UpdateBar();
	return TRUE;
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
	if (nBut->strMenu) free(nBut->strMenu);
	if (nBut->hBit) delete nBut->hBit;
	if (nBut->hBit2) delete nBut->hBit2;
	free(nBut);
	UpdateBar();
	return TRUE;
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

BOOL CToolBar::UpdateParam(WORD ID, LPCTSTR Tips)
{
	FindButton(ID,&nBut,&mBut);
	if (!nBut) return FALSE;
	if (Tips)
	{
		free(nBut->Tips);
		nBut->Tips=(char*)malloc(lstrlen(Tips)+1);
		lstrcpy(nBut->Tips,Tips);
	}
	UpdateBar();
	return TRUE;
}

BOOL CToolBar::SetMenu(WORD ID, LPCTSTR strName)
{
	FindButton(ID,&nBut,&mBut);
	if (!nBut) return FALSE;
	if (nBut->strMenu)
	{
		free(nBut->strMenu);
		if (strName)
		{
			nBut->strMenu=(char*)malloc(lstrlen(strName)+1);
			lstrcpy(nBut->strMenu,strName);
		}
		else nBut->strMenu=NULL;
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

CToolBar::CToolBar(HWND Owner, WORD x, WORD y,char ButSizeX, char ButSizeY, char Delta, LPCTSTR strBack, BOOL ShowCaption)
{
DefButw=ButSizeX;
FButh=ButSizeY;
FirstPos=0;
KolTips=0;
FirstButton=NULL;
TimerID=0;
Pressed=FALSE;
MenuShowed=FALSE;
FShowCaption=ShowCaption;
hToolTipWnd=CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_CHILD | WS_POPUP | TTS_ALWAYSTIP, 
            CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, 
             Owner, NULL, hInstance, NULL);

FDelta=Delta+2;
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
	hBackBitmap=NULL;
	hBackAll=NULL;
	hMenuBitmap=new CMemoryBitmap(hInstance,NULL);
	CreateMenuButtonImage(hMenuBitmap);
	if (strBack) hBackBitmap=LoadBitmap(hInstance,strBack);
	else hBackBitmap=NULL;

	hWnd=CreateWindow("MyToolBar","",WS_CHILD,x,y,40,FButh+FDelta*2,Owner,NULL,hInstance,NULL);
	SetWindowLong(hWnd, GWL_USERDATA, (long)this);
	UpdateBar();
	ShowWindow(hWnd,SW_SHOW);
};

void CToolBar::OnSize(WORD w, WORD h)
{
	h=FButh+FDelta*2;
	if (hBackAll) delete hBackAll;
	if (hBackBitmap)
	{
		hBackAll=new CMemoryBitmap();
		HDC dc=GetDC(0);
		hBackAll->NewEmptyBitmap(dc,w,h);
		ReleaseDC(0,dc);
		RECT rc;
		rc.left=rc.top=0;
		rc.bottom=h;
		rc.right=w;
		FillBackGround(hBackAll->hdc,&rc,hBackBitmap);
	}
	SetWindowPos(hWnd,0,0,0,w,h,SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	InvalidateRect(hWnd,NULL,TRUE);
}

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
	if (hBackBitmap) delete hBackBitmap;
	if (hBackAll) delete hBackAll;
	if (hMenuBitmap) delete hMenuBitmap;
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
	WORD x,y;
	DeleteToolTips();
	x=FirstPos+FDelta;		y=FDelta;
	nBut=FirstButton;
	while (nBut)
	{
		nBut->x=x;
		nBut->y=y;
		nBut->Width=nBut->strMenu ? DefButw+15 : DefButw;
		if (nBut->Visible) x+=(nBut->ButType==TBSTYLE_SEP) ? nBut->Width/4 : nBut->Width+8;
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

void CToolBar::LButtonDown(int x,int y)
{
	ClickButton=DetectButton(x,y);
	if (ClickButton)
	{
		RECT rc;
		GetButtonRect(ClickButton,&rc);
		MenuShowed=x>rc.left+DefButw && ClickButton->strMenu;
		if (MenuShowed)
		{
			RECT rc2;
			GetWindowRect(hWnd,&rc2);
			HMENU m1=LoadMenu(hInstance, ClickButton->strMenu);
			SendMessage(GetParent(hWnd),WM_MENULOADED,(WPARAM)m1,ClickButton->ID);
			HMENU m2=GetSubMenu(m1,0);
			ClickButton->MenuVis=TRUE;
			DrawButton(ClickButton,NULL);
			TrackPopupMenu(m2,0,rc2.left+rc.left,rc2.top+rc.bottom,0,GetParent(hWnd),NULL);
			DestroyMenu(m1);
			ClickButton->MenuVis=FALSE;
			MenuShowed=FALSE;
			POINT pt;
			GetCursorPos(&pt);
			ClickButton=DetectButton(pt.x-rc2.left,pt.y-rc2.top);
			MouseMove(ClickButton,TRUE);
			return;
		}
	}
	SetCapture(hWnd);
	Pressed=TRUE;
	MouseMove(ClickButton,TRUE);
}

void CToolBar::GetButtonRect(PToolButton b, LPRECT rc)
{
	rc->left=b->x;
	rc->top=b->y;
	rc->bottom=rc->top+FButh;
	rc->right=rc->left+b->Width;
}

void CToolBar::Update2Bitmap(PToolButton b)
{
	if (b->hBit2) delete b->hBit2;
	b->hBit2=NULL;
	if (!b->hBit) return;
	b->hBit2=new CMemoryBitmap();
	BITMAPINFOHEADER bit;
	GetObject(b->hBit->handle,sizeof(bit),&bit);
	b->hBit2->NewBitmap(b->hBit,0,0,bit.biWidth/2,bit.biHeight);
	for (WORD x=0; x<bit.biWidth; x++)
	for (WORD y=0; y<bit.biHeight; y++)
	{
		COLORREF c=GetPixel(b->hBit2->hdc,x,y);
		if (c!=0xffffff && c!=0)
		{
			byte r=GetRValue(c);
			byte g=GetGValue(c);
			byte cb=GetBValue(c);
			byte k=(r/2+g+cb)/3;
			if (k>160) c=0xffffff;
			else if (k>90) c=0xc0c0c0;
			else if (k>40) c=0x808080;
			else c=0;
			SetPixel(b->hBit2->hdc,x,y,c);
		}
	}
}

void CToolBar::PaintBitmap(CMemoryBitmap* b, CMemoryBitmap* Mask, HDC hdc, WORD xc, WORD yc, BOOL Enable)
{
	BITMAPINFOHEADER bit;
	GetObject(Mask->handle,sizeof(bit),&bit);
	WORD w=bit.biWidth/2;
	WORD h=(WORD)bit.biHeight;
	yc=yc-h/2;
	xc=xc-w/2;
	if (Enable)
	{
		CMemoryBitmap* hBit1=new CMemoryBitmap();
		hBit1->NewBitmap(b,0,0,w,h);

		if (hBackAll) BitBlt(hBit1->hdc,0,0,w,h,hBackAll->hdc,xc,yc,SRCCOPY);
		else
		{
			RECT rc;
			rc.left=rc.top=0;
			rc.bottom=h;
			rc.right=w;
			FillRect(hBit1->hdc,&rc,(HBRUSH)COLOR_WINDOW);
		}
		BitBlt(hBit1->hdc,0,0,w,h,Mask->hdc,w,0,SRCPAINT);
		BitBlt(hBit1->hdc,0,0,w,h,b->hdc,0,0,SRCAND);
		BitBlt(hdc,xc,yc,w,h,hBit1->hdc,0,0,SRCCOPY);
		delete hBit1;
	}
	else
	{
		HBITMAP hh=CreateCompatibleBitmap(b->hdc,10,10);
		SelectObject(b->hdc,hh);
		DrawState(hdc, NULL, NULL, (LPARAM)b->handle, NULL, xc, yc, w, h, DST_BITMAP |DSS_DISABLED);
		SelectObject(b->hdc,b->handle);
		DeleteObject(hh);
	}
}

void CToolBar::DrawButton(PToolButton b, HDC hdc)
{
	BOOL c=!hdc;
	if (c) hdc=GetDC(hWnd);
	RECT rc;
	GetButtonRect(b,&rc);
	if (hBackAll) BitBlt(hdc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,hBackAll->hdc,rc.left,rc.top,SRCCOPY);
	else FillRect(hdc,&rc,(HBRUSH)COLOR_WINDOW);
	if (b->State!=2)
	{
		DrawLine(hdc,b->State,hWhitePen,hGrayPen,rc.left,rc.bottom-1,rc.left,rc.top,rc.left+DefButw-1,rc.top);
		DrawLine(hdc,b->State,hGrayPen,hWhitePen,rc.left,rc.bottom-1,rc.left+DefButw-1,rc.bottom-1,rc.left+DefButw-1,rc.top);
	}
	char dd=b->State==1 ? 1:0;
	WORD x=rc.left+DefButw/2+dd;
	WORD y=(rc.top+rc.bottom)/2+dd;
	if (FShowCaption)
	{
		SetBkMode(hdc,TRANSPARENT);
		SelectObject(hdc,hMainFont);
		char f[20];
		lstrcpy(f,b->Tips);
		byte l=lstrlen(f);
		SIZE siz;
		int w=rc.right-rc.left-2;
		if (b->strMenu) w-=15;
		while (TRUE)
		{
			GetTextExtentPoint32(hdc,f,l,&siz);
			if (siz.cx<w) break;
			f[--l]=0;
		}
		if (l!=lstrlen(b->Tips))
		{
			lstrcpy(f,b->Tips);
			l=lstrlen(f);
			f[l-1]=46;
			f[l-2]=46;
			f[l-3]=46;
			while (TRUE)
			{
				GetTextExtentPoint32(hdc,f,l,&siz);
				if (siz.cx<rc.right-rc.left-2) break;
				f[--l-3]=46;
			}
		}
		GetTextExtentPoint32(hdc,f,l,&siz);
		WORD xt=rc.left+(DefButw-siz.cx)/2+dd;
		WORD yt=rc.bottom-siz.cy+dd-2;
		if (b->Enable)	TextOut(hdc,xt,yt,f,l);
		else DrawState(hdc, NULL, NULL, (LPARAM)f, NULL, xt, yt, 0, 0, DST_TEXT |DSS_DISABLED);
		y-=siz.cy/2-2;
	}
	CMemoryBitmap* Im=(b->State==2 || !b->Enable) ? b->hBit2 : b->hBit;
	PaintBitmap(Im,b->hBit, hdc,x,y,b->Enable);
	if (b->strMenu)
	{
		PaintBitmap(hMenuBitmap,hMenuBitmap,hdc,(rc.right+rc.left+DefButw)/2+dd,(rc.bottom+rc.top)/2+dd,b->Enable);
		if (b->State!=2 || b->MenuVis) 
		{
			DrawLine(hdc,b->State || b->MenuVis,hWhitePen,hGrayPen,rc.left+DefButw,rc.bottom-1,rc.left+DefButw,rc.top,rc.right-1,rc.top);
			DrawLine(hdc,b->State || b->MenuVis,hGrayPen,hWhitePen,rc.left+DefButw+1,rc.bottom-1,rc.right-1,rc.bottom-1,rc.right-1,rc.top);
		}
	}
	if (c) ReleaseDC(hWnd,hdc);
}

void CToolBar::DrawSepLine(HDC hdc,HPEN hPen,PToolButton b,char d)
{
	WORD x=b->x+(DefButw/8+d-4);
	SelectObject(hdc,hPen);
	MoveToEx(hdc,x,b->y+3,NULL);
	LineTo(hdc,x,b->y+FButh-3);
};

void CToolBar::DrawToolBar(PAINTSTRUCT *ps)
{
	HPEN hOldPen=(HPEN)SelectObject(ps->hdc,hWhitePen);
	RECT rc;
	GetClientRect(hWnd,&rc);
	MoveToEx(ps->hdc,1,1,NULL);
	LineTo(ps->hdc,rc.right-1,1);
	LineTo(ps->hdc,rc.right-1,rc.bottom-1);
	LineTo(ps->hdc,1,rc.bottom-1);
	LineTo(ps->hdc,1,1);
	SelectObject(ps->hdc,hGrayPen);
	MoveToEx(ps->hdc,0,0,NULL);
	LineTo(ps->hdc,rc.right-2,0);
	LineTo(ps->hdc,rc.right-2,rc.bottom-2);
	LineTo(ps->hdc,0,rc.bottom-2);
	LineTo(ps->hdc,0,0);

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
	if ((!b && !ActiveButton) || MenuShowed) return;
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
RECT rc;
	switch (msg)
	{
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hwnd,&ps);
		bar->DrawToolBar(&ps);
		EndPaint(hwnd,&ps);
		break;
	case WM_ERASEBKGND:
		if (bar->hBackAll)
		{
			GetClientRect(hwnd,&rc);
			BitBlt((HDC)wParam,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,bar->hBackAll->hdc,0,0,SRCCOPY);
			return TRUE;
		}
		break;
	case WM_MOUSEMOVE:
		bar->MouseMove(bar->DetectButton(LOWORD(lParam),HIWORD(lParam)));
		break;
	case WM_TIMER:
		if (wParam==bar->TimerID) bar->TimerOn();
		break;
	case WM_LBUTTONDOWN:
		bar->LButtonDown(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		if (!bar->MenuShowed)
		{
			ReleaseCapture();
			mBut=bar->DetectButton(LOWORD(lParam),HIWORD(lParam));
			bar->Pressed=FALSE;
			bar->MouseMove(mBut,TRUE);
			if (mBut==bar->ClickButton && mBut) SendMessage(GetParent(hwnd),WM_COMMAND,mBut->ID+(BN_CLICKED<<16),(LPARAM)hwnd);
		}
		break;
	}
	return(DefWindowProc(hwnd, msg, wParam, lParam));
}

void CToolBar::CreateMenuButtonImage(CMemoryBitmap* b)
{
	HDC hdc=GetDC(0);
	b->handle=CreateCompatibleBitmap(hdc,14,4);
	ReleaseDC(0,hdc);
	SelectObject(b->hdc,b->handle);
	RECT rc;
	rc.left=rc.top=0;
	rc.right=14;
	rc.bottom=4;
	FillRect(b->hdc,&rc,(HBRUSH)GetStockObject(WHITE_BRUSH));
	rc.left=7;
	FillRect(b->hdc,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
	SelectObject(b->hdc,GetStockObject(BLACK_BRUSH));
	SelectObject(b->hdc,GetStockObject(BLACK_PEN));
	POINT Poi[3];
	Poi[0].x=Poi[0].y=Poi[1].y=0;
	Poi[1].x=6;
	Poi[2].x=Poi[2].y=3;
	Polygon(b->hdc,Poi,3);
	SelectObject(b->hdc,GetStockObject(WHITE_BRUSH));
	SelectObject(b->hdc,GetStockObject(WHITE_PEN));
	Poi[0].x=7;
	Poi[1].x=13;
	Poi[2].x=10;
	Polygon(b->hdc,Poi,3);
}
