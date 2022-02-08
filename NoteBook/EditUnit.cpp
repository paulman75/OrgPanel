#include "windows.h"
#include "../Headers/general.h"
#include "units.h"
#include "EditUnit.h"
#include "res/resource.h"
#include <commctrl.h>
#include "../Headers/const.h"
#include "Group.h"

extern LPNOTEUNIT	ActiveUnit;
extern HINSTANCE hInstance;		  // Global instance handle for application
char*	Captions[]={"Личные","Домашние","Разное"};
extern  HWND hwndMain;

HWND	hIntDialog=NULL;
int		iIntIndex;
LPNOTEUNIT	EditUnit;
BOOL	bUnitModif;
extern  LPFIRMUNIT	FirstFirm;
extern  LPFIRMUNIT	nfNote;
BOOL	bHumanEditing;

#define IMENLABELID	12

int EditId[]={IDC_EDIT1,IDC_EDIT2,IDC_EDIT3,IDC_EDIT6,IDC_EDIT1,IDC_EDIT9,IDC_EDIT11,IDC_EDIT10,IDC_EDIT12,IDC_EDIT1,IDC_EDIT2,IDC_EDIT4,IDC_EDIT3,IDC_EDIT5,IDC_EDIT7,IDC_EDIT8,IDC_EDIT3,IDC_EDIT4,IDC_EDIT4,IDC_EDIT5};

//*************Диалог именин******************
HWND hPropDlg;

BOOL OnInitImenDlg(HWND hDlg)
{
	TDate bDate;
	char *temp=(char*)malloc(100);
	char *temp2=(char*)malloc(100);
	GetWindowText(GetDlgItem(hPropDlg,IDC_EDIT1),temp,20);
	int i=GetNameIndex(temp);
	if (i==-1)
	{
		free(temp);
		return FALSE;
	}
	SetWindowText(GetDlgItem(hDlg,IDC_STATICNAME),temp);
	GetDate(&bDate,1);
	DateToString(&bDate,temp);
	lstrcpy(temp2,"Дата рождения: ");
	lstrcat(temp2,temp);
	SetWindowText(GetDlgItem(hDlg,IDC_STATICD),temp2);
	TDate Mainimen,imen;
	GetHumanMainImenDate(i,&bDate,&Mainimen);
	DateToStringWithoutYear(&Mainimen,temp);
	lstrcpy(temp2,"Именины: ");
	lstrcat(temp2,temp);
	SetWindowText(GetDlgItem(hDlg,IDC_STATICD2),temp2);
	int j=0;
	while (TRUE)
	{
		if (!GetHumanImenDate(i,j++,&imen)) break;
		DateToStringWithoutYear(&imen,temp);
		if (imen.Day==Mainimen.Day && imen.Month==Mainimen.Month)
		{
			lstrcpy(temp2,"****** ");
			lstrcat(temp2,temp);
			lstrcpy(temp,temp2);
			lstrcat(temp," ******");
		}
		SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_ADDSTRING,0,(LPARAM)temp);
	}
	free(temp);
	free(temp2);
	return TRUE;
}

BOOL FAR PASCAL ImenWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		return OnInitImenDlg(hDlg);
	case WM_COMMAND:
		if ( LOWORD(wParam)==IDCANCEL || LOWORD(wParam)==IDOK) 
		{
			EndDialog(hDlg, TRUE);
			return (TRUE);
	    }
    }
    return (FALSE);
}

void ShowImen(HWND hd)
{
	hPropDlg=hd;
	DialogBox(hInstance, "IMEN", hd, (DLGPROC)ImenWndProc);
}

//*************Конец Диалог именин************

void UpdateImenin(HWND hDlg)
{
	char* name=(char*)malloc(100);
	GetWindowText(GetDlgItem(hDlg,IDC_EDIT1),name,100);
	int i=GetNameIndex(name);
	name[0]=0;
	if (i!=-1)
	{
		TDate dat,imen;
		GetDate(&dat,1);
		if (dat.Month)
		{
			GetHumanMainImenDate(i,&dat,&imen);
			if (imen.Month) DateToStringWithoutYear(&imen,name);
		}
	}
	if (name[0]) SetSensorCaption(name,IMENLABELID);
	ShowSensorLabel(IMENLABELID,name[0]);//SetWindowText(GetDlgItem(hDlg,IDC_STATIC3),name);
	free(name);
}

void UpdateAgeIcon(HWND hDlg)
{
	int iID[]={IDC_STATICNO,IDC_STATICBOY,IDC_STATICMAN,IDC_STATICGMAN,IDC_STATICGIRL,IDC_STATICWOMAN,IDC_STATICGWOMAN};
	int v;
	if (!EditUnit->BDate.Month || !EditUnit->Sex) v=0;
	else
	{
		v=(EditUnit->Sex-1)*3+1;
		TDate d=Now();
		int y=d.Year-EditUnit->BDate.Year;
		if (EditUnit->BDate.Month>d.Month) y++;
		if (EditUnit->BDate.Month==d.Month && EditUnit->BDate.Day>=d.Day) y++;
		if (y>15) v++;
		if (y>35) v++;
	}
	for (int i=0; i<7; i++) ShowWindow(GetDlgItem(hDlg,iID[i]),i!=v ? SW_HIDE : SW_SHOW);
	char* sWife[]={"Супруг(а):","Жена:","Муж:"};
	SetWindowText(GetDlgItem(hDlg,IDC_STATICWIFE),sWife[EditUnit->Sex]);
}

void SetHumanDate(int nIndex)
{
	SetWindowText(GetDlgItem(hIntDialog,EditId[nIndex]),EditUnit->Fields[nIndex]);
}

void GetHumanDate(int nIndex)
{
	if (EditUnit->Fields[nIndex]) free (EditUnit->Fields[nIndex]);
	EditUnit->Fields[nIndex]=NULL;
	int l=GetWindowTextLength(GetDlgItem(hIntDialog,EditId[nIndex]));
	if (l)
	{
		EditUnit->Fields[nIndex]=(char*)malloc(l+1);
		GetWindowText(GetDlgItem(hIntDialog,EditId[nIndex]),EditUnit->Fields[nIndex],l+1);
	}
}

BOOL Creating;
int iLastLen;

void OnEditChange(HWND hEdit)
{
	Creating=TRUE;
	char* temp=(char*)malloc(100);
	GetWindowText(hEdit,temp,100);
	char* name=(char*)malloc(100);
	int i=0;
	int len=lstrlen(temp);
	while (TRUE)
	{
		if (!GetHumanName(i++,name)) break;
		if (len>iLastLen && StringCompare(temp,name)==2)
		{
			DWORD dwS,dwE;
			int cur=SendMessage(hEdit,EM_GETSEL,(WPARAM)&dwS,(LPARAM)&dwE);
			SetWindowText(hEdit,name);
			SendMessage(hEdit,EM_SETSEL,lstrlen(temp),lstrlen(name));
			break;
		}
	}
	iLastLen=len;
	free(name);
	free(temp);
	Creating=FALSE;
}

void UpdateGroupLabel()
{
	char *t=GetGroupName(EditUnit->Group);
	if (!t)	t="не задана";
	SetWindowText(GetDlgItem(hIntDialog,IDC_GROUP),t);
}

BOOL FAR PASCAL IntDialogProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	int i;
    switch (message) {
	case WM_INITDIALOG:
		Creating=TRUE;
		iLastLen=0;
		hIntDialog=hDlg;
		switch (iIntIndex)
		{
		case 0:
			SendMessage(GetDlgItem(hDlg,IDC_COMBOSEX),CB_ADDSTRING,0,(LPARAM)"Не указан");
			SendMessage(GetDlgItem(hDlg,IDC_COMBOSEX),CB_ADDSTRING,0,(LPARAM)"Мужской");
			SendMessage(GetDlgItem(hDlg,IDC_COMBOSEX),CB_ADDSTRING,0,(LPARAM)"Женский");
			UpdateAgeIcon(hDlg);
			RECT rc,rect2;
			GetClientRect(hDlg,&rc);
			POINT pt;
			GetWindowRect(GetDlgItem(hDlg,IDC_STATICDATA),&rect2);
			pt.x=(rect2.right+rect2.left)/2;
			ScreenToClient(hDlg,&pt);
			CreateDateLabel(hDlg,pt.x,(int)(rc.bottom*0.93),&(EditUnit->BDate),1,TRUE);
			GetWindowRect(GetDlgItem(hDlg,IDC_STATICIMEN),&rect2);
			pt.x=(rect2.right+rect2.left)/2;
			ScreenToClient(hDlg,&pt);
			CreateSensorLabel(hDlg,pt.x,(int)(rc.bottom*0.93),0xff0000,0xffffc6,NULL,1,IMENLABELID);
			SetHumanDate(UN_NAME);
			SetHumanDate(UN_MNAME);
			SetHumanDate(UN_SNAME);
			SetHumanDate(UN_WIFE);
			SetHumanDate(UN_CHILDREN);
			SendMessage(GetDlgItem(hDlg,IDC_COMBOSEX),CB_SETCURSEL,EditUnit->Sex,0);
			UpdateImenin(hDlg);
			break;
		case 1:
			SetHumanDate(UN_HOMETEL);
			SetHumanDate(UN_FAX);
			for (i=UN_FAX; i<=UN_PAGER; i++)
				SetHumanDate(i);
			break;
		case 2:
			SetHumanDate(UN_WORKTEL);
			SetHumanDate(UN_DOLV);
			SetHumanDate(UN_MISC);
			nfNote=FirstFirm;
			i=SendMessage(GetDlgItem(hDlg,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)"");
			SendMessage(GetDlgItem(hDlg,IDC_COMBO1),CB_SETITEMDATA,i,0);
			while (nfNote)
			{
				if (nfNote->Fields[UNF_NAME]) i=SendMessage(GetDlgItem(hDlg,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)nfNote->Fields[UNF_NAME]);
				else i=SendMessage(GetDlgItem(hDlg,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)"");
				SendMessage(GetDlgItem(hDlg,IDC_COMBO1),CB_SETITEMDATA,i,(LPARAM)nfNote);
				if (EditUnit->Firm==nfNote->Key) SendMessage(GetDlgItem(hDlg,IDC_COMBO1),CB_SETCURSEL,i,0);
				nfNote=nfNote->Next;
			}
			UpdateGroupLabel();
			break;
		}
		Creating=FALSE;
		return TRUE;
	case WM_DESTROY:
		switch (iIntIndex)
		{
		case 0:
			DeleteAllDateLabels();
			DeleteAllSensorLabels();
			GetHumanDate(UN_NAME);
			GetHumanDate(UN_MNAME);
			GetHumanDate(UN_SNAME);
			GetHumanDate(UN_WIFE);
			GetHumanDate(UN_CHILDREN);
			break;
		case 1:
			GetHumanDate(UN_HOMETEL);
			GetHumanDate(UN_FAX);
			for (i=UN_FAX; i<=UN_PAGER; i++)
				GetHumanDate(i);
			break;
		case 2:
			GetHumanDate(UN_WORKTEL);
			GetHumanDate(UN_DOLV);
			GetHumanDate(UN_MISC);
			break;
		}
		break;
	case WM_CALENDARDATECHANGE:
		GetDate(&EditUnit->BDate,1);
		UpdateAgeIcon(hDlg);
		UpdateImenin(hDlg);
		bUnitModif=TRUE;
		break;
	case WM_SENSORLABELCLICK:
		if (lParam==IMENLABELID) ShowImen(hDlg);
		break;
	case WM_COMMAND:
		if (HIWORD(wParam)==CBN_SELCHANGE && LOWORD(wParam)==IDC_COMBOSEX)
		{
			EditUnit->Sex=(byte)SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
			UpdateAgeIcon(hDlg);
			bUnitModif=TRUE;
		}
		if (HIWORD(wParam)==CBN_SELCHANGE && LOWORD(wParam)==IDC_COMBO1)
		{
			i=SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
			nfNote=(LPFIRMUNIT)SendMessage((HWND)lParam,CB_GETITEMDATA,i,0);
			if (nfNote) EditUnit->Firm=nfNote->Key;
			else EditUnit->Firm=0xffffffff;
			bUnitModif=TRUE;
		}
		if (HIWORD(wParam)==EN_CHANGE && !Creating)
		{
			bUnitModif=TRUE;
			if (LOWORD(wParam)==IDC_EDIT1 && !iIntIndex) OnEditChange((HWND)lParam);
		}
		if (HIWORD(wParam)==EN_KILLFOCUS && LOWORD(wParam)==IDC_EDIT1 && !iIntIndex) UpdateImenin(hDlg);
		if (HIWORD(wParam)==CBN_SELCHANGE && LOWORD(wParam)==IDC_COMBO1)
		{
			bUnitModif=TRUE;
		}
		if (HIWORD(wParam)==BN_CLICKED && LOWORD(wParam)==IDC_BUTGROUP)
		{
			SelectGroup(&EditUnit->Group,hDlg);
			bUnitModif=TRUE;
			UpdateGroupLabel();
		}
		break;
	}
	return FALSE;
}

void UpdateDialog(int iIndex, HWND hDlg)
{
	DestroyWindow(hIntDialog);
	iIntIndex=iIndex;
	if (iIndex==-1) return;
	char	Dia[]="INTDIA ";
	Dia[6]=iIndex+48;
	CreateDialog(hInstance,Dia,hDlg,IntDialogProc);
	ShowWindow(hIntDialog,SW_SHOW);
}

BOOL FAR PASCAL PropWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	int i;

    switch (message) {
	case WM_SHOWWINDOW:
		SetFocus(GetDlgItem(hIntDialog,!iIntIndex ? IDC_EDIT3 : IDC_EDIT1));
		break;
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		TC_ITEM	tc;
		ZeroMemory(&tc,sizeof(tc));
		tc.mask=TCIF_TEXT;
		for (i=0; i<3; i++)
		{
			tc.pszText=Captions[i];
			SendMessage(GetDlgItem(hDlg,IDC_TAB1),TCM_INSERTITEM,i,(LPARAM)&tc);
		}
		UpdateDialog(0,hDlg);
		return TRUE;
	case WM_NOTIFY:
		if (wParam==IDC_TAB1)
		{
			LPNMHDR nmh;
			nmh=(LPNMHDR)lParam;
			if (nmh->code==TCN_SELCHANGE) 
			{
				i=SendMessage(nmh->hwndFrom,TCM_GETCURSEL,0,0);
				UpdateDialog(i,hDlg);
			}
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam)==IDOK || LOWORD(wParam)==IDCANCEL)
		{
			UpdateDialog(-1,NULL);
			EndDialog(hDlg,LOWORD(wParam)==IDOK);
			return TRUE;
		}
	}
	return FALSE;
}

LPNOTEUNIT UnitProperty(LPNOTEUNIT nu)
{
	LPNOTEUNIT	eu=nu;
	if (!nu)
	{
		eu=(LPNOTEUNIT)malloc(sizeof(TNOTEUNIT));
		ZeroMemory(eu,sizeof(TNOTEUNIT));
		eu->Firm=0xffffffff;
		eu->Group=0xffffffff;
	}
	EditUnit=(LPNOTEUNIT)malloc(sizeof(TNOTEUNIT));
	EditUnit->BDate=eu->BDate;
	EditUnit->Firm=eu->Firm;
	EditUnit->Sex=eu->Sex;
	EditUnit->Group=eu->Group;
	for (int i=0; i<UN_BDATE; i++)
	{
		if (eu->Fields[i])
		{
			EditUnit->Fields[i]=(LPTSTR)malloc(lstrlen(eu->Fields[i])+1);
			lstrcpy(EditUnit->Fields[i],eu->Fields[i]);
		}
		else EditUnit->Fields[i]=0;
	}
	bUnitModif=FALSE;
	bHumanEditing=TRUE;
	BOOL b=DialogBox(hInstance, "UNITPROP", hwndMain, (DLGPROC)PropWndProc);
	bHumanEditing=FALSE;
	b=b&&bUnitModif;
	if (b)
	{
		eu->BDate=EditUnit->BDate;
		eu->Firm=EditUnit->Firm;
		eu->Sex=EditUnit->Sex;
		eu->Group=EditUnit->Group;
		for (int i=0; i<UN_BDATE; i++)
		{
			if (eu->Fields[i]) free(eu->Fields[i]);
			eu->Fields[i]=EditUnit->Fields[i];
		}
	}
	free(EditUnit);
	if (!nu && !b)
	{
		for (int i=0; i<UN_BDATE; i++)
			if (eu->Fields[i]) free(eu->Fields[i]);
		free(eu);
	}
	return b ? eu : NULL;
}
