#include <windows.h>
#include "Group.h"
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "units.h"
#include "res/resource.h"

LPGROUPUNIT Gr=NULL;
LPGROUPUNIT gp;
DWORD Selected;
HWND hGroupDlg;
BOOL GroupChanged;

extern HINSTANCE hInstance;		  // Global instance handle for application

void OnSelected()
{
	HWND hList=GetDlgItem(hGroupDlg,IDC_LIST1);
	int seli=SendMessage(hList,LB_GETCURSEL,0,0);
	Selected=SendMessage(hList,LB_GETITEMDATA,seli,0);
	EnableWindow(GetDlgItem(hGroupDlg,IDC_BUTGRDEL), Selected!=0xffffffff);
	EnableWindow(GetDlgItem(hGroupDlg,IDC_BUTCHANGE), Selected!=0xffffffff);
}

void UpdateGroupList()
{
	HWND hList=GetDlgItem(hGroupDlg,IDC_LIST1);
	SendMessage(hList,LB_RESETCONTENT,0,0);
	int in=SendMessage(hList,LB_ADDSTRING,0,(LPARAM)"_не задана_");
	SendMessage(hList,LB_SETITEMDATA,in,-1);
	gp=Gr;
	int Count=1;
	while (gp)
	{
		int i=SendMessage(hList,LB_ADDSTRING,0,(LPARAM)gp->Name);
		SendMessage(hList,LB_SETITEMDATA,i,(LPARAM)gp->Key);
		gp=gp->Next;
		Count++;
	}
	int i=0;
	while (i<Count)
	{
		in=SendMessage(hList,LB_GETITEMDATA,i,0);
		if ((DWORD)in==Selected)
		{
			SendMessage(hList,LB_SETCURSEL,i,0);
			break;
		}
		i++;
	}
	OnSelected();
}

void OnChangeClick()
{
	gp=Gr;
	while (gp)
	{
		if (gp->Key==Selected) break;
		gp=gp->Next;
	}
	if (!gp) return;

	char* buf=(char*)malloc(100);
	if (!InputQuery(hGroupDlg,"Группа","Введите новое название группы",gp->Name,buf,100))
	{
		free(buf);
		return;
	}
	free(gp->Name);
	gp->Name=(char*)malloc(strlen(buf)+1);
	strcpy(gp->Name,buf);
	free(buf);
	UpdateGroupList();
	GroupChanged=TRUE;
}

void OnGrDelClick()
{
	gp=Gr;
	LPGROUPUNIT gpr=NULL;
	while (gp)
	{
		if (gp->Key==Selected) break;
		gpr=gp;
		gp=gp->Next;
	}
	if (!gp) return;
	if (gpr) gpr->Next=gp->Next;
	else Gr=gp->Next;
	free(gp);
	Selected=-1;
	UpdateGroupList();
	GroupChanged=TRUE;
}

void OnAddGroup()
{
	char* buf=(char*)malloc(100);
	if (!InputQuery(hGroupDlg,"Новая группа","Введите название группы",NULL,buf,100))
	{
		free(buf);
		return;
	}
	LPGROUPUNIT gu=(LPGROUPUNIT)malloc(sizeof(TGROUPUNIT));
	gu->Next=NULL;
	gu->Name=(char*)malloc(strlen(buf)+1);
	strcpy(gu->Name,buf);
	free(buf);
	gp=Gr;
	if (!Gr)
	{
		gu->Key=0;
		Gr=gu;
	}
	else
	{
		gp=Gr;
		while (gp->Next) gp=gp->Next;
		gp->Next=gu;
		gu->Key=gp->Key+1;
	}
	Selected=gu->Key;
	UpdateGroupList();
	GroupChanged=TRUE;
}

BOOL FAR PASCAL GroupWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_SHOWWINDOW:
		SetFocus(GetDlgItem(hDlg,IDC_LIST1));
		break;
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		hGroupDlg=hDlg;
		UpdateGroupList();
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam)==IDOK || LOWORD(wParam)==IDCANCEL)
		{
			EndDialog(hDlg,LOWORD(wParam)==IDOK);
			return TRUE;
		}
		if (LOWORD(wParam)==IDC_BUTADD) OnAddGroup();
		if (LOWORD(wParam)==IDC_BUTCHANGE) OnChangeClick();
		if (LOWORD(wParam)==IDC_BUTGRDEL) OnGrDelClick();
		if (LOWORD(wParam)==IDC_LIST1 && HIWORD(wParam)==LBN_SELCHANGE) OnSelected();
		if (LOWORD(wParam)==IDC_LIST1 && HIWORD(wParam)==LBN_DBLCLK)
		{
			EndDialog(hDlg,TRUE);
			return TRUE;
		}
	}
	return FALSE;
}

void SelectGroup(DWORD* pIndex, HWND hOwner)
{
	Selected=*pIndex;
	if (DialogBox(hInstance, "GROUPDLG", hOwner, (DLGPROC)GroupWndProc))
		*pIndex=Selected;
}

void LoadGroups(HANDLE fil)
{
	Gr=gp=NULL;
	DWORD Count;
	DWORD dw;
	ReadFile(fil,&Count,4,&dw,NULL);
	if (dw!=4)
	{
		Count=0;
		return;
	}
	while (Count--)
	{
		LPGROUPUNIT gu=(LPGROUPUNIT)malloc(sizeof(TGROUPUNIT));
		ReadFile(fil,&gu->Key,4,&dw,NULL);
		gu->Name=ReadField(fil);
		gu->Next=NULL;
		if (!gp) Gr=gu;
		else gp->Next=gu;
		gp=gu;
	}
}

void SaveGroups(HANDLE fil)
{
	DWORD Count=0;
	gp=Gr;
	while (gp)
	{
		Count++;
		gp=gp->Next;
	}
	DWORD dw;
	WriteFile(fil,&Count,4,&dw,NULL);
	gp=Gr;
	while (gp)
	{
		WriteFile(fil,&gp->Key,4,&dw,NULL);
		WriteField(fil,gp->Name);
		gp=gp->Next;
	}
}

char* GetGroupName(DWORD Index)
{
	gp=Gr;
	while (gp)
	{
		if (gp->Key==Index) return gp->Name;
		gp=gp->Next;
	}
	return NULL;
}

void LoadGroupToCombo(HWND hCombo, DWORD *Sel)
{
	SendMessage(hCombo,CB_RESETCONTENT,0,0);
	int in=SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)"");
	SendMessage(hCombo,CB_SETITEMDATA,in,-1);
	gp=Gr;
	int Count=1;
	while (gp)
	{
		int i=SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)gp->Name);
		SendMessage(hCombo,CB_SETITEMDATA,i,(LPARAM)gp->Key);
		gp=gp->Next;
		Count++;
	}
	int i=0;
	while (i<Count)
	{
		in=SendMessage(hCombo,CB_GETITEMDATA,i,0);
		if ((DWORD)in==*Sel)
		{
			SendMessage(hCombo,CB_SETCURSEL,i,0);
			break;
		}
		i++;
	}
}