#include "windows.h"
#include "AppBar.h"
#include "BarUnit.h"
#include "Res\Resource.h"
#include <commctrl.h>
#include "CTimer.h"
#include "..\Headers\general.h"
#include "Compon\MyUtils.h"
#include "..\Headers\const.h"
#include "RegUnit.h"
#include "Option.h"
#include "BarStruct.h"
#include "Compon\FolderSw.h"
#include "UnitProp.h"
#include "Compon\ToolBar2.h"
#include "Sounds.h"

CAppBar* hBar=NULL;
CFolderSwitch*	Switch;
extern HPEN hpnColor1,hWhitePen,hGrayPen, hpnBlack;
extern HFONT		hMainFont;
HFONT	hVertFont;
extern HINSTANCE	hInstance;		  // Global instance handle for application
extern BarUnit*		nUnit;
extern PBarFolder	ActiveFolder,MainFolder,FirstFolder,nFolder;
extern BarUnit*	hMenuUnit;
extern char	buf[1000],buf2[230];
extern char	MainDir[200];
extern HINSTANCE hModUser32;

HWND MenuWnd;
HWND BarhWnd;
HANDLE HorLogo,VerLogo;
HWND hwndToolTip;
CTimer*		HourTimer;
CTimer*		TimeSyncTimer;
TBarConfig	BarCon;
byte	bIconSize,IconBorder,FullIcon;
HMENU	hButMenu;
int xTit;
int yTit;

//**************Защита***************
BOOL	bPassInputed;
CTimer*	PassTimer;

BOOL	BarCheckPassword(BOOL bSelfRequest=TRUE)
{
	if (!BarCon.UsedPassword) return TRUE;
	if (!bPassInputed && bSelfRequest)
	{
		bPassInputed=CheckPassword(TRUE,BarhWnd);
	}
	if (bPassInputed) BarUsePassword();
	return bPassInputed;
}

void	BarUsePassword()
{
	PassTimer->Stop();
	bPassInputed=TRUE;
	if (!BarCon.UsedPassword || !BarCon.RestorePassword) return;
	char in[]={2,5,10,20,30};
	PassTimer->SetInterval(in[BarCon.RestorePassword-1]*60000);
	PassTimer->Start();
}

void OnPassTimer()
{
	PassTimer->Stop();
	bPassInputed=FALSE;
}
//*************Конец защиты**************

void CheckSyncTime()
{
	if (!BarCon.SyncTime) return;
	FILETIME ft, ftLocal;
	SYSTEMTIME st;

	BarCon.LastSync=0;
	ReadRegistry("LastTimeBin",(LPBYTE)&BarCon.LastSync,sizeof(BarCon.LastSync));
 	if (BarCon.LastSync)
	{
		*((DWORDLONG *)&ft) = 10000 * (BarCon.LastSync + DWORDLONG(9435484800000));
		FileTimeToLocalFileTime(&ft, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
		TDate d,d2;
		d2=Now();
		d.Day=(byte)st.wDay;
		d.Month=(byte)st.wMonth;
		d.Year=st.wYear;
		DWORD del=EncodeDate(&d2)-EncodeDate(&d);
		const byte adel[4]={1,2,7,30};
		if (del<adel[BarCon.SyncTime-1]) return;
	}
	TimeSync(FALSE,BarhWnd);
	SetHourTimer();
}

void SetHourTimer()
{
SYSTEMTIME systim;
GetLocalTime(&systim);
int inter=3600000-systim.wMilliseconds-systim.wSecond*1000-systim.wMinute*60000;
if (inter>1800000) inter-=1800000;
HourTimer->SetInterval(inter);
HourTimer->Stop();
HourTimer->Start();
//в 15 и 45 минут.
inter=3600000-900000-systim.wMilliseconds-systim.wSecond*1000-systim.wMinute*60000;
if (inter>1800000) inter-=1800000;
if (inter<0) inter+=1800000;
TimeSyncTimer->SetInterval(inter);
TimeSyncTimer->Stop();
TimeSyncTimer->Start();
}

void HourTimerEvent()
{
SYSTEMTIME systim;
GetLocalTime(&systim);
if (!hBar->bFullScreen)
{
	if (systim.wMinute<35 && systim.wMinute>25)
	{
		if (lstrlen(BarCon.HalfHourFileName)) PlayWavFile(BarCon.HalfHourFileName,1);
	}
	else
	{
		if (lstrlen(BarCon.HourFileName))
		{
			if (BarCon.bCuckoo)
			{
				byte i=(byte)systim.wHour;
				if (systim.wMinute>56) i++;
				if (i>12) i-=12;
				if (!i) i=12;
				PlayWavFile(BarCon.HourFileName,i);
			}
			else PlayWavFile(BarCon.HourFileName,1);
		}
	}
}
SetHourTimer();
}

void CalcIconSize()
{
	if (BarCon.LargeIcons)
	{
		bIconSize=32;
		IconBorder=4;
	}
	else 
	{
		bIconSize=16;
		IconBorder=5;
	}
	FullIcon=bIconSize+2*IconBorder-2;
}

void UpdateIconSize()
{
	CalcIconSize();
  hBar->BarHeight=FullIcon+7;
  hBar->BarWidth=FullIcon+7;
	PlaceIcon();
	hBar->UpdateBar();
	
}

void UpdateIconInFolder(PBarFolder fol)
{
	if (fol==NULL) return;
	nUnit=fol->FirstUnit;
	while (nUnit!=NULL)
	{
		BitmapToUnitFromIcon(nUnit);
		nUnit=nUnit->NextUnit;
	}
}

void UpdateIcon()
{
	UpdateIconInFolder(MainFolder);
	nFolder=FirstFolder;
	while (nFolder!=NULL) 
	{
		UpdateIconInFolder(nFolder);
		nFolder=nFolder->NextFolder;
	}
	SaveMainBitmap();
	PlaceIcon();
}

void SetAutoHideChecked(HMENU hMenu)
{
	if (hBar->GetAutoHide()) CheckMenuItem(hMenu, ID_AUTOHIDE, MF_CHECKED);
	else CheckMenuItem(hMenu, ID_AUTOHIDE, MF_UNCHECKED);
}

void InitProc()
{   
	hwndToolTip=CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_CHILD | WS_POPUP | TTS_ALWAYSTIP, 
            CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, 
             BarhWnd, NULL, hInstance, NULL);
	SendMessage(hwndToolTip,TTM_SETDELAYTIME, TTDT_INITIAL,40);
  HorLogo=LoadImage(hInstance,"HORLOGO",IMAGE_BITMAP,0,0,0);
  VerLogo=LoadImage(hInstance,"VERLOGO",IMAGE_BITMAP,0,0,0);

   BarConfigFromRegistry(&BarCon);
	HourTimer=new CTimer(0,BarhWnd,3);
	TimeSyncTimer=new CTimer(0,BarhWnd,15);
	SetHourTimer();
	PassTimer=new CTimer(0,BarhWnd,4);
	//Установка параметров окна
  CalcIconSize();

  CONST LOGFONT lfvert = {
    20,                        //int   lfHeight;
    0,                         //int   lfWidth;
    900,                         //int   lfEscapement;
    900,                         //int   lfOrientation;
    FW_DONTCARE,               //int   lfWeight;
    0,                         //BYTE  lfItalic;
    0,                         //BYTE  lfUnderline;
    0,                         //BYTE  lfStrikeOut;
    RUSSIAN_CHARSET,           //BYTE  lfCharSet;
    OUT_DEFAULT_PRECIS,        //BYTE  lfOutPrecision;
    CLIP_DEFAULT_PRECIS,       //BYTE  lfClipPrecision;
    DEFAULT_QUALITY,           //BYTE  lfQuality;
    DEFAULT_PITCH | FF_SWISS,  //BYTE  lfPitchAndFamily;
    "Comic Sans MS",           //BYTE  lfFaceName[LF_FACESIZE];
    };

  hVertFont=CreateFontIndirect(&lfvert);

  xTit=0;
  yTit=0;
  hBar=new CAppBar(BarhWnd);
  hBar->bNeedTitle=BarCon.TitShow;
  hBar->BarHeight=FullIcon+7;
  hBar->BarWidth=FullIcon+7;
  hBar->SetAutoHide(BarCon.AutoHide);
  hBar->SetEdge(BarCon.Edge);
  hBar->SetSpeed(BarCon.Speed,BarCon.Speed);
  hBar->SetOpenDelay(BarCon.OpenDelay*10);
  Switch=new CFolderSwitch(BarhWnd);
  CreateBarStruct();
  Switch->UpdateToolTip();
  hBar->RegisterBar();
	MenuWnd=NULL;
	bPassInputed=FALSE;
}

void ShowOptions(BOOL Always)
{
    if ((BarCon.FName[0]==0) || (Always)) UserInfoToScreen(BarhWnd,FALSE);
}

void ExitFromProgram()
{
	delete Switch;
	delete HourTimer;
	delete PassTimer;
	delete TimeSyncTimer;
	FreeAllStruct();
	DeleteObject(HorLogo);
	DeleteObject(VerLogo);
	BarCon.AutoHide=hBar->GetAutoHide();
	BarCon.Edge=hBar->MoveEdge;
	if (BarCon.Edge==ABE_DESKTOP) 
	{
		RECT rc;
		GetWindowRect(BarhWnd,&rc);
		BarCon.DesktopX=(WORD)rc.left;
		BarCon.DeskTopY=(WORD)rc.top;
	}
	delete hBar;
	SaveOptions(&BarCon);
}

void PaintBar(HDC wdc)
{
RECT rc;
GetClientRect(BarhWnd,&rc);

TRIVERTEX vert[2] ;
GRADIENT_RECT gRect;
vert [0] .x = rc.left;
vert [0] .y = rc.top;
vert [0] .Red = GetRValue(ActiveFolder->BkColor)*256;
vert [0] .Green = GetGValue(ActiveFolder->BkColor)*256;
vert [0] .Blue = GetBValue(ActiveFolder->BkColor)*256;
vert [0] .Alpha = 0x0000;

vert [1] .x = rc.right;
vert [1] .y = rc.bottom;
vert [1] .Red = GetRValue(ActiveFolder->BkColor2 )*256;
vert [1] .Green = GetGValue(ActiveFolder->BkColor2)*256;
vert [1] .Blue = GetBValue(ActiveFolder->BkColor2)*256;
vert [1] .Alpha = 0x0000;

gRect.UpperLeft = 0;
gRect.LowerRight = 1;

/*HDC hMemDC = CreateCompatibleDC(wdc);
HANDLE b=CreateCompatibleBitmap(wdc,rc.right, rc.bottom);
SelectObject(hMemDC,b);
*/

GradientFill(wdc,vert,2,&gRect,1, hBar->MoveEdge==ABE_LEFT||hBar->MoveEdge==ABE_RIGHT ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);

/*BLENDFUNCTION bfn;
bfn.BlendOp = AC_SRC_OVER;
bfn.BlendFlags = 0;
bfn.SourceConstantAlpha = 100;

#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA		0x01
#endif

bfn.AlphaFormat = 1;

if (!AlphaBlend(wdc, rc.left, rc.top, rc.right, rc.bottom, 
		   hMemDC, rc.left, rc.top, rc.right, rc.bottom, bfn))
{
	int r=GetLastError();
}

DeleteObject(b);
DeleteDC(hMemDC);
*/
/*HBRUSH hb;

if (!ActiveFolder) hb=CreateSolidBrush(0);
else hb=CreateSolidBrush(ActiveFolder->BkColor);
FillRect(wdc,&rc,hb);
DeleteObject(hb);*/


HPEN hOldPen=(HPEN)SelectObject(wdc,hpnColor1);

SelectObject(wdc,hGrayPen);
if (hBar->GetTitleVisible())
{
	int dx=0;
	int dy=0;
	switch (hBar->MoveEdge)
	{
	case ABE_LEFT:
			dx=-1;
			break;
	case ABE_TOP:
			dy=-1;
			break;
	}
	MoveToEx(wdc,hBar->ptbar[0].x, hBar->ptbar[0].y,NULL);
	for (int i=1; i<8; i++)
	LineTo(wdc,hBar->ptbar[i].x+dx, hBar->ptbar[i].y+dy);
	if (ActiveFolder)
	{
		if (hBar->MoveEdge==ABE_LEFT || hBar->MoveEdge==ABE_RIGHT)
			SelectObject(wdc,hVertFont);
		else
			SelectObject(wdc,hMainFont);
		SetBkMode(wdc, TRANSPARENT);
		SIZE siz;
		GetTextExtentPoint32(wdc, ActiveFolder->Caption->Text,strlen(ActiveFolder->Caption->Text), &siz);
		SetTextColor(wdc, ActiveFolder->TitColor);

		switch (hBar->MoveEdge)
		{
		case ABE_LEFT:
			TextOut(wdc,hBar->BarWidth+TitSize/2-siz.cy/2-2, (hBar->ptbar[5].y+hBar->ptbar[2].y+siz.cx)/2, ActiveFolder->Caption->Text,strlen(ActiveFolder->Caption->Text));
			break;
		case ABE_RIGHT:
			TextOut(wdc,0+TitSize/2-siz.cy/2-2, (hBar->ptbar[5].y+hBar->ptbar[2].y+siz.cx)/2, ActiveFolder->Caption->Text,strlen(ActiveFolder->Caption->Text));
			break;
		case ABE_TOP:
			TextOut(wdc,(hBar->ptbar[5].x+hBar->ptbar[2].x-siz.cx)/2, hBar->BarHeight+TitSize/2-siz.cy/2-2, ActiveFolder->Caption->Text,strlen(ActiveFolder->Caption->Text));
			break;
		case ABE_BOTTOM:
			TextOut(wdc,(hBar->ptbar[5].x+hBar->ptbar[2].x-siz.cx)/2, 0+TitSize/2-siz.cy/2-2, ActiveFolder->Caption->Text,strlen(ActiveFolder->Caption->Text));
			break;
		}
	}
}
else 
{
	MoveToEx(wdc,rc.right-1,1,NULL);
	LineTo(wdc,rc.right-1,rc.bottom-1);
	LineTo(wdc,0,rc.bottom-1);
}
//MoveToEx(wdc,rc.right-1,0,NULL);
//LineTo(wdc,0,0);
//LineTo(wdc,0,rc.bottom-1);
SelectObject(wdc,hWhitePen);
MoveToEx(wdc,rc.right-2,1,NULL);
LineTo(wdc,1,1);
//LineTo(wdc,1,rc.bottom-2);
SelectObject(wdc,hOldPen);

HDC hCompDC=CreateCompatibleDC(wdc);
HBITMAP hOldBit;
if ((hBar->MoveEdge==ABE_TOP) || (hBar->MoveEdge==ABE_BOTTOM))
{
	hOldBit=(HBITMAP)SelectObject(hCompDC,VerLogo);
	BitBlt(wdc,2,hBar->YTitle+2,12,46,hCompDC,0,0,SRCCOPY);
} else
{
	hOldBit=(HBITMAP)SelectObject(hCompDC,HorLogo);
	if (hBar->MoveEdge==ABE_DESKTOP) BitBlt(wdc,hBar->GetWidthOnDesktop()/2-23,2,46,12,hCompDC,0,0,SRCCOPY);
	else BitBlt(wdc,hBar->XTitle+2,2,46,12,hCompDC,0,0,SRCCOPY);
}
SelectObject(hCompDC,hOldBit);
DeleteDC(hCompDC);
if ((xTit!=hBar->XTitle) || (yTit!=hBar->YTitle))
{
	xTit=hBar->XTitle;
	yTit=hBar->YTitle;
	PlaceIcon();
}
}

void RepaintBar()
{
	HDC hDC=GetDC(BarhWnd);
	PaintBar(hDC);
	ReleaseDC(BarhWnd, hDC);
}

void ButtonClick(BarUnit* b)
{
	if (b==NULL) return;
	if (b->Splitter) return;

	HINSTANCE hi=ShellExecute(BarhWnd, NULL, b->FilePath->Text, NULL, NULL, SW_NORMAL);
	if ((int)hi<32)
	{
		char bb[2];
		bb[1]=0;
		bb[0]=(int)hi+32;
		MessageBox(BarhWnd,b->FilePath->Text,"Ссылка",MB_OK);
		switch ((int)hi) {
		case 5:		MessageBox(BarhWnd,"ACCESS_DENIED","Код",MB_OK);
					break;
		case 2:		MessageBox(BarhWnd,"FILE_NOT_FOUND","Код",MB_OK);
					break;
		case 3:		MessageBox(BarhWnd,"PATH_NOT_FOUND","Код",MB_OK);
					break;
		case 6:		MessageBox(BarhWnd,"INVALID_HANDLE","Код",MB_OK);
					break;
		default:	MessageBox(BarhWnd,&bb[0],"Код",MB_OK);
		}
	}
}

void OptionFolder()
{
	if (BarCheckPassword())  UserInfoToScreen(BarhWnd, TRUE);
}


void DropFiles(HDROP hDrop)
{
POINT	cur;
GetCursorPos(&cur);
char pszFile[230];
RECT rc;
GetClientRect(BarhWnd,&rc);
cur.x-=rc.left;
cur.y-=rc.top;
WORD KolDrop=DragQueryFile(hDrop, 0xFFFFFFFF, pszFile, 230 );
WORD i=0;
while (i<KolDrop) 
{
   DragQueryFile(hDrop,i,pszFile, 230 );
   DropUnit(pszFile,(WORD)cur.x,(WORD)cur.y);
   i++;
}
DragFinish(hDrop);
PlaceIcon();
InvalidateRect(BarhWnd,NULL,TRUE);
SaveMainBitmap();
SaveBarStruct();
}

void LnkToDesktop()
{
GetDesktop((LPTSTR)&buf);
ExtractFileName(hMenuUnit->FilePath->Text,(LPTSTR)&buf2);
wsprintf((LPTSTR)&buf,"%s\x5c%s",(LPCTSTR)&buf,(LPCTSTR)&buf2);
CopyFile(hMenuUnit->FilePath->Text,(LPCTSTR)&buf,TRUE);
}

LONG FAR PASCAL BarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HMENU	hMenu;
	static POINT	point;
	int i;
	if (hBar) hBar->AppBarWinProc(hwnd,msg,wParam,lParam);
    switch(msg) {
	case WM_CREATE:
		BarhWnd=hwnd;
		InitProc();
        hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(MainPopupMenu));
        hMenu = GetSubMenu(hMenu,0);
        hButMenu = LoadMenu(hInstance, MAKEINTRESOURCE(ButtonPopupMenu));
        hButMenu = GetSubMenu(hButMenu,0);
		SetAutoHideChecked(hMenu);

		SetWindowLong(hwnd,GWL_EXSTYLE,GetWindowLong(hwnd,GWL_EXSTYLE)|0x80000);

		if (ActiveFolder)
			SetAlpha(ActiveFolder->Alpha);
		break;
	case WM_TIMER:
		if (wParam==3) HourTimerEvent();
		if (wParam==4) OnPassTimer();
		if (wParam==15) CheckSyncTime();
		break;
    case WM_RBUTTONDOWN:
	  point.x = LOWORD(lParam);
	  point.y = HIWORD(lParam);
      ClientToScreen(hwnd,&point);
	  if (MenuWnd!=NULL)  SendMessage(MenuWnd,WM_CANCELMODE,0,0);
	  EnableMenuItem(hMenu,IDD_DELETETOPIC, ActiveFolder==NULL ? MF_GRAYED:MF_ENABLED);
	  EnableMenuItem(hMenu,IDC_FOLDEROPTION, ActiveFolder==NULL ? MF_GRAYED:MF_ENABLED);
      TrackPopupMenu(hMenu, 0, point.x, point.y, 0, hwnd, NULL);
      MenuWnd=hwnd;
	  break;
  	case WM_ERASEBKGND:
		PaintBar((HDC)wParam);
		return 0;
	case WM_APPBARCHANGESTATE:
		PlaceIcon();
		BarCon.Edge=hBar->MoveEdge;
		SaveOptions(&BarCon);
		break;
	case WM_SWITCHVALUECHANGED:
		byte nom;
		if (ActiveFolder!=NULL) HideButton(ActiveFolder->FirstUnit);
		ActiveFolder=FirstFolder;
		for (nom=1; nom<wParam; nom++) if (ActiveFolder!=NULL) ActiveFolder=ActiveFolder->NextFolder;
		PlaceIcon();
		WriteWord2Ini("LastFolderIndex",(WORD)wParam);
		BarCon.LastFolderIndex=(WORD)wParam;
		if (ActiveFolder)
			SetAlpha(ActiveFolder->Alpha);
		break;
	case WM_SWITCHGETFOLDERNAME:
		nFolder=FirstFolder;
		i=1;
		while (nFolder && i++!=(int)wParam) nFolder=nFolder->NextFolder;
		if (nFolder) return (long)nFolder->Caption->Text;
		return NULL;
		break;
	case WM_DROPFILES:
		DropFiles((HDROP)wParam);
		break;
	case WM_TOOLBARBUTTONCLICK:
		ButtonClick((BarUnit*)wParam);
		break;
	case WM_TOOLBARBUTTONRCLICK:
		hMenuUnit=(BarUnit*)wParam;
		if (hMenuUnit!=NULL) 
		{
			GetCursorPos(&point);
			if (MenuWnd!=NULL) SendMessage(MenuWnd,WM_CANCELMODE,0,0);
			EnableMenuItem(hButMenu,ID_UNITPROP, hMenuUnit->Splitter ? MF_GRAYED:MF_ENABLED);
			EnableMenuItem(hButMenu,ID_UNITCRLNK, hMenuUnit->Splitter ? MF_GRAYED:MF_ENABLED);
			TrackPopupMenu(hButMenu, 0, point.x, point.y, 0, BarhWnd, NULL);
			MenuWnd=BarhWnd;
		}
		break;
	case WM_PASSWORDUSED:
		BarUsePassword();
		break;
	case WM_CHECKPASSWORD:
		if (BarCheckPassword(FALSE)) return OK_PASS;
		break;
	case WM_TIMECHANGE:
		SetHourTimer();
		break;
	case WM_DROPTOOLBUT:
		UnitDropEnd((BarUnit*)wParam, (POINT*)lParam);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case ID_AUTOHIDE:
			hBar->SetAutoHide(!hBar->GetAutoHide());
			BarCon.AutoHide=hBar->GetAutoHide();
			SetAutoHideChecked(hMenu);
			SaveOptions(&BarCon);
			break;
		case ID_EXIT:
			if (GetModuleHandle("OrgInet.dll"))
				MessageBox(hwnd,"Закройте пожалуйста модуль работы с интернетом","Ошибка",MB_ICONWARNING);
			else PostQuitMessage(0);
			break;
		case ID_OPTION:
			if (BarCheckPassword()) ShowOptions(TRUE);
			break;
		case ID_UPDATEICON:
			UpdateIcon();
			InvalidateRect(BarhWnd,NULL,TRUE);
			break;
		case ID_UNITDELETE:
			if (BarCheckPassword())
			{
				DeleteUnit(hMenuUnit);
				SaveMainBitmap();
				SaveBarStruct();
			}
			break;
		case ID_UNITCRLNK:
			LnkToDesktop();
			break;
		case ID_ADDICON:
			AddIcon(&point);
			break;
		case ID_ADDSPLITTER:
			AddSplitter(&point);
			break;
		case ID_RENAMEFOLDER:
			RenameFolder(ActiveFolder);
			break;
		case ID_ADDTOPIC:
			NewFolder();
			break;
		case IDC_FOLDEROPTION:
			OptionFolder();
			break;
		case IDD_DELETETOPIC:
			if (BarCheckPassword())	DeleteFolder(ActiveFolder);
			break;
		case ID_UNITRENAME:
			if (BarCheckPassword()) RenameUnit(hMenuUnit);
			break;
		case ID_UNITHIDE:
			if (BarCheckPassword()) 
			{
				hMenuUnit->Hidden=TRUE;
				PlaceIcon();
				hMenuUnit->ParentFolder->Modified=TRUE;
				SaveBarStruct();
			}
			break;
		case ID_UNITPROP:
			UnitProp(hMenuUnit);
			break;
		case IDC_ABOUT:
			ShowAbout(hwnd,"О программе. ",hInstance);
			break;
		case IDHELP:
			lstrcpy((LPTSTR)&buf,MainDir);
			lstrcat((LPTSTR)&buf,"Orgpanel.hlp");
			WinHelp(hwnd,(LPCTSTR)&buf,HELP_INDEX,0);
			break;
		}
    }
	return(DefWindowProc(hwnd, msg, wParam, lParam));
}

void SetSwitchValue(int Ind)
{
	byte n=0;
	nFolder=FirstFolder;
	while (nFolder!=NULL)
	{
		n++;
		nFolder=nFolder->NextFolder;
	}
	Switch->SetValue(Ind,n);
}

BOOL (WINAPI *SetFolderLayer)(HWND,COLORREF,BYTE,DWORD);
void SetAlpha(byte pAlpha)
{
	if (!hModUser32) return;
	SetFolderLayer =(int(__stdcall*)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hModUser32,"SetLayeredWindowAttributes");
	if (SetFolderLayer)
	SetFolderLayer(BarhWnd,0,255-pAlpha,0x02);
}