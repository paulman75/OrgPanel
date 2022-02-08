#include "windows.h"
#include "../Headers/general.h"
#include "units.h"
#include "EditFirm.h"
#include "res/resource.h"
#include "../Headers/const.h"
#include <commctrl.h>

extern LPNOTEUNIT	ActiveFirm;
extern HINSTANCE hInstance;		  // Global instance handle for application
char*	FirmCaptions[]={"Общие","Адрес","Разное"};
extern  HWND hwndMain;
extern  LPNOTEUNIT	FirstUnit;
extern  LPNOTEUNIT	nNote;

HWND	hIntFirmDialog=NULL;
int		iFirmIntIndex;
LPFIRMUNIT	EditFirm;
int		FirmEditId[]={IDC_EDIT2,IDC_EDIT11,IDC_EDIT10,IDC_EDIT6,IDC_EDIT7,IDC_EDIT1,IDC_EDIT2,IDC_EDIT4,IDC_EDIT5,IDC_EDIT3,IDC_EDIT9,IDC_EDIT3,IDC_EDIT1,IDC_EDIT4};
BOOL	bFirmModif;
LPNOTEUNIT	ActiveUnitinFirm;

void UpdateActiveUnit(NM_LISTVIEW* nm)
{
	if (!(nm->uNewState&LVIS_SELECTED)) ActiveUnitinFirm=NULL;
	else
		ActiveUnitinFirm=(LPNOTEUNIT)nm->lParam;
	EnableWindow(GetDlgItem(hIntFirmDialog,IDC_BUTTON2),ActiveUnitinFirm!=NULL);
}

void SetFirmData(int nIndex)
{
	SetWindowText(GetDlgItem(hIntFirmDialog,FirmEditId[nIndex]),EditFirm->Fields[nIndex]);
}

void GetFirmData(int nIndex)
{
	if (EditFirm->Fields[nIndex]) free (EditFirm->Fields[nIndex]);
	EditFirm->Fields[nIndex]=NULL;
	int l=GetWindowTextLength(GetDlgItem(hIntFirmDialog,FirmEditId[nIndex]));
	if (l)
	{
		EditFirm->Fields[nIndex]=(char*)malloc(l+1);
		GetWindowText(GetDlgItem(hIntFirmDialog,FirmEditId[nIndex]),EditFirm->Fields[nIndex],l+1);
	}
}

void SetListView(HWND hList)
{
	SendMessage(hList,WM_SETREDRAW,0,0);
	SendMessage(hList,LVM_DELETEALLITEMS,0,0);
	nNote=FirstUnit;
	int iItem=0;
	char* temp=(char*)malloc(1000);
	while (nNote)
	{
		if (nNote->Firm==EditFirm->Key)
		{
			LV_ITEM	lvi;
			MakeUnitName(nNote,temp);
			lvi.pszText=temp;
			lvi.lParam=(LPARAM)nNote;
			lvi.iItem=iItem++;
			lvi.iSubItem=0;
			lvi.mask=LVIF_TEXT | LVIF_PARAM;
			SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&lvi);
			lvi.pszText=nNote->Fields[UN_DOLV];
			lvi.mask=LVIF_TEXT;
			lvi.iSubItem=1;
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&lvi);
		}
		nNote=nNote->Next;
	}
	free(temp);
	SendMessage(hList,WM_SETREDRAW,1,0);
	InvalidateRect(hList,NULL,FALSE);
}

//*********************************************
//AddHuman to Firm
LPNOTEUNIT f_ActiveUnit;
int	f_TimerId;

void UpdateUnitList(HWND hDlg)
{
	char* temp=(char*)malloc(100);
	temp[0]=0;
	GetWindowText(GetDlgItem(hDlg,IDC_EDIT1),temp,100);
	HWND hList=GetDlgItem(hDlg,IDC_LIST1);
	SendMessage(hList,WM_SETREDRAW,0,0);
	while (TRUE)
	{
		int c=SendMessage(hList,LB_GETCOUNT,0,0);
		if (!c) break;
		for (int j=0; j<c; j++) SendMessage(hList,LB_DELETESTRING,j,0);
	}
	nNote=FirstUnit;
	int iAll=0;
	int iView=0;
	char* t=(char*)malloc(1000);
	while (nNote)
	{
		iAll++;
		MakeUnitName(nNote,t);
		BOOL b=temp[0]==0;
		if (!b)
		{
			int i=FindInString(t,temp);
			if (i!=-1) b=TRUE;
		}
		if (b)
		{
			int i=SendMessage(hList,LB_ADDSTRING,0,(LPARAM)t);
			SendMessage(hList,LB_SETITEMDATA,i,(LPARAM)nNote);
			iView++;
		}
		nNote=nNote->Next;
	}
	f_ActiveUnit=NULL;
	wsprintf(t,"Показано: %d из %d",iView,iAll);
	SendMessage(GetDlgItem(hDlg,IDC_STATICVIEW),WM_SETTEXT,0,(LPARAM)t);
	free(t);
	free(temp);
	SendMessage(hList,WM_SETREDRAW,1,0);
	InvalidateRect(hList,NULL,FALSE);
}

BOOL FAR PASCAL FirmAddWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		UpdateUnitList(hDlg);
		f_TimerId=0;
		return TRUE;
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (LOWORD(wParam)==IDOK)
			{
				f_ActiveUnit->Firm=EditFirm->Key;
				EndDialog(hDlg,TRUE);
				return TRUE;
			}
			if (LOWORD(wParam)==IDCANCEL)
			{
				EndDialog(hDlg,FALSE);
				return TRUE;
			}
			break;
		case LBN_SELCHANGE:
			int i;
			i=SendMessage((HWND)lParam,LB_GETCURSEL,0,0);
			if (i!=LB_ERR)
				f_ActiveUnit=(LPNOTEUNIT)SendMessage((HWND)lParam,LB_GETITEMDATA,i,0);
			else f_ActiveUnit=NULL;
			EnableWindow(GetDlgItem(hDlg,IDOK),f_ActiveUnit!=NULL);
			break;
		case EN_CHANGE:
			if (f_TimerId) KillTimer(hDlg,f_TimerId);
			f_TimerId=SetTimer(hDlg,1,1000,NULL);
			break;
		}
		break;
		case WM_TIMER:
			KillTimer(hDlg,f_TimerId);
			f_TimerId=0;
			UpdateUnitList(hDlg);
			break;
	}
	return FALSE;
}

void AddHumanToFirm()
{
	if (DialogBox(hInstance, "ADDTOFIRM", hIntFirmDialog, (DLGPROC)FirmAddWndProc))
		SetListView(GetDlgItem(hIntFirmDialog,IDC_LIST1));
}
//End of AddHuman to Firm
//*********************************************

BOOL FAR PASCAL FirmIntDialogProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	int i;
    switch (message) {
	case WM_INITDIALOG:
		hIntFirmDialog=hDlg;
		switch (iFirmIntIndex)
		{
		case 0:
			LV_COLUMN lvc;
			RECT rc;
			HWND hList;
			hList=GetDlgItem(hDlg,IDC_LIST1);
			GetClientRect(hList,&rc);
			lvc.mask=LVCF_WIDTH | LVCF_TEXT;
			lvc.pszText="ФИО";
			lvc.cx=rc.right/2;
			SendMessage(hList,LVM_INSERTCOLUMN,0,(LPARAM)&lvc);
			lvc.pszText="Должность";
			SendMessage(hList,LVM_INSERTCOLUMN,1,(LPARAM)&lvc);
			SetFirmData(UNF_NAME);
			SetFirmData(UNF_OPIS);
			SetListView(hList);
			EnableWindow(GetDlgItem(hDlg,IDC_BUTTON2),FALSE);
			break;
		case 1:
			for (i=UNF_HOMEPAGE; i<=UNF_FAX; i++)
				SetFirmData(i);
			break;
		case 2:
			SetFirmData(UNF_REKV);
			SetFirmData(UNF_MISC);
			break;
		}
		return TRUE;
	case WM_DESTROY:
		switch (iFirmIntIndex)
		{
		case 0:
			GetFirmData(UNF_NAME);
			GetFirmData(UNF_OPIS);
			break;
		case 1:
			for (i=UNF_HOMEPAGE; i<=UNF_FAX; i++)
				GetFirmData(i);
			break;
		case 2:
			GetFirmData(UNF_REKV);
			GetFirmData(UNF_MISC);
			break;
		}
		break;
	case WM_NOTIFY:
		if (wParam==IDC_LIST1)
		{
			NM_LISTVIEW* lpnm;
			lpnm=(NM_LISTVIEW*)lParam;
			if (lpnm->hdr.code==LVN_ITEMCHANGED && lpnm->uChanged&LVIF_STATE) 
				UpdateActiveUnit(lpnm);
		}
		break;
	case WM_COMMAND:
		if (HIWORD(wParam)==EN_CHANGE) bFirmModif=TRUE;
		if (HIWORD(wParam)==BN_CLICKED)
		{
			if (LOWORD(wParam)==IDC_BUTTON2)
			{
				ActiveUnitinFirm->Firm=0xffffffff;
				SetListView(GetDlgItem(hDlg,IDC_LIST1));
				EnableWindow(GetDlgItem(hIntFirmDialog,IDC_BUTTON2),FALSE);
				bFirmModif=TRUE;
			}
			if (LOWORD(wParam)==IDC_BUTTON1) AddHumanToFirm();
		}
		break;
	}
	return FALSE;
}

void UpdateFirmDialog(int iIndex, HWND hDlg)
{
	DestroyWindow(hIntFirmDialog);
	iFirmIntIndex=iIndex;
	if (iIndex==-1) return;
	char	Dia[]="INTDIA ";
	Dia[6]=iIndex+51;
	CreateDialog(hInstance,Dia,hDlg,FirmIntDialogProc);
	ShowWindow(hIntFirmDialog,SW_SHOW);
}

BOOL FAR PASCAL FirmPropWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	int i;

    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		TC_ITEM	tc;
		ZeroMemory(&tc,sizeof(tc));
		tc.mask=TCIF_TEXT;
		for (i=0; i<3; i++)
		{
			tc.pszText=FirmCaptions[i];
			SendMessage(GetDlgItem(hDlg,IDC_TAB1),TCM_INSERTITEM,i,(LPARAM)&tc);
		}
		UpdateFirmDialog(0,hDlg);
		return TRUE;
	case WM_SHOWWINDOW:
		if (!iFirmIntIndex) SetFocus(GetDlgItem(hIntFirmDialog,IDC_EDIT2));
		break;
	case WM_NOTIFY:
		if (wParam==IDC_TAB1)
		{
			LPNMHDR nmh;
			nmh=(LPNMHDR)lParam;
			if (nmh->code==TCN_SELCHANGE) 
			{
				i=SendMessage(nmh->hwndFrom,TCM_GETCURSEL,0,0);
				UpdateFirmDialog(i,hDlg);
			}
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam)==IDOK || LOWORD(wParam)==IDCANCEL)
		{
			UpdateFirmDialog(-1,NULL);
			EndDialog(hDlg,TRUE);
			return TRUE;
		}
	}
	return FALSE;
}

LPFIRMUNIT FirmProperty(LPFIRMUNIT nu,DWORD dwNewKey)
{
	LPFIRMUNIT ef=nu;
	if (!nu)
	{
		ef=(LPFIRMUNIT)malloc(sizeof(TFIRMUNIT));
		ZeroMemory(ef,sizeof(TFIRMUNIT));
		ef->Key=dwNewKey;
	}
	EditFirm=(LPFIRMUNIT)malloc(sizeof(TFIRMUNIT));
	EditFirm->Key=ef->Key;
	for (int i=0; i<UNF_COUNT; i++)
	{
		if (ef->Fields[i])
		{
			EditFirm->Fields[i]=(LPTSTR)malloc(lstrlen(ef->Fields[i])+1);
			lstrcpy(EditFirm->Fields[i],ef->Fields[i]);
		}
		else EditFirm->Fields[i]=NULL;
	}
	bFirmModif=FALSE;
	BOOL b=DialogBox(hInstance, "UNITPROP", hwndMain, (DLGPROC)FirmPropWndProc);
	b=b&&bFirmModif;
	if (b)
	{
		for (int i=0; i<UNF_COUNT; i++)
		{
			if (ef->Fields[i]) free(ef->Fields[i]);
			ef->Fields[i]=EditFirm->Fields[i];
		}
	}
	free(EditFirm);
	if (!nu && !b)
	{
		for (int i=0; i<UNF_COUNT; i++)
			if (ef->Fields[i]) free(ef->Fields[i]);
		free(ef);
	}
	return b ? ef : NULL;
}
