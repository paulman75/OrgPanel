#include <windows.h>
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "../Headers/DynList.h"
#include "Pages.h"
#include "Dela.h"
#include "DelaList.h"
#include <commctrl.h>

extern HWND hwndMain;
extern HINSTANCE hInstance;		  // Global instance handle for application
extern DynArray<TDelo> DeloList;
HWND	hListView;
HIMAGELIST	hImageList;

void UpdateToolBar();

int CALLBACK DelaListCompareFunc(LPARAM lParam1, LPARAM lParam2,  LPARAM lParamSort)
{
	TDelo dl1;
	TDelo dl2;
	GetDeloByID(lParam1,&dl1);
	GetDeloByID(lParam2,&dl2);
	return dl1.begDate>dl2.begDate;
}

void InitDelaList()
{
	hListView=CreateWindowEx(WS_EX_CLIENTEDGE,"SYSLISTVIEW32","", LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP | WS_CHILD | LVS_SHOWSELALWAYS,0,30,200,200,hwndMain,NULL,hInstance,NULL);

	SetWindowLong(hListView, GWL_ID, LV_ID);
	SendMessage(hListView,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
	LV_COLUMN lvc;
	lvc.mask=LVCF_WIDTH | LVCF_TEXT;
	lvc.pszText="Название";
	lvc.cx=135;
	SendMessage(hListView,LVM_INSERTCOLUMN,1,(LPARAM)&lvc);
	lvc.pszText="Начало";
	lvc.cx=100;
	SendMessage(hListView,LVM_INSERTCOLUMN,2,(LPARAM)&lvc);
	lvc.pszText="Окончание";
	lvc.cx=100;
	SendMessage(hListView,LVM_INSERTCOLUMN,3,(LPARAM)&lvc);
	lvc.pszText="Фактическая";
	lvc.cx=100;
	SendMessage(hListView,LVM_INSERTCOLUMN,4,(LPARAM)&lvc);

	hImageList=ImageList_Create(10,10,ILC_COLOR4,0,0);
	SendMessage(hListView,LVM_SETIMAGELIST,LVSIL_SMALL,(LPARAM)hImageList);
	HBITMAP hBit=LoadBitmap(hInstance,"ENDNO");
	ImageList_Add(hImageList,hBit,NULL);
	hBit=LoadBitmap(hInstance,"ENDOK");
	ImageList_Add(hImageList,hBit,NULL);
	hBit=LoadBitmap(hInstance,"FACTYES");
	ImageList_Add(hImageList,hBit,NULL);
	hBit=LoadBitmap(hInstance,"FACTNO");
	ImageList_Add(hImageList,hBit,NULL);

	TDate d=Now();
	DateBefore(&d,30);
	CreateDateLabel(hwndMain,130,53,&d,ID_STDATE,FALSE);
	DateForward(&d,60);
	CreateDateLabel(hwndMain,250,53,&d,ID_FINDATE,FALSE);
	ShowDateLabel(ID_STDATE,FALSE);
	ShowDateLabel(ID_FINDATE,FALSE);
}

int AddItem(LPTSTR lpstrText, int iItem, int iSubItem, LPARAM lParam, int iImage)
{
	LV_ITEM lvi;
	lvi.pszText=lpstrText;
	lvi.lParam=lParam;
	lvi.iItem=iItem;
	lvi.iSubItem=iSubItem;
	int res=iItem;
	if (iSubItem)
	{
		lvi.mask=LVIF_TEXT;
		SendMessage(hListView,LVM_SETITEM,0,(LPARAM)&lvi);
	}
	else
	{
		lvi.mask=LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
		lvi.iImage=iImage;
		res=SendMessage(hListView,LVM_INSERTITEM,0,(LPARAM)&lvi);
	}
	return res;
}

void UpdateDelaList()
{
	TDate d1;
	TDate d2;
	GetDate(&d1,ID_STDATE);
	GetDate(&d2,ID_FINDATE);
	char* temp=(char*)malloc(100);
	SendMessage(hListView,LVM_DELETEALLITEMS,0,0);
	int Count=0;
	for (int i=0; i<DeloList.count(); i++)
	{
		TDelo dl=DeloList[i];
		int iImage=0;
		if (dl.begDate>d2) continue;
		if (dl.factDate.Month)
		{
			if (d1>dl.endDate && d1>dl.factDate) continue;
			iImage=1; //законченное
			if (dl.endDate<dl.factDate) iImage=2; //просроченное законченное
		}
		else
		{
			if (dl.endDate<Now()) iImage=3; //просроченное
		}
		AddItem(dl.Text,Count,0,dl.ID,iImage);
		DateToString(&dl.begDate,temp);
		AddItem(temp,Count,1,0,0);
		DateToString(&dl.endDate,temp);
		AddItem(temp,Count,2,0,0);
		if (dl.factDate.Month)
		{
			DateToString(&dl.factDate,temp);
			AddItem(temp,Count,3,0,0);
		}
		Count++;
	}
	free(temp);
	SendMessage(hListView,LVM_SORTITEMS,0,(LPARAM)DelaListCompareFunc);
}

void UpdateDelaPage()
{
	ShowWindow(hListView,SW_SHOW);
	ShowDateLabel(ID_STDATE,TRUE);
	ShowDateLabel(ID_FINDATE,TRUE);

	InvalidateRect(hwndMain,NULL,TRUE);
	UpdateDelaList();
	UpdateToolBar();
}

void CloseDelaPage()
{
	ShowWindow(hListView,SW_HIDE);
	ShowDateLabel(ID_STDATE,FALSE);
	ShowDateLabel(ID_FINDATE,FALSE);

	InvalidateRect(hwndMain,NULL,TRUE);
	UpdateToolBar();
}
