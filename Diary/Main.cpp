#include <windows.h>
#include <zmouse.h>
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "res/resource.h"
#include <mmsystem.h>
#include "../OrgPanel/ToolBar.h"
#include <commctrl.h>
#include "../Headers/exChange.h"
#include "Pages.h"
#include "Praz.h"
#include "Human.h"
#include "Event.h"
#include "../Headers/DynList.h"
#include "TrayBar.h"
#include "IntEvent.h"
#include "Print.h"
#include "Days.h"
#include "Dela.h"
#include "DelaList.h"
#include "../Addons/Logger/logger.h"

HWND hwndMain;
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow);
LONG FAR PASCAL FormWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HFONT	hMainFont;
HINSTANCE hInstance;		  // Global instance handle for application
char	DataDir[200];
char	MainDir[200];
HPEN	hpnColor1,hWhitePen,hGrayPen,hpnBlack;
HBRUSH	hBlueBrush,hWhiteBrush,hSilverBrush,hBlackBrush;
CToolBar*	bar;
HBITMAP hBack;
HFONT	hFont35,hFont14;
HWND hWndToolTip;
int		TopHeight;
BOOL	bSkyInfo;
BOOL	bFullImen;
char	Order[7];
double	GeoD;
double	GeoS;
TMode	Mode;

TDate	OwnDate;
BOOL	bResident;
int		iDeleteDelo;
int		iDeleteEvent;
HMENU	hIskDayMenu;

BOOL    bDnevModif;
int ActiveIDInListView;

extern TPage LeftPage, RightPage;
extern int PageWidth;
extern DynArray<TEVENT> EventList;
extern DynArray<TINTEVENT> IntEventList;
extern DynArray<TISKDAY> days;
extern DynArray<TDelo> DeloList;
extern HWND	hListView;

BOOL bMenuOnLeftPage;
BOOL bHumanExchangeOK=FALSE;
HACCEL accel;

#define IDC_VIEWDNEV 222
#define IDC_VIEWDELO 223

void UpdateToolBar()
{
	int typ=-1;
	BOOL bExecDelo=FALSE;
	if (Mode==mDnev)
	{
		PPage pp;
		int i;
		GetActiveNote(&pp,&i);
		if (pp)
		{
			//Есть активная строка
			typ=pp->st[i].IconType;
			if (typ==ID_DELA)
			{
				TDelo dl;
				GetDeloByID((int)pp->st[i].poi,&dl);
				bExecDelo=dl.factDate.Month;
			}
		}
	}
	else
	{
		if (ActiveIDInListView!=-1)
		{
			typ=ID_DELA;
			TDelo dl;
			GetDeloByID(ActiveIDInListView,&dl);
			bExecDelo=dl.factDate.Month;
		}
	}
	bar->EnableButton(IDC_CHDELO,typ==ID_DELA);
	bar->EnableButton(IDC_DELDELO,typ==ID_DELA);
	bar->EnableButton(IDC_DELOEXEC,typ==ID_DELA && !bExecDelo);

	bar->EnableButton(IDC_ADDEVENT,Mode!=mDelo);
	bar->EnableButton(IDC_CHEVENT,typ==ID_EVENT);
	bar->EnableButton(IDC_DELEVENT,typ==ID_EVENT);

	HMENU hMenu=GetMenu(hwndMain);
	HMENU hSub=GetSubMenu(hMenu,1);
	EnableMenuItem(hSub,IDC_CHDELO,typ==ID_DELA ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hSub,IDC_DELDELO,typ==ID_DELA ? MF_ENABLED : MF_GRAYED);
	hSub=GetSubMenu(hMenu,2);
	EnableMenuItem(hSub,IDC_ADDEVENT,Mode!=mDelo ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hSub,IDC_CHEVENT,typ==ID_EVENT ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hSub,IDC_DELEVENT,typ==ID_EVENT ? MF_ENABLED : MF_GRAYED);
}

void DnevToScreen(BOOL bSetNoActive)
{
	CalcAstro(&LeftPage);

	RightPage.Date=LeftPage.Date;
	RightPage.Date++;
	CalcAstro(&RightPage);
	UpdatePage(&LeftPage, bSetNoActive);
	UpdatePage(&RightPage, bSetNoActive);
	UpdateToolBar();
}

void InitDnev()
{
	LeftPage.Tab=10;
	RightPage.Tab=17;

	LeftPage.Date=Now();
	LeftPage.CountSt=0;
	RightPage.CountSt=0;
	DnevToScreen(TRUE);
}

void CloseDnev()
{
	EventList.Clear();
	IntEventList.Clear();
}

void LoadDnev()
{
	char* temp=(char*)malloc(220);
	lstrcpy(temp,DataDir);
	lstrcat(temp,DNEV_FILE);
	bDnevModif=FALSE;
	HANDLE hFile=CreateFile(temp,GENERIC_READ, 0,0, OPEN_EXISTING,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD dw;
		ReadFile(hFile,temp,5,&dw,NULL);
		if (temp[0]!='d' || temp[1]!='n' || temp[2]!='e' || temp[3]!='v')
		{
			MessageBox(hwndMain,"Файл не является файлом дневника","Ошибка",MB_ICONSTOP);
			CloseHandle(hFile);
			free(temp);
			return;
		}
		if (temp[4]!='3')
		{
			MessageBox(hwndMain,"Неверная версия файла дневника","Ошибка",MB_ICONSTOP);
			CloseHandle(hFile);
			free(temp);
			return;
		}
		EventList.LoadFromFile(hFile);
		IntEventList.LoadFromFile(hFile);
		days.LoadFromFile(hFile);
		DeloList.LoadFromFile(hFile);
		CloseHandle(hFile);
	}
	free(temp);
}

void SaveDnev()
{
	if (!bDnevModif) return;
	char* temp=(char*)malloc(220);
	lstrcpy(temp,DataDir);
	lstrcat(temp,DNEV_FILE);
	HANDLE hFile=CreateFile(temp,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		temp[0]='d';
		temp[1]='n';
		temp[2]='e';
		temp[3]='v';
		temp[4]='3';
		DWORD dw;
		WriteFile(hFile,temp,5,&dw,NULL);
		EventList.SaveToFile(hFile);
		IntEventList.SaveToFile(hFile);
		days.SaveToFile(hFile);
		DeloList.SaveToFile(hFile);
		CloseHandle(hFile);
	}
	free(temp);
	bDnevModif=FALSE;
}

void ReadConfig()
{
	DWORD dw;
	ReadRegistry("ShowSkyInfo",(byte*)&dw,4);
	bSkyInfo=(BOOL)dw;
	TopHeight=bSkyInfo ? 130 : 75;
	ReadRegistry("Dnev",(byte*)&Order,7);
	ReadRegistry("BDate",(byte*)&OwnDate,sizeof(TDate));

	char ch[10];
	ReadRegistry("GeoD",(byte*)&ch[0],10);
	GeoD=atof(ch);
	ReadRegistry("GeoS",(byte*)&ch[0],10);
	GeoS=atof(ch);

	ReadRegistry("FullImen",(byte*)&dw,4);
	bFullImen=(BOOL)dw;
	
	ReadRegistry("DeleteEvent",(byte*)&iDeleteEvent,4);
	ReadRegistry("DeleteDelo",(byte*)&iDeleteDelo,4);
}

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow)
{
	MSG        msgMain;
    WNDCLASS   wc;
    hInstance = hInst;
	HWND hPrev=FindWindow(DiaryWindow,NULL);
	if (hPrev)
	{
		ShowWindow(hPrev,SW_SHOWNORMAL);
		return 0;
	}
	GetDataDir(DataDir,200);
	GetModulePath(hInstance,MainDir,200);

	wc.lpszMenuName		= MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName	= DiaryWindow;
	wc.hInstance        = hInst;
	wc.hIcon			= LoadIcon(hInstance,"1MainIcon");
	wc.hCursor          = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wc.style            = CS_DBLCLKS;
	wc.lpfnWndProc		= FormWndProc;
	wc.cbClsExtra       = 0;
	wc.cbWndExtra       = 0;

	if (!RegisterClass(&wc)) return(0);

	if ((hwndMain = CreateWindowEx(0,DiaryWindow,
			 "OrgPanel - Дневник",
                             WS_OVERLAPPEDWINDOW,
                             0, 0,
                             390, 550,
                             NULL, NULL, hInst, NULL)) == NULL)
    return(0);
	PutOnCenter(hwndMain);
	Mode=mDnev;

	ReadConfig();
	bResident=FALSE;
	if (lstrlen(lpstrCmdLine)>1)
	{
		if (lpstrCmdLine[0]=='/' && lpstrCmdLine[1]=='r') bResident=TRUE;
	}

	if (!bResident)
	{
		if (!CheckPassword(FALSE, hwndMain))
		{
			DestroyWindow(hwndMain);
			UnregisterClass(DiaryWindow,hInstance);
			return 0;
		}
	}
#ifdef DEBUG
InitLog("c:\\dnev.log");
WriteLog("Started");
#endif

	LoadDnev();

	hMainFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hWhitePen = CreatePen(PS_SOLID,1,0x0ffffff);
	hGrayPen = CreatePen(PS_SOLID,1,0x808080);
	hBlueBrush=CreateSolidBrush(0xff0000);
	hpnBlack	= CreatePen(PS_SOLID,1,0);
	hWhiteBrush=CreateSolidBrush(0xffffff);
	hSilverBrush=CreateSolidBrush(0xc0c0c0);
	hBlackBrush = CreateSolidBrush(0);
	hBack = LoadBitmap(hInstance,"BACK");
	hpnColor1 = CreatePen(PS_SOLID,1,0x0e0e0e0);

	InitCommonControls();
	hWndToolTip=CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_CHILD | WS_POPUP | TTS_ALWAYSTIP, 
      CW_USEDEFAULT, CW_USEDEFAULT, 10, 10,
      hwndMain, NULL, hInstance, NULL);
	bar=new CToolBar(hwndMain,0,0,29,TRUE,hWndToolTip);
	bar->AddButton("Добавить дело",	 IDC_ADDDELO, "ADDDELO",	TBSTYLE_BUTTON);
	bar->AddButton("Изменить дело",	 IDC_CHDELO, "CHDELO",	TBSTYLE_BUTTON);
	bar->AddButton("Удалить дело",	 IDC_DELDELO, "DELDELO",	TBSTYLE_BUTTON);
	bar->AddButton("Дело выполнено",	 IDC_DELOEXEC, "DELOEXEC",	TBSTYLE_BUTTON);
	bar->AddButton("",	 IDC_ABOUT+200, "",	TBSTYLE_SEP);
	bar->AddButton("Добавить событие",	 IDC_ADDEVENT, "ADDEVENT",	TBSTYLE_BUTTON);
	bar->AddButton("Изменить событие",	 IDC_CHEVENT, "CHEVENT",	TBSTYLE_BUTTON);
	bar->AddButton("Удалить событие",	 IDC_DELEVENT, "DELEVENT",	TBSTYLE_BUTTON);
	bar->AddButton("",	 IDC_ABOUT+201, "",	TBSTYLE_SEP);
	bar->AddButton("Дневник",	 IDC_VIEWDNEV, "VIEWDNEV",	TBSTYLE_BUTTON);
	bar->AddButton("Список дел",		IDC_VIEWDELO, "VIEWDELO",	TBSTYLE_BUTTON);
	bar->AddButton("",	 IDC_ABOUT+203, "",	TBSTYLE_SEP);
	bar->AddButton("О программе",	 IDC_ABOUT, "ABOUT",	TBSTYLE_BUTTON);
	bar->AddButton("Свернуть",	 IDC_MIN, "MIN",	TBSTYLE_BUTTON);

	InitDelaList();

	HFONT hFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);//=(HFONT)SendMessage(hwndMain,WM_GETFONT,0,0);
	LOGFONT lf;
	GetObject(hFont,sizeof(lf),&lf);
	lstrcpy(lf.lfFaceName,"Arial");
	lf.lfCharSet=RUSSIAN_CHARSET;
	lf.lfHeight=45;
	hFont35=CreateFontIndirect(&lf);
	lf.lfHeight=28;
	hFont14=CreateFontIndirect(&lf);

	OnSize();
	InitPraz();
	InitDnev();
	InitPages();
	SetTrayBar();// Должно быть перед InitHuman
	InitHuman();
	InitPrinter();

	if (!bResident)
	{
		ShowWindow(hwndMain,SW_SHOW);
		UpdateWindow(hwndMain);
	}

	accel=LoadAccelerators(hInstance,"ACCEL");
    while (GetMessage((LPMSG) &msgMain, NULL, 0, 0))
    {
		if (!TranslateAccelerator(hwndMain,accel,(LPMSG)&msgMain))
		{
			TranslateMessage((LPMSG) &msgMain);
			DispatchMessage((LPMSG) &msgMain);
		}
    }
	RemoveIcon();
	FreePraz();
	FreeHuman();
	SaveDnev();
	CloseDnev();
	FreePages();
	DeleteObject(hWhitePen);
	DeleteObject(hGrayPen);
	DeleteObject(hMainFont);
	DeleteObject(hBack);
	DeleteObject(hpnColor1);
	DeleteObject(hFont35);
	DeleteObject(hFont14);
	DeleteObject(hBlueBrush);
	DeleteObject(hWhiteBrush);
	DeleteObject(hSilverBrush);
	DeleteObject(hBlackBrush);
	DeleteObject(hpnBlack);
	delete bar;
    return(0);
}

void OnMouseRDown(WORD x, WORD y)
{
	if (Mode==mDelo) return;
	RECT rc;
	GetClientRect(hwndMain,&rc);
	int h=rc.bottom-rc.top;
	if (y<TopHeight && y>48)
	{
		bMenuOnLeftPage=(x<PageWidth-5 && x>10);
		SendMessage(hwndMain,WM_CANCELMODE,0,0);
		POINT pt;
		GetCursorPos(&pt);
		TDate dd=bMenuOnLeftPage ? LeftPage.Date : RightPage.Date;
		CheckMenuItem(hIskDayMenu,ID_ISKDAY, GetDayColor(&dd)!=0 ? MF_CHECKED : MF_UNCHECKED);

		TrackPopupMenu(hIskDayMenu, 0, pt.x, pt.y, 0, hwndMain, NULL);
	}
}

void OnViewDeloPage()
{
	if (Mode==mDelo) return;
	ActiveIDInListView=-1;
	Mode=mDelo;
	UpdateDelaPage();
}

void OnViewDnevPage()
{
	if (Mode==mDnev) return;
	Mode=mDnev;
	CloseDelaPage();
}

void OnMouseDown(WORD x, WORD y)
{
	if (Mode==mDelo) return;
	RECT rc;
	GetClientRect(hwndMain,&rc);
	int h=rc.bottom-rc.top;

	if (y<TopHeight && y>48)
	{
		if (x<PageWidth-5 && x>10)
		{
			LeftPage.Date--;
			MySound();
		}
		if (x>=PageWidth-5 && x<=PageWidth+25) 
		{
			StartCalendar(&LeftPage.Date,hwndMain,FALSE);
		}
		if (x>PageWidth+25 && x<PageWidth*2+10) 
		{
			LeftPage.Date=RightPage.Date;
			MySound();
		}
		DnevToScreen(TRUE);
		PaintDnev();
		return;
	}
	if (y>TopHeight && y<h-47)
	{
		SetActiveNote(x,y,h);
		return;
	}
	if (y>h-43 && y<h-35)
	{
		int lp=LeftPage.x0+(PageWidth>>1);
		if (x>lp+12 && x<lp+28 && LeftPage.TopBut) LeftPage.TopStIndex--;
		if (x>lp-28 && x<lp-12 && LeftPage.BotBut) LeftPage.TopStIndex++;
		lp=RightPage.x0+(PageWidth>>1);
		if (x>lp+12 && x<lp+28 && RightPage.TopBut) RightPage.TopStIndex--;
		if (x>lp-28 && x<lp-12 && RightPage.BotBut) RightPage.TopStIndex++;
		PaintDnev();
	}
}

void OnWheel(short Delta, WORD x, WORD y)
{
	if (Mode==mDelo) return;
	RECT rc;
	GetWindowRect(hwndMain,&rc);
	PPage pp=&LeftPage;
	if (x-rc.left>LeftPage.x0+PageWidth) pp=&RightPage;
	if (Delta>0 && pp->TopBut) pp->TopStIndex--;
	if (Delta<0 && pp->BotBut) pp->TopStIndex++;
	PaintDnev();
}

void OnMouseDbl(WORD x, WORD y)
{
	if (Mode==mDelo) return;

	RECT rc;
	GetClientRect(hwndMain,&rc);
	int h=rc.bottom-rc.top;

	if (y>h-52) return;

	int typ=-1;
	PPage pp;
	int i;
	GetActiveNote(&pp,&i);
	if (pp) typ=pp->st[i].IconType;

	if (typ==ID_ROV || typ==ID_IMEN)
	{
		PEXCHANGE pex=(PEXCHANGE)pp->st[i].poi;
		if (!pex) return;
		HWND hh=FindWindow(NoteBookWindow,NULL);
		if (hh) SendMessage(hh,WM_OPENHUMAN,pex->Key,0);
		else 
		{
			char *temp=(char*)malloc(300);
			lstrcpy(temp,MainDir);
			lstrcat(temp,"NoteBook.exe");
			char ck[10];
			wsprintf(ck,"%d",pex->Key);
			ShellExecute(hwndMain,NULL,temp,ck,NULL,SW_SHOW);
			free(temp);
		}
		return;
	}
	if (typ==ID_EVENT) ChangeEvent();
	if (typ==ID_DELA) ChangeDelo();
}

LONG FAR PASCAL FormWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HMENU hMenu;

    switch(msg)
	{
	case WM_GETMINMAXINFO:
		LPMINMAXINFO mx;
		mx=(LPMINMAXINFO)lParam;
		mx->ptMinTrackSize.x=480;
		mx->ptMinTrackSize.y=340;
		break;
	case WM_LBUTTONUP:
		OnMouseDown(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_RBUTTONUP:
		OnMouseRDown(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_LBUTTONDBLCLK:
		OnMouseDbl(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_MOUSEWHEEL:
		OnWheel((short)HIWORD(wParam),LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_SIZE:
		OnSize();
		break;
	case WM_COMMAND:
		if (!HIWORD(wParam) || HIWORD(wParam)==1)
		switch (LOWORD(wParam))
		{
		case IDC_ABOUT:
			ShowAbout(hwnd,"Дневник.",hInstance);
			break;
		case IDHELP:
			char* buf;
			buf=(char*)malloc(230);
			lstrcpy(buf,MainDir);
			lstrcat(buf,"Orgpanel.hlp");
			WinHelp(hwnd,buf,HELP_CONTEXT,2);
			free (buf);
			break;
		case IDC_PRINTSETUP:
			ShowPrintDlg();
			break;
		case IDC_PRINT:
			Print();
			break;
		case IDC_BIO:
			ShowBioritm(hwnd);
			break;
		case IDC_ADDEVENT:
			NewEvent();
			break;
		case IDC_CHEVENT:
			ChangeEvent();
			break;
		case IDC_DELEVENT:
			DeleteEvent();
			break;
		case IDC_ADDDELO:
			NewDelo();
			break;
		case IDC_CHDELO:
			ChangeDelo();
			break;
		case IDC_DELDELO:
			DeleteDelo();
			break;
		case ID_OPENWINDOW:
			OnIconDblClick();
			break;
		case ID_EXIT:
		case IDC_EXIT:
			if (MessageDlg(hwndMain,"Завершить работу с","    дневником","Выход из программы")==IDNO) break;
	        PostQuitMessage(0);
			break;
		case IDC_MIN:
			ShowWindow(hwndMain,SW_HIDE);
			UsePassword();
			SaveDnev();
			bResident=TRUE;
			break;
		case ID_CHANGEDATE:
			StartCalendar(&LeftPage.Date,hwndMain,FALSE);
			DnevToScreen(TRUE);
			PaintDnev();
			break;
		case ID_ISKDAY:
			ChangeIskDay(bMenuOnLeftPage ? &LeftPage.Date : &RightPage.Date);
			UpdateIntEvent();
			DnevToScreen(TRUE);
			PaintDnev();
			bDnevModif=TRUE;
			break;
		case IDC_VIEWDELO:
			OnViewDeloPage();
			break;
		case IDC_VIEWDNEV:
			OnViewDnevPage();
			break;
		case IDM_PRAZEDIT:
			PrazEdit();
			DnevToScreen(TRUE);
			PaintDnev();
			break;
		case IDC_DELOEXEC:
			DeloExec();
			break;
		case IDM_SVOD:
			ShowSvod();
			break;
		}
		break;
	case WM_FILLBIOCOMBO:
		FillBioCombo((HWND)wParam);
		break;
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hwnd,&ps);
		PaintDnev();
		EndPaint(hwnd,&ps);
		break;
	case WM_ERASEBKGND:
		if (Mode==mDelo)
		{
			PaintBack((HDC)wParam);
			return TRUE;
		}
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
		break;
	case WM_STARTEXCHANGE:
		OnStartExchange(wParam);
		break;
	case WM_CONFIGCHANGE:
		ReadConfig();
		break;
	case WM_SYSCOMMAND:
		if (wParam==SC_MINIMIZE)
		{
			ShowWindow(hwndMain,SW_HIDE);
			UsePassword();
			SaveDnev();
			bResident=TRUE;
			return 1;
		}
		if (wParam==SC_CLOSE)
		{
			if (MessageDlg(hwndMain,"Завершить работу с","    дневником","Выход из программы")==IDNO) return 1;
		}
		break;
	case WM_CREATE:
        hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
        hMenu = GetSubMenu(hMenu,0);
		hIskDayMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENUDAY));
        hIskDayMenu = GetSubMenu(hIskDayMenu ,0);
		break;
	case WM_TRAYBARCALLBACK:
		if (lParam==WM_RBUTTONUP)
		{
			POINT pt;
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hwnd, NULL);
		}
		if (lParam==WM_LBUTTONDBLCLK)
			OnIconDblClick();
		break;
	case WM_CALENDARDATECHANGE:
		UpdateDelaList();
		break;
	case WM_NOTIFY:
		if (wParam==LV_ID)
		{
			NM_LISTVIEW* lpnm;
			lpnm=(NM_LISTVIEW*)lParam;
			if (lpnm->hdr.code==LVN_ITEMCHANGED && lpnm->uChanged&LVIF_STATE)
			{
				if (!lpnm->uNewState) ActiveIDInListView=-1;
				else ActiveIDInListView=lpnm->lParam;
				UpdateToolBar();
			}
			if (lpnm->hdr.code==NM_DBLCLK) ChangeDelo();
		}
	}
    return(DefWindowProc(hwnd, msg, wParam, lParam));
}
