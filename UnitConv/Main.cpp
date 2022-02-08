#include <windows.h>
#include "../OrgPanel/ToolBar.h"
#include <commctrl.h>
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "res/resource.h"
#include "Units.h"

const char Str1[]="Исходные данные";
const char Str2[]="Результат";
const char Str3[]="Единица измерения";
const char Str4[]="Значение";
const char Str5[]="Мировые константы!";
const char Str6[]="Физические константы";
const char Str7[]="Название";

//---------------------------------------------------------------------------
// Function declarations
//---------------------------------------------------------------------------
HWND hMainDlg;
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow);
//---------------------------------------------------------------------------
// Global Variables...
//---------------------------------------------------------------------------
HINSTANCE	hInstance;		  // Global instance handle for application
CToolBar*	bar;
HWND		hWndToolTip=NULL;
HPEN		hGrayPen,hWhitePen;
HFONT		hMainFont;
HBITMAP		hBack;
int 		CurrentIndex;
BOOL		bFiz=FALSE;

void UpdateDlgCaption(LPCTSTR cap)
{
	char* buf=(char*)malloc(120);
	wsprintf(buf,"ОргПанель - Конвертер (%s)",cap);
	SendMessage(hMainDlg,WM_SETTEXT,0,(LPARAM)buf);
	free(buf);
	BOOL b=CurrentIndex==ID_FIZ;
	if (b!=bFiz)
	{
		bFiz=b;
		ShowWindow(GetDlgItem(hMainDlg,IDC_COMBO2), bFiz ? SW_HIDE : SW_SHOW);
		ShowWindow(GetDlgItem(hMainDlg,IDC_GROUP2), bFiz ? SW_HIDE : SW_SHOW);
		ShowWindow(GetDlgItem(hMainDlg,IDC_EDIT2), bFiz ? SW_HIDE : SW_SHOW);
		ShowWindow(GetDlgItem(hMainDlg,IDC_BUTFROMBUF), bFiz ? SW_HIDE : SW_SHOW);
		ShowWindow(GetDlgItem(hMainDlg,IDC_BUTTOBUF), bFiz ? SW_HIDE : SW_SHOW);
		InvalidateRect(hMainDlg,NULL,TRUE);
	}
}

void InitDlg()
{
	PutOnCenter(hMainDlg);
	InitCommonControls();
	hWndToolTip=CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_CHILD | WS_POPUP | TTS_ALWAYSTIP, 
            CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, 
            hMainDlg, NULL, hInstance, NULL);
	bar=new CToolBar(hMainDlg,0,0,29,TRUE,hWndToolTip);
	bar->AddButton("Длина",	 ID_LEN, "LEN",	TBSTYLE_BUTTON);
	UpdateDlgCaption(bar->GetButtonTips(ID_LEN));
	bar->AddButton("Площадь",	 ID_SQ, "SQ",	TBSTYLE_BUTTON);
	bar->AddButton("Объем",	 ID_3D, "3D",	TBSTYLE_BUTTON);
	bar->AddButton("Масса",	 ID_MASSA, "MASSA",	TBSTYLE_BUTTON);
	bar->AddButton("Скорость",	 ID_SPEED, "SPEED",	TBSTYLE_BUTTON);
	bar->AddButton("Меры жидкостей",	 ID_LIQ, "LIQ",	TBSTYLE_BUTTON);
	bar->AddButton("Меры сыпучих тел",	 ID_SYP, "SYP",	TBSTYLE_BUTTON);
	bar->AddButton("Температура",	 ID_TEMP, "TEMP",	TBSTYLE_BUTTON);
	bar->AddButton("Мощность",	 ID_POWER, "POWER",	TBSTYLE_BUTTON);
	bar->AddButton("Давление",	 ID_PRESS, "PRESS",	TBSTYLE_BUTTON);
	bar->AddButton("Системы счисления",	 ID_HEX, "HEX",	TBSTYLE_BUTTON);
	bar->AddButton("Физические константы",	 ID_FIZ, "FIZ",	TBSTYLE_BUTTON);
	bar->AddButton("О программе",	 ID_ABOUT, "ABOUT",	TBSTYLE_BUTTON);
	HICON hi=LoadIcon(hInstance,"MAINICON");
	SendMessage(hMainDlg,WM_SETICON,ICON_BIG,(LPARAM)hi);
	UpdateUnitName();
}

void OnPaint()
{
	PAINTSTRUCT pai;
	BeginPaint(hMainDlg,&pai);
	HDC dc=pai.hdc;
	RECT rc;
	GetWindowRect(hMainDlg,&rc);
	SIZE siz;
	SelectObject(dc,hMainFont);
	SetBkMode(dc,TRANSPARENT);
	int w=rc.right-rc.left;
	int m1=w/4;
	int m2=3*w/4;
	if (!bFiz)
	{
		GetTextExtentPoint32(dc,Str1,strlen(Str1),&siz);
		TextOut(dc,m1-siz.cx/2,41,Str1,strlen(Str1));
		GetTextExtentPoint32(dc,Str2,strlen(Str2),&siz);
		TextOut(dc,m2-siz.cx/2,41,Str2,strlen(Str2));
		GetTextExtentPoint32(dc,Str3,strlen(Str3),&siz);
		TextOut(dc,m1-siz.cx/2,80,Str3,strlen(Str3));
		TextOut(dc,m2-siz.cx/2,80,Str3,strlen(Str3));
	}
	GetTextExtentPoint32(dc,Str4,strlen(Str4),&siz);
	TextOut(dc,m1-siz.cx/2,140,Str4,strlen(Str4));
	if (!bFiz) TextOut(dc,m2-siz.cx/2,140,Str4,strlen(Str4));
	if (bFiz)
	{
		GetTextExtentPoint32(dc,Str6,strlen(Str6),&siz);
		TextOut(dc,m1-siz.cx/2,41,Str6,strlen(Str6));
		GetTextExtentPoint32(dc,Str7,strlen(Str7),&siz);
		TextOut(dc,m1-siz.cx/2,80,Str7,strlen(Str7));
		GetTextExtentPoint32(dc,Str5,strlen(Str5),&siz);
		TextOut(dc,m2-siz.cx/2,112,Str5,strlen(Str5));
	}
	EndPaint(hMainDlg,&pai);
}

void OnSize(WORD w, WORD h)
{
	if (h>230) h=230;
	MoveWindow(GetDlgItem(hMainDlg,IDC_GROUP1),10,50,w/2-20,h-60,TRUE);
	MoveWindow(GetDlgItem(hMainDlg,IDC_GROUP2),w/2+10,50,w/2-20,h-60,TRUE);
	RECT rc;
	GetWindowRect(GetDlgItem(hMainDlg,IDC_COMBO1),&rc);
	MoveWindow(GetDlgItem(hMainDlg,IDC_COMBO1),20,100,w/2-40,rc.bottom-rc.top,TRUE);
	MoveWindow(GetDlgItem(hMainDlg,IDC_COMBO2),w/2+20,100,w/2-40,rc.bottom-rc.top,TRUE);
	GetWindowRect(GetDlgItem(hMainDlg,IDC_EDIT1),&rc);
	MoveWindow(GetDlgItem(hMainDlg,IDC_EDIT1),25,160,w/2-50,rc.bottom-rc.top,TRUE);
	MoveWindow(GetDlgItem(hMainDlg,IDC_EDIT2),w/2+25,160,w/2-50,rc.bottom-rc.top,TRUE);
	GetWindowRect(GetDlgItem(hMainDlg,IDC_BUTTON1),&rc);
	MoveWindow(GetDlgItem(hMainDlg,IDC_BUTTON1),10+(w/2-20-rc.right+rc.left)/2,190,rc.right-rc.left,rc.bottom-rc.top,TRUE);
	MoveWindow(GetDlgItem(hMainDlg,IDC_BUTTON2),w/2+10+(w/2-20-rc.right+rc.left)/2,190,rc.right-rc.left,rc.bottom-rc.top,TRUE);
}

BOOL FAR PASCAL MainDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
    switch (message) {
	case WM_INITDIALOG:
		hMainDlg=hDlg;
		InitDlg();
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam)==EN_CHANGE && LOWORD(wParam)==IDC_EDIT1 && !bFiz) CalcResult();
		if (HIWORD(wParam)==CBN_SELCHANGE) CalcResult();
		if (HIWORD(wParam)==BN_CLICKED)
		switch (LOWORD(wParam))
		{
		case IDC_BUTTOBUF:
			SendMessage(GetDlgItem(hMainDlg,IDC_EDIT2),EM_SETSEL,0,-1);
			SendMessage(GetDlgItem(hMainDlg,IDC_EDIT2),WM_COPY,0,0);
			SendMessage(GetDlgItem(hMainDlg,IDC_EDIT2),EM_SETSEL,-1,0);
			break;
		case IDC_BUTFROMBUF:
			SendMessage(GetDlgItem(hMainDlg,IDC_EDIT1),EM_SETSEL,0,-1);
			SendMessage(GetDlgItem(hMainDlg,IDC_EDIT1),WM_PASTE,0,0);
			break;
		case IDCANCEL:
			EndDialog(hDlg,TRUE);
			return TRUE;
		case ID_ABOUT:
			ShowAbout(hDlg,"Конвертер.",hInstance);
			break;
		case ID_LEN:
		case ID_SQ:
		case ID_3D:
		case ID_MASSA:
		case ID_SPEED:
		case ID_TEMP:
		case ID_SYP:
		case ID_LIQ:
		case ID_POWER:
		case ID_PRESS:
		case ID_HEX:
		case ID_FIZ:
			CurrentIndex=LOWORD(wParam);
			UpdateDlgCaption(bar->GetButtonTips(LOWORD(wParam)));
			UpdateUnitName();
			if (!bFiz)
			{
				SendMessage(GetDlgItem(hMainDlg,IDC_EDIT1),EM_SETSEL,0,-1);
				SendMessage(GetDlgItem(hMainDlg,IDC_EDIT1),WM_CLEAR,0,0);
			}
			break;
		}
		break;
	case WM_GETMINMAXINFO:
		LPMINMAXINFO	lpmi;
		lpmi=(LPMINMAXINFO)lParam;
		lpmi->ptMinTrackSize.x=490;
		lpmi->ptMinTrackSize.y=260;
		break;
	case WM_PAINT:
		OnPaint();
		break;
	case WM_ERASEBKGND:
		GetClientRect(hDlg,&rc);
		FillBackGround((HDC)wParam,&rc,hBack);
		return TRUE;
	case WM_SIZE:
		OnSize(LOWORD(lParam),HIWORD(lParam));
		break;
	}
	return FALSE;
}

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow)
{
	hInstance=hInst;
	hBack=LoadBitmap(hInstance,"BACK");
	hWhitePen = CreatePen(PS_SOLID,1,0x0ffffff);
	hGrayPen = CreatePen(PS_SOLID,1,0x808080);
	hMainFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);//=(HFONT)SendMessage(hwndMain,WM_GETFONT,0,0);
	CurrentIndex=ID_LEN;
	DialogBox(hInstance,"MAINDLG",0,MainDlgProc);

	delete bar;
	DeleteObject(hWhitePen);
	DeleteObject(hGrayPen);
	DeleteObject(hBack);
    return(0);
}
