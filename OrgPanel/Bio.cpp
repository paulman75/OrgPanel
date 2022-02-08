#include <windows.h>
#include "math.h"
#define _MYDLL_
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "res/resource.h"
#include <commctrl.h>
#include "RegUnit.h"

extern HINSTANCE	hInstance;		  // Global instance handle for application
extern HFONT		hMainFont;
TDate BDate;
TDate CalDate;
HPEN	hpnBlue,hpnRed,hpnGreen,hpnBlue2;
HBRUSH hBackBrush;
const int xw=120;
HWND hwndToolTip;
int	 iTips;
extern BOOL	bShowFullName;

typedef struct
{
	float	FazaFiz,FazaEm,FazaIn;
} TFazaInfo, *PFazaInfo;

TFazaInfo	Faza;
DWORD		KolDays;

WNDPROC	hOldProc;
HICON hLeftIcon,hLeftIcon2,hRightIcon,hRightIcon2;

void CalculateBioritms()
{
	if (!BDate.Month) return;
	int i=EncodeDate(&CalDate)-EncodeDate(&BDate);
	if (i<1) i=0;
	KolDays=i;
	Faza.FazaFiz=mod(KolDays,23);
	Faza.FazaEm=mod(KolDays,28);
	Faza.FazaIn=mod(KolDays,33);
}

void SetDlgText(HWND hWnd)
{
	char buf[50];
	wsprintf((LPTSTR)&buf,"Прожито дней: %d",KolDays);
	SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)&buf);	
}

void DrawSin(HDC dc,int xc,int yc, float sx, float sy, HPEN hPen,float Faz,byte Period)
{
	HPEN hOldPen=(HPEN)SelectObject(dc,hPen);
	int yp=0;
	int i,y;
	float t;
	for (i=0; i<21; i++)
	{
        t=(float)i-10;
        y=(int)(40*sin(6.283*(t+Faz)/Period+3.1416));
        if (i!=0)
		{
              MoveToEx(dc,xc+(int)(sx*((i-1)*12-xw)),yc+(int)(sy*yp),NULL);
              LineTo(dc,xc+(int)(sx*(i*12-xw)),yc+(int)(sy*y));
        }
        yp=y;
	}
	SelectObject(dc,hOldPen);
}

void OnPaint(HWND hwnd, HDC dc)
{
	RECT rc;
	SelectObject(dc,hMainFont);
	SIZE siz;
	GetTextExtentPoint32(dc,"1",1,&siz);
	GetClientRect(hwnd,&rc);
	rc.left+=2;
	rc.right-=2;
	rc.bottom-=2;
	rc.top+=siz.cy/2+1;
	FillRect(dc,&rc,hBackBrush);
	HPEN hOldPen=(HPEN)SelectObject(dc,hpnBlue);

	float sx=(float)(rc.right-rc.left)/250;
	float sy=(float)(rc.bottom-rc.top)/100;

	int xc=(rc.right+rc.left)/2;

	char ch2[]="Интеллектуальный";
	SetTextColor(dc,0xc06810);
	GetTextExtentPoint32(dc,ch2,lstrlen(ch2),&siz);
	TextOut(dc,xc-siz.cx/2,rc.bottom-siz.cy-1,ch2,lstrlen(ch2));

	char ch[]="Физический";
	SetTextColor(dc,0x8000);
	SIZE siz2;
	GetTextExtentPoint32(dc,ch,lstrlen(ch),&siz2);
	TextOut(dc,(xc-siz2.cx)/2-siz.cx/4,rc.bottom-siz.cy-1,ch,lstrlen(ch));


	SetTextColor(dc,0xd0);
	char ch3[]="Эмоциональный";
	GetTextExtentPoint32(dc,ch3,lstrlen(ch3),&siz2);
	TextOut(dc,(rc.right+xc+siz.cx/2)/2-siz2.cx/2,rc.bottom-siz.cy-1,ch3,lstrlen(ch3));

	int yc=(rc.top+rc.bottom-siz.cy)/2;

	MoveToEx(dc,xc,yc+(int)(40*sy),NULL);
	LineTo(dc,xc,yc-(int)(40*sy));
	MoveToEx(dc,xc+(int)(xw*sx),yc,NULL);
	LineTo(dc,xc-(int)(xw*sx),yc);
	TDate	d=CalDate;
	DateBefore(&d,10);
	SetTextColor(dc,0);
	SetBkMode(dc,TRANSPARENT);
	byte i;
	for (i=0; i<21; i++)
	{
        MoveToEx(dc,xc+(int)(sx*(i*12-xw)),yc,NULL);
        LineTo(dc,xc+(int)(sx*(i*12-xw)),yc+(int)(5*sy));
        if ((i&1)==0)
		{
			char buf2[10];
			wsprintf((LPTSTR)&buf2,"%d",d.Day);
			GetTextExtentPoint32(dc,buf2,lstrlen(buf2),&siz);
            TextOut(dc,xc+(int)(sx*(i*12-xw))-siz.cx/2,yc+(int)(6*sy),buf2,lstrlen(buf2));
        }
        DateForward(&d,1);
	}
	GetTextExtentPoint32(dc,"+",1,&siz);
	TextOut(dc,xc-siz.cx/2,yc-(int)(sy*48),"+",1);
	GetTextExtentPoint32(dc,"-",1,&siz);
	TextOut(dc,xc-siz.cx/2,yc+(int)(sy*39),"-",1);
	if (BDate.Month)
	{
		DrawSin(dc,xc,yc,sx,sy,hpnGreen,Faza.FazaFiz,23);
		DrawSin(dc,xc,yc,sx,sy,hpnRed,Faza.FazaEm,28);
		DrawSin(dc,xc,yc,sx,sy,hpnBlue2,Faza.FazaIn,33);
	}
	SelectObject(dc,hOldPen);
}

void UpdateBDay(HWND hWnd)
{
	DeleteAllDateLabelsOnWindow(hWnd);
	int iIndex=SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);
	if (iIndex==-1) return;
	PDate pd=(PDate)SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETITEMDATA,iIndex,0);
	HWND hStatic=GetDlgItem(hWnd,IDC_DAYSTATIC);
	RECT rc;
	POINT pt={0,0};
	GetWindowRect(hStatic,&rc);
	ClientToScreen(hWnd,&pt);
	if (!pd)
	{
		CreateDateLabel(hWnd, (rc.left+rc.right)/2-pt.x, (rc.top+rc.bottom)/2-pt.y, &BDate, 1, FALSE);
		SetWindowText(hStatic,NULL);
	}
	else
	{
		BDate=*pd;
		char* temp=(char*)malloc(100);
		DateToString(&BDate,temp);
		SetWindowText(hStatic,temp);
		free(temp);
	}
	RECT rc2;
	GetWindowRect(GetDlgItem(hWnd,IDC_STATICCAL),&rc2);
	CreateDateLabel(hWnd, (rc2.left+rc2.right)/2-pt.x, (rc.top+rc.bottom)/2-pt.y, &CalDate, 2, FALSE);
	CalculateBioritms();
	SetDlgText(hWnd);
	InvalidateRect(GetDlgItem(hWnd,IDC_BIOGROUP),NULL,FALSE);
}

void ChangeCalcDate(int d, HWND hWnd)
{
	MySound();
	if (d>0) DateForward(&CalDate,d);
	else DateBefore(&CalDate,-d);
	SetDate(&CalDate,2);
	CalculateBioritms();
	SetDlgText(hWnd);
	InvalidateRect(GetDlgItem(hWnd,IDC_BIOGROUP),NULL,FALSE);
}

long CALLBACK BioGroupProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg==WM_PAINT)
	{
		HDC dc=GetDC(hwnd);
		OnPaint(hwnd,dc);
		ReleaseDC(hwnd,dc);
	}
	return CallWindowProc(hOldProc, hwnd,msg, wParam, lParam);
}

void SetToolTip(HWND hWnd, LPTSTR s)
{
	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize=sizeof(ti);
	ti.uFlags = TTF_SUBCLASS ;  //TTF_SUBCLASS causes the tooltip to automatically subclass the window and look for the messages it is interested in.
	ti.hwnd = hWnd;
	ti.uId = 0;
	ti.lpszText=s;
	GetClientRect(hWnd,&ti.rect);
	SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void OnComboInit(HWND hDlg)
{
	HWND hCombo=GetDlgItem(hDlg,IDC_COMBO1);
	int i=SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)"__Незнакомец__");
	SendMessage(hCombo,CB_SETITEMDATA,i,0);
	char* name=(char*)malloc(50);
	char* sname=(char*)malloc(50);
	name[0]=0;
	sname[0]=0;
	ReadRegistry("FName",(LPBYTE)name,50);
	ReadRegistry("SName",(LPBYTE)sname,50);
	if (!name[0])
	{
		free(name);
		name=NULL;
	}
	if (!sname[0])
	{
		free(sname);
		sname=NULL;
	}
	char* temp=(char*)malloc(100);
	MakeShortName(name,NULL,sname,temp);
	static TDate MyDate;
	ReadRegistry("BDate",(LPBYTE)&MyDate,sizeof(MyDate));
	if (!name) name=(char*)malloc(100);
	lstrcpy(name,"** ");
	lstrcat(name,temp);
	lstrcat(name," **");
	i=SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)name);
	SendMessage(hCombo,CB_SETITEMDATA,i,(LPARAM)&MyDate);
	free(temp);
	free(name);
	if (sname) free(sname);
}

BOOL FAR PASCAL BioDialog(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:		  // message: initialize dialog box
		PutOnCenter(hDlg);
		hwndToolTip=CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_CHILD | WS_POPUP | TTS_ALWAYSTIP, 
            CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, 
             hDlg, NULL, hInstance, NULL);
		SendMessage(hwndToolTip,TTM_SETDELAYTIME, TTDT_INITIAL,100);
		iTips=0;
		hBackBrush  = CreateSolidBrush(0xffffff);
		hpnGreen	= CreatePen(PS_SOLID,1,0x8000);
		hpnRed		= CreatePen(PS_SOLID,1,0xdc);
		hpnBlue2	= CreatePen(PS_SOLID,1,0xc06810);
		hpnBlue		= CreatePen(PS_SOLID,1,0xff0000);
		hLeftIcon=LoadIcon(hInstance,"LEFT");
		hLeftIcon2=LoadIcon(hInstance,"LEFT2");
		hRightIcon=LoadIcon(hInstance,"RIGHT");
		hRightIcon2=LoadIcon(hInstance,"RIGHT2");
		SendMessage(GetDlgItem(hDlg,IDC_BUTLEF1),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hLeftIcon);
		SendMessage(GetDlgItem(hDlg,IDC_BUTLEF2),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hLeftIcon2);
		SendMessage(GetDlgItem(hDlg,IDC_BUTRIG1),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hRightIcon);
		SendMessage(GetDlgItem(hDlg,IDC_BUTRIG2),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hRightIcon2);
		SetToolTip(GetDlgItem(hDlg,IDC_BUTLEF1),"Назад на день");
		SetToolTip(GetDlgItem(hDlg,IDC_BUTLEF2),"Назад на неделю");
		SetToolTip(GetDlgItem(hDlg,IDC_BUTRIG1),"Вперед на день");
		SetToolTip(GetDlgItem(hDlg,IDC_BUTRIG2),"Вперед на неделю");
		SetToolTip(GetDlgItem(hDlg,IDOK),"Выход");
		OnComboInit(hDlg);
		SendMessage(GetParent(hDlg),WM_FILLBIOCOMBO,(WPARAM)GetDlgItem(hDlg,IDC_COMBO1),0);
		UpdateBDay(hDlg);
		SetDlgText(hDlg);
		hOldProc=(WNDPROC)SetWindowLong(GetDlgItem(hDlg,IDC_BIOGROUP),GWL_WNDPROC,(long)&BioGroupProc);
		return (TRUE);
	case WM_COMMAND:
		if (HIWORD(wParam)==BN_CLICKED)
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			DeleteObject(hBackBrush);
			DeleteObject(hpnBlue);
			DeleteObject(hpnBlue2);
			DeleteObject(hpnGreen);
			DeleteObject(hpnRed);
			DeleteObject(hLeftIcon);
			DeleteObject(hLeftIcon2);
			DeleteObject(hRightIcon);
			DeleteObject(hRightIcon2);
			DestroyWindow(hwndToolTip);
			DeleteAllDateLabelsOnWindow(hDlg);
			EndDialog(hDlg, TRUE);	  // Exits the dialog box
			return TRUE;
		case IDC_BUTRIG1:
			ChangeCalcDate(1,hDlg);
			break;
		case IDC_BUTRIG2:
			ChangeCalcDate(7,hDlg);
			break;
		case IDC_BUTLEF1:
			ChangeCalcDate(-1,hDlg);
			break;
		case IDC_BUTLEF2:
			ChangeCalcDate(-7,hDlg);
			break;
	    }
		if (HIWORD(wParam)==CBN_SELCHANGE)
		{
			UpdateBDay(hDlg);
			return FALSE;
		}
	    break;
	case WM_CALENDARDATECHANGE:
		if (wParam==1) GetDate(&BDate,1);
		else GetDate(&CalDate,2);
		CalculateBioritms();
		SetDlgText(hDlg);
		InvalidateRect(GetDlgItem(hDlg,IDC_BIOGROUP),NULL,FALSE);
		break;
    }
    return FALSE;			  // Didn't process a message
}

void __stdcall ShowBioritm(HWND hWnd)
{
	BDate.Month=0;
	CalDate=Now();
	CalculateBioritms();
	DialogBox(hInstance,"BIO",hWnd,	(DLGPROC)BioDialog);			  // About() instance address
}

void __stdcall MakeShortName(LPCTSTR Name, LPCTSTR MName, LPCTSTR SName, LPTSTR s)
{
	s[0]=0;
	if (bShowFullName)
	{
		if (SName)
		{
			lstrcpy(s,SName);
			lstrcat(s," ");
		}
		if (Name)
		{
			lstrcat(s,Name);
			lstrcat(s," ");
		}
		if (MName)
		{
			lstrcat(s,MName);
			lstrcat(s," ");
		}
		return;
	}
	if (SName)
	{	
		lstrcpy(s,SName);
		int l=lstrlen(s);
		s[l++]=32;
		if (Name)
		{
			s[l++]=Name[0];
			s[l++]=0x2e;
			if (MName)
			{
				s[l++]=MName[0];
				s[l++]=0x2e;
			}
		}
		s[l]=0;
	}
	else
	{
		if (Name) lstrcpy(s,Name);
	}
}
