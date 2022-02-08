#include "windows.h"
#include "CTimer.h"
#include "AppBar.h"
#include <windowsx.h>
#include "..\Headers\general.h"
#include "Compon\MyUtils.h"
#include "..\Headers\const.h"
#include "RegUnit.h"

CTimer*	Timer1;
CTimer*	Timer2;
CTimer* Timer3;
extern TBarConfig	BarCon;

void	CAppBar::UpdateOnDesktop()
{
	if (MoveEdge!=ABE_DESKTOP) return;
	RECT	rc;
	GetWindowRect(Main_hwnd,&rc);
    MoveWindow(Main_hwnd,rc.left,rc.top,FWidthOnDesktop,FHeightOnDesktop,TRUE);
	InvalidateRect(Main_hwnd,NULL,TRUE);
}

void	CAppBar::SetOpenDelay(int od)
{
	OpenDelay=od;
	Timer3->SetInterval(od);
}

void	CAppBar::SetWidthOnDesktop(WORD New)
{
	FWidthOnDesktop=New;
	UpdateOnDesktop();
}

BOOL CAppBar::GetTitleVisible()
{
	return (BarWidth!=FullBarWidth || BarHeight!=FullBarHeight);
}

void	CAppBar::SetHeightOnDesktop(WORD New)
{
	FHeightOnDesktop=New;
	UpdateOnDesktop();
}

void CAppBar::UpdateBar()
{
if (!Registered) return;
UnRegisterBar();
RegisterBar();
}

BOOL CAppBar::GetAutoHide()
{
	return FAutoHide;
}

void CAppBar::SetAutoHide(BOOL NewValue)
{
	if (NewValue==FAutoHide) return;
	FAutoHide=NewValue;
	if (Registered) UpdateBar();
}

void CAppBar::SetSpeed(byte os,byte cs)
{
	FOpenSpeed=os;
	FCloseSpeed=cs;
}

void CAppBar::SetEdge(byte NewValue)
{
	if (NewValue==FEdge) return;
	FEdge=NewValue;
	MoveEdge=NewValue;
	if (Registered) UpdateBar();
}

HWND CAppBar::GetMainWindow()
{
	return Main_hwnd;
}

void CAppBar::WindowToBottom()
{
BOOL cc;
if (!FAutoHide) return;
cc=TRUE;
switch (FEdge)
{
case	ABE_LEFT:      
	cc=(OpenPanelX==0);
	break;
case	ABE_RIGHT:      
	cc=(ClosePanelX==(GetSystemMetrics(SM_CXSCREEN)-FDelta));
	break;
case	ABE_TOP:        
	cc=(OpenPanelY==0);
	break;
case	ABE_BOTTOM:     
	cc=(ClosePanelY==(GetSystemMetrics(SM_CYSCREEN)-FDelta));
	break;
}
if (!cc) SetWindowPos(Dat.hWnd,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void CAppBar::MoveMainWindow(BOOL XChange,int Pos)
{
GetWindowRect(Main_hwnd,&rc);
if (XChange) 
{
	rc.left=Pos;
	rc.right=Pos+FullBarWidth;
} else 
{
	rc.top=Pos;
	rc.bottom=Pos+FullBarHeight;
}
MoveWindow(Main_hwnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);
}

void CAppBar::ChangeBarRgn(BOOL bOpening)
{
if (!bNeedTitle) return;

if ((bOpening && !GetTitleVisible())/*||FAutoHide*/)
{
	GetWindowRect(Main_hwnd,&rc);
	XTitle=0;
	YTitle=0;
	switch (FEdge)
	{
	case	ABE_LEFT: 
		FullBarWidth=BarWidth+TitSize;
		ClosePanelX-=TitSize;
		InvalidateRect(Main_hwnd, NULL, true);

		ptbar[0].x=0;				ptbar[0].y=0;
		ptbar[1].x=BarWidth;		ptbar[1].y=0;
		ptbar[2].x=BarWidth;		ptbar[2].y=100;
		ptbar[3].x=FullBarWidth;	ptbar[3].y=100+TitSize;
		ptbar[4].x=FullBarWidth;	ptbar[4].y=400-TitSize;
		ptbar[5].x=BarWidth;		ptbar[5].y=400;
		ptbar[6].x=BarWidth;		ptbar[6].y=rc.bottom-rc.top;
		ptbar[7].x=0;				ptbar[7].y=rc.bottom-rc.top;
		break;
	case	ABE_RIGHT: 
		FullBarWidth=BarWidth+TitSize;
		OpenPanelX-=TitSize;
		InvalidateRect(Main_hwnd, NULL, true);

		ptbar[0].x=FullBarWidth;	ptbar[0].y=0;
		ptbar[1].x=TitSize;			ptbar[1].y=0;
		ptbar[2].x=TitSize;			ptbar[2].y=100;
		ptbar[3].x=0;				ptbar[3].y=100+TitSize;
		ptbar[4].x=0;				ptbar[4].y=400-TitSize;
		ptbar[5].x=TitSize;			ptbar[5].y=400;
		ptbar[6].x=TitSize;			ptbar[6].y=rc.bottom-rc.top;
		ptbar[7].x=FullBarWidth;	ptbar[7].y=rc.bottom-rc.top;
		XTitle=TitSize;
		break;
	case	ABE_TOP:
		FullBarHeight=BarHeight+TitSize;
		ClosePanelY-=TitSize;
		InvalidateRect(Main_hwnd, NULL, true);

		ptbar[0].x=0;				ptbar[0].y=0;
		ptbar[1].x=0;				ptbar[1].y=BarHeight;
		ptbar[2].x=100;				ptbar[2].y=BarHeight;
		ptbar[3].x=100+TitSize;		ptbar[3].y=FullBarHeight;
		ptbar[4].x=400-TitSize;		ptbar[4].y=FullBarHeight;
		ptbar[5].x=400;				ptbar[5].y=BarHeight;
		ptbar[6].x=rc.right-rc.left;ptbar[6].y=BarHeight;
		ptbar[7].x=rc.right-rc.left;ptbar[7].y=0;
		break;
	case	ABE_BOTTOM:
		FullBarHeight=BarHeight+TitSize;
		OpenPanelY-=TitSize;
		InvalidateRect(Main_hwnd, NULL, true);

		ptbar[0].x=0;				ptbar[0].y=FullBarHeight;
		ptbar[1].x=0;				ptbar[1].y=TitSize;
		ptbar[2].x=100;				ptbar[2].y=TitSize;
		ptbar[3].x=100+TitSize;		ptbar[3].y=0;
		ptbar[4].x=400-TitSize;		ptbar[4].y=0;
		ptbar[5].x=400;				ptbar[5].y=TitSize;
		ptbar[6].x=rc.right-rc.left;ptbar[6].y=TitSize;
		ptbar[7].x=rc.right-rc.left;ptbar[7].y=FullBarHeight;
		YTitle=TitSize;
		break;
	}
	if (rgn) DeleteObject(rgn);
	rgn=CreatePolygonRgn(&ptbar[0],8,ALTERNATE);
	SetWindowRgn(Main_hwnd, rgn, FALSE);
}
if (!bOpening && GetTitleVisible()/*&&FAutoHide*/)
{
	FullBarWidth=BarWidth;
	FullBarHeight=BarHeight;
	GetWindowRect(Main_hwnd,&rc);
	switch (FEdge)
	{
	case	ABE_LEFT: 
		ClosePanelX+=TitSize;
		MoveWindow(Main_hwnd,rc.right-FullBarWidth,rc.top,FullBarWidth,rc.bottom-rc.top,FALSE);
		break;
	case	ABE_RIGHT: 
		OpenPanelX+=TitSize;
		MoveWindow(Main_hwnd,rc.left,rc.top,FullBarWidth,rc.bottom-rc.top,FALSE);
		break;
	case	ABE_TOP: 
		ClosePanelY+=TitSize;
		MoveWindow(Main_hwnd,rc.left, rc.bottom-FullBarHeight, rc.right-rc.left,FullBarHeight, FALSE);
		break;
	case	ABE_BOTTOM: 
		OpenPanelY+=TitSize;
		MoveWindow(Main_hwnd,rc.left, rc.top, rc.right-rc.left,FullBarHeight, FALSE);
		break;
	}
	SetWindowRgn(Main_hwnd, NULL, false);
	InvalidateRect(Main_hwnd, NULL, true);
}
}

BOOL CAppBar::CheckEndPanelTrack(int XDelta,int YDelta)
{
int x;
BOOL c;
int Pos;

if (FEdge==ABE_DESKTOP) return TRUE;
GetWindowRect(Main_hwnd,&rc);
if (XDelta!=0) Pos=rc.left+XDelta;
else Pos=rc.top+YDelta;
if (FEdge==ABE_LEFT) 
{
        if (PanelOpenning) c=(Pos>=OpenPanelX);
        else c=(Pos<=ClosePanelX);
}
if (FEdge==ABE_RIGHT)
{
        if (PanelOpenning) c=(Pos<=OpenPanelX);
        else c=(Pos>=ClosePanelX);
}
if (FEdge==ABE_TOP) 
{
        if (PanelOpenning) c=(Pos>=OpenPanelY);
        else c=(Pos<=ClosePanelY);
}
if (FEdge==ABE_BOTTOM) 
{
        if (PanelOpenning) c=(Pos<=OpenPanelY);
        else c=(Pos>=ClosePanelY);
}
if (c)
{
	ReMoved=TRUE;
	if (PanelOpenning) ChangeBarRgn(PanelOpenning);
}

if ((FEdge==ABE_LEFT) || (FEdge==ABE_RIGHT)) 
{
        if (c) 
		{
                if (PanelOpenning) x=OpenPanelX;
				else x=ClosePanelX;
		}
        else x=Pos;
		MoveMainWindow(TRUE,x);
} 
else 
{
        if (c) 
		{
                if (PanelOpenning) x=OpenPanelY;
                else x=ClosePanelY;
		}
        else x=Pos;
        MoveMainWindow(FALSE,x);
}
if (c) 
{
	if (PanelOpenning) 
	{
		Timer2->SetInterval(250);
		if (GetActiveWindow()!=Main_hwnd) Timer2->Start();
        SetWindowPos(Dat.hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        if (ReMoved) ReMoved=FALSE;
	}	else	//Exists other taskbar?
	{
		BOOL tbExists=FALSE;
		GetWindowRect(Main_hwnd,&rc);
		switch (FEdge)
		{
		case ABE_LEFT:	
			tbExists=(rc.right-FDelta>0);
			break;
		case ABE_RIGHT:	
			tbExists=(rc.left+FDelta<GetSystemMetrics(SM_CXSCREEN));
			break;
		case ABE_TOP:
			tbExists=(rc.bottom-FDelta>0);
			break;
		case ABE_BOTTOM:
			tbExists=(rc.top+FDelta<GetSystemMetrics(SM_CYSCREEN));
			break;
		}
		if (!tbExists) Timer2->Stop();
		else Timer2->Start();
	}
    Timer1->Stop();
	if (!PanelOpenning) ChangeBarRgn(PanelOpenning);
}
else 
{
        WindowToBottom();
        if (ReMoved) ReMoved=FALSE;       
}
return c;
}

void CAppBar::Timer1Event()
{
Timer1->Start();
if ((FEdge==ABE_LEFT) || (FEdge==ABE_RIGHT)) 
 CheckEndPanelTrack(Direct,0);
 else CheckEndPanelTrack(0,Direct);
}

void CAppBar::RegisterBar()
{
if (FEdge!=MoveEdge) MoveEdge=FEdge;
FullBarHeight=BarHeight;
FullBarWidth=BarWidth;

if (FEdge==ABE_DESKTOP) 
{
        Registered=TRUE;
        SetWindowPos(Main_hwnd, HWND_TOPMOST, 0, 0, FWidthOnDesktop, FHeightOnDesktop, SWP_NOMOVE | SWP_NOACTIVATE);
        return; /*BarCon.AlwaysOnTop ? HWND_TOPMOST : 0*/
}
if (Registered) return;

Dat.uEdge=FEdge;
Dat.uCallbackMessage=WM_APPBAR_EVENT;
if ((FAutoHide) && (CheckOtherPanel())) FAutoHide=FALSE;
if (!FAutoHide) SetWindowPos(Main_hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
SHAppBarMessage(ABM_NEW,&Dat);
SetedHide=FALSE;
GetWindowRect(Main_hwnd,&rc);
if ((FEdge==ABE_LEFT) || (FEdge==ABE_RIGHT)) 
{
        if (FEdge==ABE_LEFT) rc.left=-FullBarWidth;
        else rc.left=GetSystemMetrics(SM_CXSCREEN);
} else 
{
        if (FEdge==ABE_TOP) rc.top=-FullBarHeight;
        else rc.top=GetSystemMetrics(SM_CYSCREEN);
}

rc.bottom=rc.top+FullBarHeight;
rc.right=rc.left+FullBarWidth;
SetWindowPos(Main_hwnd,HWND_TOPMOST,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,SWP_NOACTIVATE);
AppBarQuerySetPos();
Registered=TRUE;
if (FAutoHide) 
{
        WindowToBottom();
        Opening();
} else
{
    SetWindowPos(Main_hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ChangeBarRgn(true);
}
}

BOOL CAppBar::CheckOtherPanel()
{
	APPBARDATA Dat1;
	HWND	Panel;

Dat1.cbSize=sizeof(APPBARDATA);
Dat1.uEdge=FEdge;
Panel=(HWND) SHAppBarMessage(ABM_GETAUTOHIDEBAR,&Dat1);
if (Panel==Main_hwnd) Panel=0;
return (Panel!=0);
}

void CAppBar::MesActivate(WORD wParam)
{
if (FEdge==ABE_DESKTOP) return;
if (FAutoHide) 
{
        if (wParam==WA_INACTIVE) 
		CheckMouseOver();//Closing;
        else
		if (Main_hwnd==GetForegroundWindow() && BarCon.AutoOpen) Opening();
}
SHAppBarMessage(ABM_ACTIVATE,&Dat);
}

void CAppBar::AppBarQuerySetPos()
{
	WORD h,w;

Dat.rc.top=0;
Dat.rc.bottom=GetSystemMetrics(SM_CYSCREEN);
Dat.rc.left=0;
Dat.rc.right=GetSystemMetrics(SM_CXSCREEN);
SHAppBarMessage(ABM_QUERYPOS,&Dat);
w=FullBarWidth;
h=FullBarHeight;
if (FAutoHide) 
{
        h=FDelta;
        w=FDelta;
}
rc=Dat.rc;
switch (Dat.uEdge)
{
	case ABE_LEFT:  
        Dat.rc.right=Dat.rc.left+FullBarWidth;
        rc.right=rc.left+w;
        rc.left=rc.right-FullBarWidth;
		break;
	case ABE_RIGHT: 
        Dat.rc.left=Dat.rc.right-FullBarWidth;
        rc.left=rc.right-w;
		break;
	case ABE_TOP:   
        Dat.rc.bottom=Dat.rc.top+FullBarHeight;
        rc.bottom=rc.top+h;
        rc.top=rc.bottom-FullBarHeight;
		break;
	case ABE_BOTTOM: 
        Dat.rc.top=Dat.rc.bottom-FullBarHeight;
        rc.top=rc.bottom-w;
		break;
}
OpenPanelX=Dat.rc.left;
OpenPanelY=Dat.rc.top;
Dat.rc=rc;
SHAppBarMessage(ABM_SETPOS,&Dat);
if ((FAutoHide) && (!SetedHide)) 
{
        Dat.lParam=1;
        SHAppBarMessage(ABM_SETAUTOHIDEBAR,&Dat);
        SetedHide=TRUE;
}
ClosePanelX=rc.left;
ClosePanelY=rc.top;
if ((FEdge==ABE_LEFT) || (FEdge==ABE_RIGHT)) 
        MoveWindow(Main_hwnd,rc.left,rc.top,FullBarWidth,rc.bottom-rc.top,TRUE);
else    MoveWindow(Main_hwnd,rc.left,rc.top,rc.right-rc.left,FullBarHeight,TRUE);
GetWindowRect(Main_hwnd,&rc);
}

void CAppBar::AppBarCallBack (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UINT uState;
switch (wParam)
{
	case ABN_STATECHANGE:
        uState=SHAppBarMessage(ABM_GETSTATE,&Dat);
        if ((ABS_ALWAYSONTOP) && (uState=ABS_ALWAYSONTOP)) 
        SetWindowPos(Main_hwnd,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        else SetWindowPos(Main_hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	break;
	case ABN_FULLSCREENAPP:
         if (lParam) 
		 {
                SetWindowPos(Main_hwnd,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                Timer2->Stop();
				bFullScreen=TRUE;
         }
         else 
		 {
                SetWindowPos(Main_hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                if (FAutoHide) Timer2->Start();
				bFullScreen=FALSE;
         }
	break;
	case ABN_POSCHANGED:        
		AppBarQuerySetPos();
	break;
}
}

void CAppBar::CheckMouseOver()
{
	POINT	pp;
	BOOL	MOut;

if(FMouseDown) 
{
	MouseMove();
	return;
}
if (FEdge==ABE_DESKTOP) return;
GetCursorPos(&pp);
MOut=TRUE;
GetWindowRect(Main_hwnd,&rc);
switch (FEdge)
{
	case ABE_LEFT:       
		MOut=(pp.x>rc.right);
	break;
	case ABE_RIGHT:      
		MOut=(pp.x<rc.left);
	break;
	case ABE_TOP:        
		MOut=(pp.y>rc.bottom);
	break;
	case ABE_BOTTOM:     
		MOut=(pp.y<rc.top);
	break;
}
if (MOut) 
{
        MouseOver=FALSE;
        if (GetActiveWindow()!=Main_hwnd) Closing();
}
else MouseMove();
}

void CAppBar::Opening()
{
if ((FMouseDown) || (FEdge==ABE_DESKTOP)) return;
PanelOpenning=TRUE;
switch (FEdge)
{
	case ABE_LEFT:		
		Direct=FOpenSpeed;
	break;
	case ABE_RIGHT: 
		Direct=-FOpenSpeed;
	break;
	case ABE_TOP:   
		Direct=FOpenSpeed;
	break;
	case ABE_BOTTOM:
		Direct=-FOpenSpeed;
	break;
}
Timer1Event();
}

void CAppBar::Closing()
{
if ((FMouseDown) || (FEdge==ABE_DESKTOP)) return;
PanelOpenning=FALSE;
if ((FEdge==ABE_LEFT) || (FEdge==ABE_TOP)) Direct=-(FCloseSpeed);
else Direct=FCloseSpeed;
WindowToBottom();
Timer1Event();
}

void CAppBar::WindowPosChang()
{
if ((!FMouseDown) && (FEdge!=ABE_DESKTOP)) SHAppBarMessage(ABM_WINDOWPOSCHANGED,&Dat);
}

void CAppBar::UnRegisterBar()
{
if (!Registered) return;
Registered=FALSE;
if (FEdge==ABE_DESKTOP) return;
if (SetedHide) 
{
        Dat.lParam=0;
        SHAppBarMessage(ABM_SETAUTOHIDEBAR,&Dat);
}
SHAppBarMessage(ABM_REMOVE,&Dat);
}

CAppBar::CAppBar(HWND hWnd)
{
XTitle=0;
YTitle=0;
BarWidth=100;
BarHeight=100;
FullBarWidth=BarWidth;
FullBarHeight=BarHeight;
bNeedTitle=false;
Direct=0;
FAutoHide=TRUE;
FCloseSpeed=10;
FDelta=2;
FEdge=ABE_RIGHT;
FHeightOnDesktop=100;
FMouseDown=FALSE;
FOpenSpeed=10;
CanOpen=FALSE;
OpenDelay=0;
rgn=NULL;
FWidthOnDesktop=100;
MouseOver=FALSE;
Moveable=TRUE;
PanelOpenning=FALSE;
ReMoved=FALSE;
Registered=FALSE;
Dat.cbSize=sizeof(APPBARDATA);
Dat.uCallbackMessage=WM_APPBAR_EVENT;
if (hWnd==0) return;
Main_hwnd=hWnd;
Dat.hWnd=Main_hwnd;
Timer1=new CTimer(10,Main_hwnd,1);
Timer2=new CTimer(100,Main_hwnd,2);
Timer3=new CTimer(OpenDelay,Main_hwnd,5);
bFullScreen=FALSE;
}

CAppBar::~CAppBar()
{
UnRegisterBar();
delete Timer1;
delete Timer2;
delete Timer3;
}

byte CAppBar::DetectEdge(pSIZERECT rc)
{
	int rx,ry,rx1,ry1;
	POINT CursorPos;

GetCursorPos(&CursorPos);
Dat.rc.top=0;
Dat.rc.bottom=GetSystemMetrics(SM_CYSCREEN);
Dat.rc.left=0;
Dat.rc.right=GetSystemMetrics(SM_CXSCREEN);
SHAppBarMessage(ABM_QUERYPOS,&Dat);
if (Dat.rc.bottom==0) Dat.rc.bottom=GetSystemMetrics(SM_CYSCREEN);
if (Dat.rc.right==0) Dat.rc.right=GetSystemMetrics(SM_CXSCREEN);
rx=CursorPos.x-Dat.rc.left;
if (rx<1) rx=1;
rx1=Dat.rc.right-CursorPos.x;
if (rx1<1) rx1=1;
if (rx1<rx) rx=-rx1;
ry=CursorPos.y-Dat.rc.top;
if (ry<1) ry=1;
ry1=Dat.rc.bottom-CursorPos.y;
if (ry1<1) ry1=1;
if (ry1<ry) ry=-ry1;
if (abs(rx)>FullBarWidth) rx=1000;
if (abs(ry)>FullBarHeight) ry=1000;
rx1=0;
if (abs(ry)<abs(rx)) 
{
        rx=ry;
        rx1=1;
}
if (rx==1000)
{
        rc->left=StartPos.x-MovePos.x+CursorPos.x;
        rc->top=StartPos.y-MovePos.y+CursorPos.y;
        rc->width=FWidthOnDesktop;
        rc->height=FHeightOnDesktop;
		return ABE_DESKTOP;        
}
if (rx1==0) 
{
        rc->width=FullBarWidth;
        rc->height=Dat.rc.bottom-Dat.rc.top;
        rc->top=Dat.rc.top;
        if (rx>0) 
		{
                rc->left=Dat.rc.left;
				return ABE_LEFT;
		} else 
		{
                rc->left=Dat.rc.right-rc->width;
				return ABE_RIGHT;                
		}
} else 
{
        rc->width=Dat.rc.right-Dat.rc.left;
        rc->height=FullBarHeight;
        rc->left=Dat.rc.left;
        if (rx>0) 
		{
                rc->top=Dat.rc.top;
				return ABE_TOP;                
		} else 
		{
                rc->top=Dat.rc.bottom-rc->height;
                return ABE_BOTTOM;
        }
}
}

void CAppBar::MouseMove(WPARAM wParam)
{
	byte	aEdge;
	SIZERECT	rc;
if (FMouseDown) 
{
	if (!(wParam&MK_LBUTTON))
	{
		EndMoveWindow();
		return;
	}
        aEdge=DetectEdge(&rc);
        if (MoveEdge!=aEdge)
		{	
			MoveEdge=aEdge;
			SendMessage(Main_hwnd,WM_APPBARCHANGESTATE,MoveEdge,0);
			InvalidateRect(Main_hwnd,NULL,TRUE);
		}
        MoveWindow(Main_hwnd,rc.left,rc.top,rc.width,rc.height,TRUE);
} else
{
	if ((MouseOver) || (!FAutoHide)) return;
	if (!CanOpen && OpenDelay)
	{
		Timer3->Start();
		return;
	}
	MouseOver=TRUE;
	Opening();
}
}

void CAppBar::OnDelayTimer()
{
	Timer3->Stop();
	if (MouseOver) return;
	POINT pt;
	GetCursorPos(&pt);
	RECT rc;
	GetWindowRect(Main_hwnd,&rc);
	if (pt.x>=rc.left && pt.x<=rc.right && pt.y>=rc.top && pt.y<=rc.bottom)
	{
		MouseOver=TRUE;
		Opening();
	}
}

void CAppBar::StartMoveWindow(WORD le,WORD to)
{
if (!Moveable) return;
ChangeBarRgn(false);

FMouseDown=TRUE;
GetWindowRect(Main_hwnd,&rc);
MovePos.x=rc.left+FWidthOnDesktop/2;
MovePos.y=rc.top+17;
if (MoveEdge==ABE_DESKTOP) GetCursorPos(&MovePos);
StartPos.x=rc.left;
StartPos.y=rc.top;
MoveEdge=FEdge;
Timer2->SetInterval(15);
Timer2->Start();
}

void CAppBar::EndMoveWindow()
{
	byte aEdge;
	SIZERECT rc;

if (!FMouseDown) return;
aEdge=DetectEdge(&rc);
FMouseDown=FALSE;
Timer2->Stop();
if (FEdge==aEdge)return;
UnRegisterBar();
FEdge=aEdge;
if (aEdge!=MoveEdge) MoveEdge=aEdge;
RegisterBar();
}

void CAppBar::AppBarWinProc(HWND  hwnd,  UINT  uMsg,  WPARAM  wParam, LPARAM  lParam)
{
switch (uMsg)
{
case WM_WINDOWPOSCHANGED: 
	WindowPosChang();
	break;    
case WM_ACTIVATEAPP: 
	MesActivate(wParam);
	break;
case WM_TIMER:
	if (wParam==1) Timer1Event();
	if (wParam==2) CheckMouseOver();
	if (wParam==5) OnDelayTimer();
	break;
case WM_APPBAR_EVENT: 
	AppBarCallBack(uMsg,wParam,lParam);
	break;
case WM_MOUSEMOVE: 
	MouseMove(wParam);
	break;
case WM_LBUTTONDOWN: 
	StartMoveWindow(LOWORD(lParam),HIWORD(lParam));
	break;
case WM_LBUTTONUP: 
	EndMoveWindow();
	break;
}
}
