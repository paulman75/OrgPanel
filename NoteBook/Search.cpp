#include "windows.h"
#include "../Headers/general.h"
#include "res/resource.h"
#include "Units.h"
#include "search.h"
#include "EditUnit.h"
#include "EditFirm.h"

extern  HINSTANCE   hInstance;		  // Global instance handle for application
extern  LPNOTEUNIT	FirstUnit;
extern  LPNOTEUNIT	nNote;
extern  LPFIRMUNIT	FirstFirm;
extern  LPFIRMUNIT	nfNote;
extern	BOOL	bHumanMode;
extern  BOOL	g_bUnitModified;
int		NormalW;
int 	NormalH;
HWND	hsList;
HWND	hswnd;
int		iItems;
LPARAM	ActiveU;

BOOL Check(LPCTSTR s, LPCTSTR s2)
{
	if (!lstrlen(s2)) return TRUE;
	if (!s) return FALSE;
	return (FindInString(s,s2)!=-1);
}

BOOL Check(LPCTSTR s, int ID)
{
	char* t=(char*)malloc(100);
	GetWindowText(GetDlgItem(hswnd,ID),t,100);
	if (!lstrlen(t)) return TRUE;
	BOOL b;
	if (!s) return b=FALSE;
	else b=FindInString(s,t)!=-1;
	free (t);
	return b;
}

void UpdateButtons(NM_LISTVIEW* nm)
{
	if (!(nm->uNewState&LVIS_SELECTED)) ActiveU=NULL;
	else
		ActiveU=nm->lParam;
	EnableWindow(GetDlgItem(hswnd,IDC_BUTTON4),ActiveU!=0);
	EnableWindow(GetDlgItem(hswnd,IDC_BUTTON5),ActiveU!=0);
}

int CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int res;
	if (bHumanMode)
	{
		nNote=(LPNOTEUNIT)lParam1;
		LPNOTEUNIT nNote2=(LPNOTEUNIT)lParam2;
		res=CompareStrings(nNote->Fields[2],nNote2->Fields[2]);
	}
	else
	{
		nfNote=(LPFIRMUNIT)lParam1;
		LPFIRMUNIT nfNote2=(LPFIRMUNIT)lParam2;
		res=CompareStrings(nfNote->Fields[0],nfNote2->Fields[0]);
	}
	return res;
}

void AddNote(LPTSTR s1,LPTSTR s2,LPTSTR s3,LPTSTR s4, LPARAM lpr)
{
	LV_ITEM lvi;
	lvi.pszText=s1;
	lvi.lParam=lpr;
	lvi.iItem=iItems++;
	lvi.iSubItem=0;
	lvi.mask=LVIF_TEXT | LVIF_PARAM;
	SendMessage(hsList,LVM_INSERTITEM,0,(LPARAM)&lvi);
	lvi.pszText=s2;
	lvi.iSubItem=1;
	lvi.mask=LVIF_TEXT;
	SendMessage(hsList,LVM_SETITEM,0,(LPARAM)&lvi);
	lvi.pszText=s3;
	lvi.iSubItem=2;
	lvi.mask=LVIF_TEXT;
	SendMessage(hsList,LVM_SETITEM,0,(LPARAM)&lvi);
	lvi.pszText=s4;
	lvi.iSubItem=3;
	lvi.mask=LVIF_TEXT;
	SendMessage(hsList,LVM_SETITEM,0,(LPARAM)&lvi);
}

void Find()
{
	int l=SendMessage(GetDlgItem(hswnd,IDC_EDIT1),WM_GETTEXTLENGTH,0,0);
	l+=SendMessage(GetDlgItem(hswnd,IDC_EDIT2),WM_GETTEXTLENGTH,0,0);
	l+=SendMessage(GetDlgItem(hswnd,IDC_EDIT3),WM_GETTEXTLENGTH,0,0);
	l+=SendMessage(GetDlgItem(hswnd,IDC_EDIT13),WM_GETTEXTLENGTH,0,0);
	if (!l)
	{
		MessageBox(hswnd,"Необходимо задать критерий поиска","Ошибка",MB_ICONSTOP);
		return;
	}
	ActiveU=0;
	SetWindowPos(hswnd,0,0,0,NormalW,NormalH,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	ShowWindow(hsList,SW_SHOW);
	ShowWindow(GetDlgItem(hswnd,IDC_BUTTON4),SW_SHOW);
	ShowWindow(GetDlgItem(hswnd,IDC_BUTTON5),SW_SHOW);
	SendMessage(hsList,LVM_DELETEALLITEMS,0,0);
	EnableWindow(GetDlgItem(hswnd,IDC_BUTTON4),FALSE);
	EnableWindow(GetDlgItem(hswnd,IDC_BUTTON5),FALSE);

	iItems=0;
	char* temp=(char*)malloc(100);
	char* t2=(char*)malloc(100);
	int Index;
	BOOL b2;
	GetWindowText(GetDlgItem(hswnd,IDC_EDIT1),t2,100);
	BOOL b;
	if (bHumanMode)
	{
		nNote=FirstUnit;
		while (nNote)
		{
			b=Check(nNote->Fields[UN_NAME],t2)
				|| Check(nNote->Fields[UN_SNAME],t2)
				|| Check(nNote->Fields[UN_MNAME],t2);
			b=b && Check(nNote->Fields[UN_EMAIL],IDC_EDIT2);
			b2=Check(nNote->Fields[UN_HOMETEL],IDC_EDIT3)
			 || Check(nNote->Fields[UN_WORKTEL],IDC_EDIT3);
			b=b&&b2;
			Index=UN_MISC;
			b2=Check(nNote->Fields[UN_FAX],IDC_EDIT13);
			if (b2) Index=UN_FAX;
			else
			{
				b2=Check(nNote->Fields[UN_HOMEPAGE],IDC_EDIT13);
				if (b2) Index=UN_HOMEPAGE;
				else
				{
					for (int i=UN_ICQ; i<UN_BDATE; i++)
					{
						if (Check(nNote->Fields[i],IDC_EDIT13))
						{
							Index=i;
							b2=TRUE;
							break;
						}
					}
				}
			}
			if (b && b2)
			{
				MakeUnitName(nNote,temp);
				AddNote(temp,nNote->Fields[UN_EMAIL],nNote->Fields[UN_HOMETEL],nNote->Fields[Index],(LPARAM)nNote);
			}
			nNote=nNote->Next;
		}
	}
	else
	{
		nfNote=FirstFirm;
		while (nfNote)
		{
			b=Check(nfNote->Fields[UNF_NAME],t2);
			b=b && Check(nfNote->Fields[UNF_EMAIL],IDC_EDIT2);
			b2=Check(nfNote->Fields[UNF_TEL1],IDC_EDIT3)
			 || Check(nfNote->Fields[UNF_TEL2],IDC_EDIT3);
			b=b&&b2;
			Index=UNF_MISC;
			b2=Check(nfNote->Fields[UNF_HOMEPAGE],IDC_EDIT13);
			if (b2) Index=UNF_HOMEPAGE;
			else
			{
				for (int i=UNF_ADDRESS; i<=UNF_MISC; i++)
				{
					if (Check(nfNote->Fields[i],IDC_EDIT13))
					{
						Index=i;
						b2=TRUE;
						break;
					}
				}
			}
			if (b && b2)
			{
				AddNote(nfNote->Fields[UNF_NAME],nfNote->Fields[UNF_EMAIL],nfNote->Fields[UNF_TEL1],nfNote->Fields[Index],(LPARAM)nfNote);
			}
			nfNote=nfNote->Next;
		}
	}
	free (temp);
	free (t2);
	SendMessage(hsList,LVM_SORTITEMS,0,(LPARAM)SortFunc);
}

void OnInit()
{
	RECT rc;
	GetWindowRect(hswnd,&rc);
	NormalH=rc.bottom-rc.top;
	NormalW=rc.right-rc.left;
	SetWindowPos(hswnd,0,0,0,NormalW,NormalH/2+2,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	if (!bHumanMode)
		SetWindowText(GetDlgItem(hswnd,IDC_STATIC1),"Название:");
	char* HumanCap[4]={"Имя","E-Mail","Телефон","Другое"};
	char* FirmCap[4]={"Название","E-Mail","Телефон","Другое"};
	hsList=GetDlgItem(hswnd,IDC_LIST1);
	SendMessage(hsList,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
	GetClientRect(hsList,&rc);
	for (int i=0; i<4; i++)
	{
		LV_COLUMN lvc;
		lvc.mask=LVCF_WIDTH | LVCF_TEXT;
		lvc.pszText=bHumanMode ? HumanCap[i] : FirmCap[i];
		lvc.cx=rc.right/4;
		SendMessage(hsList,LVM_INSERTCOLUMN,i,(LPARAM)&lvc);
	}
}

void PropU()
{
	if (!ActiveU) return;
	if (bHumanMode)
	{
		UnitProperty((LPNOTEUNIT)ActiveU);
	}
	else 
	{
		FirmProperty((LPFIRMUNIT)ActiveU);
	}
	g_bUnitModified=TRUE;
	Find();
}

BOOL FAR PASCAL SearchWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		hswnd=hDlg;
		OnInit();
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam)==BN_CLICKED)
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
		case IDC_BUTTON3:
			EndDialog(hDlg,TRUE);
			return TRUE;
		case IDC_BUTTON1:
			SetWindowText(GetDlgItem(hDlg,IDC_EDIT1),NULL);
			SetWindowText(GetDlgItem(hDlg,IDC_EDIT2),NULL);
			SetWindowText(GetDlgItem(hDlg,IDC_EDIT3),NULL);
			SetWindowText(GetDlgItem(hDlg,IDC_EDIT13),NULL);
			break;
		case IDC_BUTTON2:
			Find();
			break;
		case IDC_BUTTON4:
			PropU();
			break;
		case IDC_BUTTON5:
			EndDialog(hDlg,TRUE);
			break;
		}
		break;
	case WM_NOTIFY:
		if (wParam==IDC_LIST1)
		{
			NM_LISTVIEW* lpnm;
			lpnm=(NM_LISTVIEW*)lParam;
			if (lpnm->hdr.code==LVN_ITEMCHANGED && lpnm->uChanged&LVIF_STATE) 
				UpdateButtons(lpnm);
			if (lpnm->hdr.code==NM_DBLCLK) PropU();
		}
		break;
	}
	return FALSE;
}

LPARAM Search(HWND hWnd)
{
	DialogBox(hInstance, "SEARCH", hWnd, (DLGPROC)SearchWndProc);
	UpdateAllData();
	return ActiveU;
}