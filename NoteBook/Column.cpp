#include "windows.h"
#include "column.h"
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "listView.h"
#include "units.h"
#include "../Headers/DragList.h"
#include "res/resource.h"

extern char MainDir[200];
extern CSortListView*	cList;
extern HINSTANCE hInstance;
extern  HWND hwndMain;
extern BOOL	bHumanMode;
HANDLE	hColFile;
DWORD	dw;

COLUMN	Columns[UN_COUNT];
COLUMN	FirmCol[UNF_COUNT];
int		iOldOrder[UN_COUNT];
BOOL	bColModified;
DWORD	iSortUnit;	//32-бит признак направления сортировки
DWORD	iSortFirm;

CDragListBox*	box;

char* ColName[]={"Имя","","","Домашний телефон","Рабочий телефон","Факс","Домашняя страничка","E-Mail","ICQ","Адрес","Город","Область","Страна","Почтовый индекс","Сотовый телефон","Пейджер","Должность","Примечание","Жена/Муж","Дети","Дата рождения","Организация","Пол","Группа"};
char* FirmColName[]={"Название","Официальный сайт","E-Mail","Телефон №1","Телефон №2","Адрес","Город","Область","Почтовый индекс","Страна","Факс","Описание","Реквизиты","Разное"};

void CheckSortOrder()
{
	DWORD i;
	int ii;
	BOOL b;
	cList->GetSortInfo(&ii,&b);
	i=iOldOrder[ii];
	if (b) i|=0x8000;
	DWORD*	pi;
	if (bHumanMode) pi=&iSortUnit;
	else pi=&iSortFirm;
	if (*pi!=i) bColModified=TRUE;
	*pi=i;
}

void OnChangeHeadWidth(WPARAM wParam, LPARAM lParam)
{
	if (iOldOrder[lParam]==-1) return;
	int nIndex=iOldOrder[lParam];
	if (bHumanMode) Columns[nIndex].wWidth=(WORD)wParam;
	else FirmCol[nIndex].wWidth=(WORD)wParam;
	bColModified=TRUE;
}

void OnChangeOrder(WPARAM wParam, LPARAM lParam)
{
	if (iOldOrder[lParam]==-1) return;
	int nIndex=iOldOrder[lParam];
	PCOLUMN	col=bHumanMode ? &Columns[0] : &FirmCol[0];
	int max=bHumanMode ? UN_COUNT : UNF_COUNT;
	int iOld=col[nIndex].iOrder;
	int iNew=col[iOldOrder[wParam]].iOrder;
	for (int i=0; i<max; i++)
		if (col[i].iOrder>iOld) col[i].iOrder--;
	for (int i=0; i<max; i++)
		if (col[i].iOrder>=iNew) col[i].iOrder++;
	col[nIndex].iOrder=iNew;
	bColModified=TRUE;
}

void OnHideColumn(LPARAM lParam)
{
	if (iOldOrder[lParam]==-1) return;
	int nIndex=iOldOrder[lParam];
	if (bHumanMode) Columns[nIndex].bVisible=FALSE;
	else FirmCol[nIndex].bVisible=FALSE;
	bColModified=TRUE;
}

void UpdateColumns()
{
	cList->BeginUpdate();
	cList->ClearColumns();
	PCOLUMN	col=bHumanMode ? &Columns[0] : &FirmCol[0];
	int max=bHumanMode ? UN_COUNT : UNF_COUNT;
	int Order=0;
	for (int i=0; i<max; i++)
	{
		int j=0;
		while (j<max && col[j].iOrder!=i) j++;
		if (j<max && col[j].bVisible)
		{
			cList->AddColumn(bHumanMode ? ColName[j] : FirmColName[j] ,col[j].wWidth);
			iOldOrder[Order++]=j;
		}
	}
	cList->EndUpdate();
}

void LoadColumns()
{
	for (int i=0; i<UN_COUNT; i++)
	{
		Columns[i].bInFilter=FALSE;
		Columns[i].bVisible=FALSE;
		Columns[i].iOrder=i;
		Columns[i].wWidth=100;
	}
	Columns[UN_NAME].bVisible=TRUE;
	Columns[UN_MNAME].iOrder=-1;
	Columns[UN_SNAME].iOrder=-1;
	Columns[UN_BDATE].bVisible=TRUE;
	Columns[UN_HOMETEL].bVisible=TRUE;
	Columns[UN_EMAIL].bVisible=TRUE;
	Columns[UN_DOLV].bVisible=TRUE;

	for (int i=0; i<UNF_COUNT; i++)
	{
		FirmCol[i].bInFilter=FALSE;
		FirmCol[i].bVisible=FALSE;
		FirmCol[i].iOrder=i;
		FirmCol[i].wWidth=100;
	}
	FirmCol[UNF_NAME].bVisible=TRUE;
	FirmCol[UNF_EMAIL].bVisible=TRUE;
	FirmCol[UNF_TEL1].bVisible=TRUE;
	FirmCol[UNF_CITY].bVisible=TRUE;

	char* temp=(char*)malloc(220);
	lstrcpy(temp,MainDir);
	lstrcat(temp,"col.ini");
	HANDLE hColFile=CreateFile(temp,GENERIC_READ, 0,0, OPEN_EXISTING,0,NULL);
	if (hColFile!=INVALID_HANDLE_VALUE)
	{
		ReadFile(hColFile,temp,4,&dw,NULL);
		if (temp[0]==99 && temp[1]==111 && temp[2]==108 && temp[3]==31)
		{
			ReadFile(hColFile,&Columns[0],sizeof(COLUMN)*UN_COUNT,&dw,NULL);
			ReadFile(hColFile,&FirmCol[0],sizeof(COLUMN)*UNF_COUNT,&dw,NULL);
			ReadFile(hColFile,&iSortUnit,sizeof(DWORD),&dw,NULL);
			ReadFile(hColFile,&iSortFirm,sizeof(DWORD),&dw,NULL);
		}
		CloseHandle(hColFile);
	}
	free(temp);
	bColModified=FALSE;
}

void SaveColumns()
{
	CheckSortOrder();
	if (!bColModified) return;
	char* temp=(char*)malloc(220);
	lstrcpy(temp,MainDir);
	lstrcat(temp,"col.ini");
	HANDLE hColFile=CreateFile(temp,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hColFile!=INVALID_HANDLE_VALUE)
	{
		temp[0]=99;
		temp[1]=111;
		temp[2]=108;
		temp[3]=31;
		WriteFile(hColFile,temp,4,&dw,NULL);
		WriteFile(hColFile,&Columns[0],sizeof(COLUMN)*UN_COUNT,&dw,NULL);
		WriteFile(hColFile,&FirmCol[0],sizeof(COLUMN)*UNF_COUNT,&dw,NULL);
		WriteFile(hColFile,&iSortUnit,sizeof(DWORD),&dw,NULL);
		WriteFile(hColFile,&iSortFirm,sizeof(DWORD),&dw,NULL);
		CloseHandle(hColFile);
	}
	free(temp);
	bColModified=FALSE;
}

void UpdateOptBox()
{
	box->BeginUpdate();
	box->ClearAll();
	int o=0;
	PCOLUMN	col=bHumanMode ? &Columns[0] : &FirmCol[0];
	int max=bHumanMode ? UN_COUNT : UNF_COUNT;
	for (int i=0; i<max; i++)
	{
		int j=0;
		while (j<max && col[j].iOrder!=i) j++;
		if (j<max && j!=1 && j!=2)
		{
			box->AddItem(bHumanMode ? ColName[j] : FirmColName[j],col[j].bVisible,NULL);
			iOldOrder[o++]=j;
		}
	}
	box->EndUpdate();
}

void OnVisibleChange(LPARAM lParam, int nIndex)
{
	int i=iOldOrder[nIndex];
	if (i==-1) return;
	if (bHumanMode) Columns[i].bVisible=lParam;
	else FirmCol[i].bVisible=lParam;
}

void OnOrderChange(int from,int to)
{
	if (iOldOrder[from]==-1) return;
	int nIndex=iOldOrder[from];
	PCOLUMN	col=bHumanMode ? &Columns[0] : &FirmCol[0];
	int max=bHumanMode ? UN_COUNT : UNF_COUNT;
	int iOld=col[nIndex].iOrder;
	int to2=col[iOldOrder[to]].iOrder;
	for (int i=0; i<max; i++)
		if (col[i].iOrder>iOld) col[i].iOrder--;
	for (int i=0; i<max; i++)
		if (col[i].iOrder>=to2) col[i].iOrder++;
	col[nIndex].iOrder=to2;
	int i=SendMessage(box->hWnd,LB_GETTOPINDEX,0,0);
	UpdateOptBox();
	SendMessage(box->hWnd,LB_SETTOPINDEX,i,0);
	SendMessage(box->hWnd,LB_SETCURSEL,to,0);
}

BOOL FAR PASCAL ColOptWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	static BOOL  bColModif;

    switch (message) {
	case WM_LISTBOXCHECKCHANGE:
		if (LOWORD(wParam)==0) box->SetCheck(0,FALSE);
		else
		{
			bColModif=TRUE;
			OnVisibleChange(lParam,LOWORD(wParam));
		}
		break;
	case WM_LISTBOXENDDRAG:
		if (LOWORD(wParam)!=0 && LOWORD(lParam)!=0)
		{
			bColModif=TRUE;
			OnOrderChange(LOWORD(wParam),LOWORD(lParam));
		}
		break;
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (wParam==IDC_LIST1) return box->OnChildNotify(message,wParam,lParam);
		break;
	case WM_INITDIALOG:	
		bColModif=FALSE;
		PutOnCenter(hDlg);
		box=new CDragListBox(GetDlgItem(hDlg,IDC_LIST1),TRUE,0);
		UpdateOptBox();
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam)==IDOK || LOWORD(wParam)==IDCANCEL)
		{
			delete box;
			EndDialog(hDlg,bColModif);
			return TRUE;
	    }
	}
	return FALSE;
}

void ShowColOption()
{
	if (DialogBox(hInstance, "COLOPT", hwndMain, (DLGPROC)ColOptWndProc))
	{
		UpdateAllData();
		bColModified=TRUE;
	}
}
