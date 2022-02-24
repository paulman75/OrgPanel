#include <windows.h>
#include "ListView.h"
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "res/resource.h"
#include "../Headers/toolbar.h"
#include "column.h"
#include "Units.h"
#include "EditUnit.h"
#include "EditFirm.h"
#include "mytapi.h"
#include "Print.h"
#include "Filter.h"
#include "search.h"
#include "todnev.h"
#include "../Headers/Exchange.h"

HWND hwndMain;
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow);
LONG FAR PASCAL FormWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HFONT	hMainFont;
HINSTANCE hInstance;		  // Global instance handle for application
CSortListView*	cList;
CToolBar*		bar;
HBITMAP	hBit1,hBit2;
char MainDir[200];
HPEN	hWhitePen,hGrayPen;
HWND	hFilterBox;
HWND	hStatusBar;
HWND	hFilterEdit[4];
HWND	hFilterStatic[4];
extern  char Filters[2][3][30];
BOOL	bHumanMode;
extern  LPNOTEUNIT	ActiveUnit;
extern  LPFIRMUNIT	ActiveFirm;
extern  LPNOTEUNIT	FirstUnit;
extern  LPNOTEUNIT	nNote;
LONG Pos[5]={100,200,450,260,0};
extern BOOL	bHumanEditing;
HACCEL accel;

#define IDC_ADDHUMAN	1
#define IDC_ADDFIRM		2
#define IDC_MODE		3
#define IDC_ACTION		4
#define IDC_HEL			5
#define IDC_SEP1		6

#define IDC_STATUSBAR	100

char* cFilterSt[3]={"Имя","Фамилия","E-Mail"};
char* cFilterSt2[3]={"Название","Описание","E-Mail"};

void SetMenuText(HMENU hMenu)
{
	MENUITEMINFO mi;
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.fMask=MIIM_TYPE;
	GetMenuItemInfo(hMenu,IDC_HOMERING,FALSE,&mi);
	mi.dwTypeData=bHumanMode ? "Позвонить домой" : "Позвонить по тел. №1";
	SetMenuItemInfo(hMenu,IDC_HOMERING,FALSE,&mi);
	mi.cch=0;
	mi.dwTypeData=NULL;
	GetMenuItemInfo(hMenu,IDC_WORKRING,FALSE,&mi);
	mi.dwTypeData=bHumanMode ? "Позвонить на работу" : "Позвонить по тел. №2";
	SetMenuItemInfo(hMenu,IDC_WORKRING,FALSE,&mi);
}

void SetMode(BOOL bHuman)
{
	if (bHuman==bHumanMode) return;

	ActiveUnit=NULL;
	ActiveFirm=NULL;
	CheckSortOrder();
	bHumanMode=bHuman;
	ShowWindow(hFilterEdit[3],bHumanMode ? SW_SHOW : SW_HIDE);
	ShowWindow(hFilterStatic[3],bHumanMode ? SW_SHOW : SW_HIDE);
	HMENU hMenu=GetMenu(hwndMain);
	CheckMenuItem(hMenu,IDC_FIRMDATA, bHuman ? MF_UNCHECKED : MF_CHECKED);
	CheckMenuItem(hMenu,IDC_HUMANDATA, bHuman ? MF_CHECKED : MF_UNCHECKED);
	bar->ShowButton(IDC_ADDHUMAN,bHuman);
	bar->ShowButton(IDC_ADDFIRM,bHuman==false);
	SetMenuText(hMenu);
	for (int i=0; i<3; i++) SetWindowText(hFilterStatic[i],bHuman ? cFilterSt[i] : cFilterSt2[i]);
	UpdateAllData();
}

#define NoteBookReg "NoteBook"
HKEY hKey;

BOOL ReadRegistry(LPCTSTR Cap, LPVOID poi, DWORD len)
{
	if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RegistryPath,0,KEY_ALL_ACCESS,&hKey)) return FALSE;
	BOOL res=(ERROR_SUCCESS==RegQueryValueEx(hKey,Cap,NULL,NULL,(LPBYTE)poi,&len));
	RegCloseKey(hKey);
	return res;
}

void WriteBin(LPBYTE Val, DWORD len)
{
	if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RegistryPath,0,KEY_ALL_ACCESS,&hKey)) return;
	RegSetValueEx(hKey,NoteBookReg,0,REG_BINARY,Val,len);
	RegCloseKey(hKey);
}

void Ring(int i)
{
	LPTSTR s;
	if (bHumanMode)
	{
		if (i==1) s=ActiveUnit->Fields[UN_HOMETEL];
		else s=ActiveUnit->Fields[UN_WORKTEL];
	}
	else
	{
		if (i==1) s=ActiveFirm->Fields[UNF_TEL1];
		else ActiveFirm->Fields[UNF_TEL2];
	}
	BOOL PulseDial;
	char* Prefix=(char*)malloc(20);
	ReadRegistry("PulseDial",&PulseDial,4);
	ReadRegistry("Prefix",Prefix,20);
	char* temp=(char*)malloc(100);
	temp[0]=0;
	temp[1]=0;
	if (PulseDial) temp[0]=80;
	if (lstrlen(Prefix)>0)
	{
		lstrcat(temp,Prefix);
		lstrcat(temp,"w");
	}
	lstrcat(temp,s);
	MakeCall(hwndMain, temp);
	free(temp);
	free(Prefix);
}

void SendEMail(HWND hDlg)
{
	char* temp=(char*)malloc(200);
	lstrcpy(temp,"mailto:");
	if (bHumanMode)
		lstrcat(temp,ActiveUnit->Fields[UN_EMAIL]);
	else
		lstrcat(temp,ActiveFirm->Fields[UNF_EMAIL]);
	lstrcat(temp,"?body=");
	if (bHumanMode)
	{
		lstrcat(temp,"Здравствуйте ");
		if (ActiveUnit->Fields[UN_NAME])
		{
			lstrcat(temp,ActiveUnit->Fields[UN_NAME]);
			lstrcat(temp," ");
		}
		if (ActiveUnit->Fields[UN_MNAME])
			lstrcat(temp,ActiveUnit->Fields[UN_MNAME]);
		lstrcat(temp,".");
	}
	lstrcat(temp,"С уважением ");
	char* Name=(char*)malloc(15);
	ReadRegistry("SName",Name,15);
	lstrcat(temp,Name);
	ReadRegistry("FName",Name,15);
	lstrcat(temp," ");
	lstrcat(temp,Name);
	free(Name);

	ShellExecute(hDlg,"open",temp,"","",SW_SHOW);
	free(temp);
}

void GotoHomePage(HWND hDlg)
{
	char* temp=(char*)malloc(200);
	LPTSTR s;
	if (bHumanMode) s=ActiveUnit->Fields[UN_HOMEPAGE];
	else s=ActiveFirm->Fields[UNF_HOMEPAGE];
	if (FindInString(s,"://")==-1)
	{
		lstrcpy(temp,"http://");
		lstrcat(temp,s);
	}
	else lstrcpy(temp,s);
	GotoUrl(hDlg,temp);
	free(temp);
}

void OnSize(WORD w, WORD h)
{
	MoveWindow(cList->GetWnd(),0,56,w,h-132,TRUE);
	MoveWindow(hFilterBox,0,h-75,w,55,TRUE);
	MoveWindow(hStatusBar,0,h-20,w,20,TRUE);
	for (int i=0; i<4; i++)
	{
		int we=100;
		we=w/5;
		if (we<100) we=100;
		int k=(w-4*we)/5*(i+1)+we*i;
		MoveWindow(hFilterEdit[i],k,h-48,we,i==3 ? 100 : 20,TRUE);
		MoveWindow(hFilterStatic[i],k,h-62,we,14,TRUE);
	}
}

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow)
{
	MSG        msgMain;
    WNDCLASS   wc;
    hInstance = hInst;
	if (FindWindow(NoteBookWindow,NULL)) return 0;

	GetDataDir(MainDir,200);

	if (lstrlen(lpstrCmdLine)>1)
	{
		if (lpstrCmdLine[0]=='/' && lpstrCmdLine[1]=='p')
		{
			LoadUnits();
			OnInitExchange();
			DeleteUnits();
			return 0;
		}

	}

		wc.lpszMenuName		= "Main";
		wc.lpszClassName	= NoteBookWindow;
        wc.hInstance        = hInst;
		wc.hIcon			= LoadIcon(hInstance,"MainIcon");
        wc.hCursor          = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
        wc.style            = 0;
		wc.lpfnWndProc		= FormWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;

        if (!RegisterClass(&wc))
            return(0);
		ReadRegistry(NoteBookReg,&Pos,10);

		if ((hwndMain = CreateWindowEx(0,NoteBookWindow,
				 "OrgPanel - Записная книжка",
                                 WS_OVERLAPPEDWINDOW,
                                 Pos[0], Pos[1],
                                 Pos[2], Pos[3],
                                 NULL, NULL, hInst, NULL)) == NULL)
        return(0);

	if (!CheckPassword(FALSE, hwndMain))
	{
		DestroyWindow(hwndMain);
		UnregisterClass(NoteBookWindow,hInstance);
		return 0;
	}
	InitPrinter();
	hMainFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hWhitePen = CreatePen(PS_SOLID,1,0x0ffffff);
	hGrayPen = CreatePen(PS_SOLID,1,0x808080);

	bar=new CToolBar(hwndMain,0,0,60,50,0,NULL,TRUE);

	bar->AddButton("Добавить",IDC_ADDHUMAN,"ADDHUMAN",TBSTYLE_BUTTON);
	bar->AddButton("Добавить",IDC_ADDFIRM,"ADDFIRM",TBSTYLE_BUTTON,FALSE);
	bar->AddButton("Изменить",IDC_CHANGE,"CHANGE",TBSTYLE_BUTTON);
	bar->AddButton("Удалить",IDC_DELETE,"DELETE",TBSTYLE_BUTTON);
	bar->AddButton(NULL,IDC_SEP1,NULL,TBSTYLE_SEP);
	bar->AddButton("Режим",IDC_MODE,"MODE",TBSTYLE_BUTTON);
	bar->AddButton("Действие",IDC_ACTION,"ACT",TBSTYLE_BUTTON,TRUE,TRUE,"ACTMENU");
	bar->AddButton(NULL,IDC_SEP1+1,NULL,TBSTYLE_SEP);
	bar->AddButton("Поиск",IDC_SEARCH,"SEARCH",TBSTYLE_BUTTON);

	hFilterBox=CreateWindow("BUTTON","Фильтр",WS_CHILD | WS_VISIBLE | BS_GROUPBOX |WS_CLIPCHILDREN,0,0,0,0,hwndMain,NULL,hInstance,NULL);
	SendMessage(hFilterBox,WM_SETFONT,(LPARAM)hMainFont,0);

	for (byte i=0; i<3; i++)
	{
		hFilterEdit[i]=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_CHILD | WS_VISIBLE,0,0,0,0,hwndMain,NULL,hInstance,NULL);
		SetWindowLong(hFilterEdit[i],GWL_ID,IDC_FILTEREDIT+i);
		SendMessage(hFilterEdit[i],WM_SETFONT,(LPARAM)hMainFont,0);
		hFilterStatic[i]=CreateWindow("STATIC",cFilterSt[i],WS_CHILD | WS_VISIBLE,0,0,0,0,hwndMain,NULL,hInstance,NULL);
		SendMessage(hFilterStatic[i],WM_SETFONT,(LPARAM)hMainFont,0);
	}
	hFilterEdit[3]=CreateWindowEx(WS_EX_CLIENTEDGE,"COMBOBOX","",CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_VISIBLE | WS_VSCROLL,0,0,0,0,hwndMain,NULL,hInstance,NULL);
	SetWindowLong(hFilterEdit[3],GWL_ID,IDC_FILTEREDIT+3);
	SendMessage(hFilterEdit[3],WM_SETFONT,(LPARAM)hMainFont,0);
	hFilterStatic[3]=CreateWindow("STATIC","Группа",WS_CHILD | WS_VISIBLE,0,0,0,0,hwndMain,NULL,hInstance,NULL);
	SendMessage(hFilterStatic[3],WM_SETFONT,(LPARAM)hMainFont,0);

	hStatusBar=CreateStatusWindow(SBARS_SIZEGRIP | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE,NULL,hwndMain,IDC_STATUSBAR);
	int wid[2];
	wid[0]=250;
	wid[1]=-1;
	SendMessage(hStatusBar,SB_SETPARTS,2,(LPARAM)&wid);
	hBit1=LoadBitmap(hInstance,"BDAY");
	hBit2=LoadBitmap(hInstance,"IMEN");

	bHumanEditing=FALSE;
	LoadColumns();
	LoadUnits();
	UpdateToolBar();
	cList=new CSortListView(hwndMain,0,0,0,0,6,14);
	cList->AddImage(hBit1);
	cList->AddImage(hBit2);
	cList->SetCompareFunc(&MainCompareFunc);
	bHumanMode=TRUE;
	UpdateAllData();
	ShowWindow(hwndMain, Pos[4] ? SW_SHOWMAXIMIZED : SW_SHOW);
    UpdateWindow(hwndMain);	
	if (lstrlen(lpstrCmdLine)>0)
	{
		DWORD dw=atoi(lpstrCmdLine);
		nNote=FirstUnit;
		while (nNote)
		{
			if (nNote->Key==dw) break;
			nNote=nNote->Next;
		}
		if (nNote)
		{
			SetFocus(cList->GetWnd());
			SetItemSelect((long)nNote);
		}
	}
	else SetItemSelect(0);
	accel=LoadAccelerators(hInstance,"ACCEL");

    while (GetMessage((LPMSG) &msgMain, NULL, 0, 0))
    {
		if (!TranslateAccelerator(hwndMain,accel,(LPMSG)&msgMain))
		{
			TranslateMessage((LPMSG) &msgMain);
			DispatchMessage((LPMSG) &msgMain);
		}
    }
	DeleteObject(hWhitePen);
	DeleteObject(hGrayPen);
	DeleteObject(hBit1);
	DeleteObject(hBit2);
	DeleteObject(hMainFont);
	WriteBin((LPBYTE)Pos,10);
	SaveUnits();
	SaveColumns();
	DeleteUnits();
	UsePassword();
	delete bar;
	delete cList;
    return(0);
}

LONG FAR PASCAL FormWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int TimerId=0;
	static LPARAM lpr;
    switch(msg)
	{
	case WM_COMMAND:
		if (HIWORD(wParam)==EN_CHANGE)
		{
			SendMessage((HWND)lParam,WM_GETTEXT,29,(LPARAM)Filters[bHumanMode][LOWORD(wParam)-IDC_FILTEREDIT]);
			if (TimerId) KillTimer(hwnd,TimerId);
			TimerId=SetTimer(hwnd,2,1000,NULL);
		}
		if (HIWORD(wParam)==CBN_SELCHANGE && LOWORD(wParam)==IDC_FILTEREDIT+3)
			OnFilterComboSelect();
		if (HIWORD(wParam)==BN_CLICKED || HIWORD(wParam)==1)
		switch (LOWORD(wParam))
		{
		case IDC_ABOUT:
			ShowAbout(hwnd,"Записная книжка.",hInstance);
			break;
		case IDHELP:
			char* buf;
			buf=(char*)malloc(230);
			lstrcpy(buf,MainDir);
			lstrcat(buf,"Orgpanel.hlp");
			WinHelp(hwnd,buf,HELP_CONTEXT,1);
			free (buf);
			break;
		case IDC_CHANGE:
			ChangeNote();
			break;
		case IDC_FIRMDATA:
			SetMode(FALSE);
			break;
		case IDC_HUMANDATA:
			SetMode(TRUE);
			break;
		case IDC_MODE:
			SetMode(bHumanMode==false);
			break;
		case IDC_HEADOPTION:
			ShowColOption();
			break;
		case IDC_ADDHUMAN:
			AddHuman();
			break;
		case IDC_ADDFIRM:
			AddFirm();
			break;
		case IDC_ADDNOTE:
			if (bHumanMode) AddHuman();
			else AddFirm();
			break;
		case IDC_HOMERING:
			Ring(1);
			break;
		case IDC_WORKRING:
			Ring(2);
			break;
		case IDC_SENDEMAIL:
			SendEMail(hwnd);
			break;
		case IDC_HOMEPAGE:
			GotoHomePage(hwnd);
			break;
		case IDC_DELETE:
			DeleteNote();
			break;
		case IDC_PRINT:
			Print();
			break;
		case IDC_PRINTDLG:
			ShowPrintDlg();
			break;
		case IDC_BIO:
			ShowBioritm(hwnd);
			break;
		case IDC_EXIT:
			PostQuitMessage(0);
			break;
		case IDC_EXPORT:
			ExportToText();
			break;
		case IDC_SEARCH:
			lpr=Search(hwnd);
			SetTimer(hwnd,1,50,NULL);
			break;
		}
		break;
	case WM_TIMER:
		if (wParam==2)
		{
			KillTimer(hwnd,2);
			TimerId=0;
			UpdateAllData();
			MySound();
		}
		if (wParam==1)
		{
			SetFocus(cList->GetWnd());
			KillTimer(hwnd,1);
			SetItemSelect(lpr);
		}
		break;
	case WM_MENULOADED:
		EnableActionMenu((HMENU)wParam);
		SetMenuText((HMENU)wParam);
		break;
	case WM_COLUMNORDERCHANGE:
		OnChangeOrder(wParam,lParam);
		break;
	case WM_COLUMNWIDTHCHANGE:
		OnChangeHeadWidth(wParam,lParam);
		break;
	case WM_COLUMNOPTION:
		ShowColOption();
		break;
	case WM_COLUMNHIDE:
		OnHideColumn(lParam);
		UpdateAllData();
		break;
	case WM_GETMINMAXINFO:
		LPMINMAXINFO	lpmi;
		lpmi=(LPMINMAXINFO)lParam;
		lpmi->ptMinTrackSize.x=460;
		lpmi->ptMinTrackSize.y=280;
		break;
	case WM_SIZE:
		bar->OnSize(LOWORD(lParam),HIWORD(lParam));
		OnSize(LOWORD(lParam),HIWORD(lParam));
		break;
/*	case WM_DRAWITEM:
		if (wParam==6)
		{
		}
		break;*/
	case WM_NOTIFY:
		if (wParam==6)
		{
			NM_LISTVIEW* lpnm;
			lpnm=(NM_LISTVIEW*)lParam;
			if (lpnm->hdr.code==LVN_ITEMCHANGED && lpnm->uChanged&LVIF_STATE) 
				SetActiveUnit(lpnm);
			if (lpnm->hdr.code==NM_DBLCLK) ChangeNote();
		}
		break;
	case WM_EXITSIZEMOVE:
		RECT rc;
		GetWindowRect(hwnd,&rc);
		Pos[0]=rc.left>0 ? rc.left : 0;
		Pos[1]=rc.top>0 ? rc.top :0;
		Pos[2]=(WORD)(rc.right-rc.left);
		Pos[3]=(WORD)(rc.bottom-rc.top);
		break;
	case WM_SYSCOMMAND:
		if (wParam==SC_MAXIMIZE) Pos[4]=1;
		if (wParam==SC_RESTORE) Pos[4]=0;
		break;
	case WM_FILLBIOCOMBO:
		FillBioCombo((HWND)wParam);
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
		break;
	case WM_INITEXCHANGE:
		OnInitExchange();
		break;
	case WM_OPENHUMAN:
		if (!bHumanMode || bHumanEditing) break;
		nNote=FirstUnit;
		while (nNote)
		{
			if (nNote->Key==wParam) break;
			nNote=nNote->Next;
		}
		if (!nNote) break;
		SetForegroundWindow(hwnd);
		SetFocus(cList->GetWnd());
		SetItemSelect((long)nNote);
		break;
	}
    return(DefWindowProc(hwnd, msg, wParam, lParam));
}


