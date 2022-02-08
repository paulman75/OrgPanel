#include "windows.h"
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "Print.h"
#include "classes.h"
#include "Pages.h"
#include "../Headers/IconList.h"
#include "res/resource.h"
#include "../Headers/Bitmap.h"

PRINTDLG pd;
extern HINSTANCE hInstance;		  // Global instance handle for application
extern HWND hwndMain;
extern HBRUSH	hWhiteBrush;

TPage TempPage;

const int xp=200;
extern TPage		LeftPage,RightPage;
int iRight;
BOOL bToText;
TDate dTo,dFrom;
CIconListBox* cList;
char prOrder[DNEV_COUNT];
extern char	Order[DNEV_COUNT];
const char* DnevCap[]={"Дела","Дни рождения","Именины","Праздники","Приметы","События","Православие"};
extern CMyBitmap*	Image[DNEV_COUNT];

void InitPrinter()
{
	pd.lStructSize=sizeof(pd);
	pd.hInstance=hInstance;
	pd.hDevMode=NULL;
	pd.hDevNames=NULL;
	pd.Flags=PD_PRINTSETUP;
	pd.hwndOwner=hwndMain;
}

void ShowPrintDlg()
{
	PrintDlg(&pd);
}

char*	GetItem(char* s)
{
	while (*s!=44 && *s) s++;
	*s=0;
	return ++s;
}

void MyTextOut(HDC dc,LPTSTR s,int* y)
{
	RECT rc;
	rc.left=xp;
	rc.right=iRight;
	rc.top=*y;
	rc.bottom=*y+800000;
	int hei=DrawText(dc,s,lstrlen(s),&rc,DT_WORDBREAK);
	*y+=hei+35;
}

void ToText(CMyString* s)
{
	TempPage.Date=dFrom;
	TempPage.CountSt=0;
	LPCTSTR cr="\r\n";
	LPTSTR temp=(LPTSTR)malloc(500);
	while (TempPage.Date<=dTo)
	{
		UpdatePage(&TempPage, TRUE);
		int i=0;
		BOOL bDate=FALSE;
		while (i<TempPage.CountSt)
		{
			if (prOrder[TempPage.st[i].IconType]>99)
			{
				if (!bDate)
				{
					DateToString(&TempPage.Date,temp);
					lstrcat(temp,cr);
					bDate=TRUE;
					*s+=temp;
				}
				int Ind=TempPage.st[i].Index;
				if (TempPage.st[i].IconType==ID_IMEN) *s+="Именины: ";
				while (Ind==TempPage.st[i].Index && i<TempPage.CountSt)
				{
					*s+=TempPage.st[i].Text;
					i++;
				}
				*s+=cr;
			}
			else i++;
		}
		TempPage.Date++;
		if (bDate) *s+=cr;
	}
	free(temp);
}

void ExportToText(HWND hDlg)
{
	CMyString* s=new CMyString("");
	ToText(s);
	SendMessage(GetDlgItem(hDlg,IDC_EDIT1),WM_SETTEXT,0,(LPARAM)s->Text);
	delete s;
}

BOOL FAR PASCAL MemoDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		ExportToText(hDlg);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam)==IDOK || LOWORD(wParam==IDCANCEL)) 
		{
			EndDialog(hDlg, TRUE);
			return (TRUE);
	    }
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam==GetDlgItem(hDlg,IDC_EDIT1))
		{
			return (int)hWhiteBrush;
		}
		break;

	}
	return FALSE;
}

BOOL FAR PASCAL PrintDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	int n,k;
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		GetClientRect(hDlg,&rc);
		CreateDateLabel(hDlg,(int)(rc.right*0.28),(int)(rc.bottom*0.715),&dFrom,1,FALSE);
		CreateDateLabel(hDlg,(int)(rc.right*0.73),(int)(rc.bottom*0.715),&dTo,2,FALSE);
		cList=new CIconListBox(GetDlgItem(hDlg,IDC_LIST1),TRUE,20);
		for (k=0; k<DNEV_COUNT; k++)
		{
			n=prOrder[k];
			if (n>99) n-=100;
			if (n!=DNEV_COUNT) cList->AddItem(DnevCap[n],prOrder[k]>99,Image[n]->hdc);
		}
		return TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam)==IDCANCEL || LOWORD(wParam)==IDOK) 
		{
			DeleteAllDateLabelsOnWindow(hDlg);
			delete cList;
			bToText=SendMessage(GetDlgItem(hDlg,IDC_CHECK1),BM_GETSTATE,0,0)&1;
			EndDialog(hDlg, LOWORD(wParam)==IDOK);
			return (TRUE);
	    }
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		switch (wParam)
		{
		case IDC_LIST1:
			if (cList) return cList->OnChildNotify(message,wParam,lParam);
			break;
		}
		break;
	case WM_LISTBOXCHECKCHANGE:
		n=LOWORD(wParam);
		if (prOrder[n]>99) prOrder[n]-=100;
		if (lParam) prOrder[n]+=100;
		break;
	case WM_CALENDARDATECHANGE:
		GetDate(&dTo,2);
		GetDate(&dFrom,1);
		break;
    }
    return (FALSE);
}

void GoPrint()
{
	CMyString* s=new CMyString("");
	ToText(s);
	HDC dc;
	if (pd.hDevNames==NULL)
	{
		char* Info=(char*)malloc(100);
		GetProfileString("windows", "device", ",,", Info, 100);
		char* Driver=Info;
		char* PrType=GetItem(Info);
		char* Port=GetItem(PrType);
		GetItem(Port);

		dc=CreateDC(PrType, Driver, Port, NULL);
		free(Info);
	}
	else
	{
		LPDEVNAMES dm=(LPDEVNAMES)(*(LPDWORD)pd.hDevNames);
		dc=CreateDC((char*)((DWORD)dm+dm->wDriverOffset), (char*)((DWORD)dm+dm->wDeviceOffset), (char*)((DWORD)dm+dm->wOutputOffset), NULL);
	}
	int i=GetDeviceCaps(dc,HORZRES);
	int j=GetDeviceCaps(dc,HORZSIZE);
	iRight=j/200*i-200;
	if (Escape(dc, STARTDOC, 15, "Дневник", NULL)<0)
	{
		MessageBox(hwndMain, "Printing can not be started", NULL,MB_OK | MB_ICONSTOP);
		return;
	}
	if (Escape(dc, NEWFRAME, 0, NULL, NULL)<0)
	{
		MessageBox(hwndMain, "Paper can not be advanced", NULL, MB_OK | MB_ICONSTOP);
		DeleteDC(dc);
		return;
	}
	int y=(int)(GetDeviceCaps(dc,LOGPIXELSY)*0.9);
	int maxy=(int)(GetDeviceCaps(dc,VERTRES)*0.35);

	LPTSTR temp=(LPTSTR)malloc(1000);
	LPTSTR t=s->Text;

	while (*t)
	{
		int i=0;
		while (*t && *t!='\r') temp[i++]=*(t++);
		temp[i]=0;
		if (*t) t+=2;
		MyTextOut(dc,temp,&y);
		if (y>maxy)
		{
			Escape(dc, NEWFRAME, 0, NULL, NULL);
			y=(int)(GetDeviceCaps(dc,LOGPIXELSY)*0.9);
		}
	}
	free(temp);

	if (Escape(dc, NEWFRAME, 0, NULL, NULL)<0)
	{
		MessageBox(hwndMain, "Paper can not be advanced", NULL, MB_OK | MB_ICONSTOP);
		return;
	}
	if (Escape(dc, ENDDOC, 0, NULL, NULL)<0)
	{
		MessageBox(hwndMain, "Error Printing", NULL, MB_OK | MB_ICONSTOP);
	}
	DeleteDC(dc);
	delete s;
}

void Print()
{
	dFrom=LeftPage.Date;
	dTo=RightPage.Date;
	int pr=0;
	for (int k=0; k<DNEV_COUNT; k++)
	if (Order[k]>99) prOrder[pr++]=Order[k];
	for (int k=pr; k<DNEV_COUNT;k++) prOrder[pr]=DNEV_COUNT;
	if (!DialogBox(hInstance, "PRINTDIALOG", hwndMain, (DLGPROC)PrintDlgProc)) return;
	if (bToText) DialogBox(hInstance, "MEMODLG", hwndMain, (DLGPROC)MemoDlgProc);
	else GoPrint();
}
