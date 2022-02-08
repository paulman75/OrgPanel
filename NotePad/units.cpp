#include "windows.h"
#include "units.h"
#include <commctrl.h>
#include "../Headers/ToolBar.h"
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "Richedit.h"
#include "OldVers.h"
#include "res/resource.h"
#include "../Headers/DynList.h"
#include "../Headers/ExTreeView.h"

BOOL	Modif=FALSE;
extern HWND		hRich;
extern CToolBar*	bar;
extern HWND hwndMain;
extern char MainDir[200];
DWORD	dw;
extern HINSTANCE hInstance;		  // Global instance handle for application
char findstr[100]="";
char replace[100]="";
char buf[500];
HWND hDlg;
BOOL bDirUp;
BOOL bCase;
#define CANFINDSTR "Не могу найти текст"
WNDPROC OldEditProc,OldEditProc2;
BOOL bFindDlg;
extern CExTreeView* trView;
DynArray<TGROUP> GroupList;
TGROUP tg;
BOOL TextModif=FALSE;
int ShowedID;

void strcopy(char* s1, char* s2, int bufsize)
{
	int i=0;
	int l=strlen(s2);
	if (l>=bufsize) l=bufsize-1;
	while (i<l)
	{
		s1[i]=s2[i];
		i++;
	}
	s1[i]=0;
}

int FindGroup(int ID, LPGROUP pg)
{
	int i=0;
	while (i<GroupList.count())
	{
		*pg=GroupList[i];
		if (pg->ID==ID) return i;
		i++;
	}
	return -1;
}

void SaveInfo()
{
	if (!TextModif) return;
	int len=SendMessage(hRich,WM_GETTEXTLENGTH,0,0)+1;

	TGROUP selu;
	int index=FindGroup(ShowedID,&selu);

	if (selu.St) free(selu.St);
	selu.St=(char*)malloc(len);
	SendMessage(hRich,WM_GETTEXT,len,(LPARAM)selu.St);
	GroupList[index]=selu;
	SendMessage(hRich,EM_SETMODIFY,0,0);
	Modif=TRUE;
}

void ShowInfo(HTREEITEM hit)
{
	TextModif=FALSE;
	if (!hit) return;
	TTreeNode tn;
	trView->GetTreeItem(hit,&tn);
	TGROUP selu;
	if (FindGroup(tn.Data,&selu)==-1) return;
	if (selu.St) SetWindowText(hRich,selu.St);
	else SetWindowText(hRich,"");
	ShowedID=selu.ID;
	WriteDWordToReg("NotePadID",ShowedID);
}

void UpdateToolBar()
{
	HMENU hMenu=GetMenu(hwndMain);
	HMENU hSub=GetSubMenu(hMenu,0);
	EnableMenuItem(hSub,IDC_DEL,CanDelete() ? MF_ENABLED : MF_GRAYED);
}

LPTSTR ReadString(HANDLE hFile)
{
	WORD w;
	ReadFile(hFile,&w,2,&dw,NULL);
	if (!w) return NULL;
	LPTSTR s=(char*)malloc(w+1);
	ReadFile(hFile,s,w,&dw,NULL);
	WORD w2=0;
	while (w2<w) s[w2++]-=30;
	s[w2]=0;
	return s;
}

void LoadUnits()
{
	char* buf=(char*)malloc(400);
	lstrcpy(buf,MainDir);
	lstrcat(buf,NOTEPAD_FILE);
	HANDLE hBookFile = CreateFile(buf,GENERIC_READ, 0,0, OPEN_EXISTING,0,NULL);
	if (hBookFile!=INVALID_HANDLE_VALUE)
	{
		ReadFile(hBookFile,buf,4,&dw,NULL);
		if (buf[0]=='p' && buf[1]=='a' && buf[2]=='d')
		{
			if (buf[3]!='4') LoadOldVersion(hBookFile,buf[3]);
			else
			{
				int LoadIndex=1;
				int Count;
				ReadFile(hBookFile,&Count,4,&dw,NULL);
				while (Count--)
				{
					TGROUP tg;
					//Загружаем группу
					ReadFile(hBookFile,&tg,sizeof(TGROUP),&dw,NULL);
					tg.St=ReadString(hBookFile);
					tg.LoadIndex=LoadIndex++;
					GroupList.Add(&tg);
				}
			}
		}
		else LoadVeryOldVersion(hBookFile);
		CloseHandle(hBookFile);
	}
	if (GroupList.count()==0)
	{
		tg.Group_ID=0;
		tg.ID=1;
		tg.LoadIndex=1;
		tg.St=NULL;
		strcpy(tg.Name,"Все разделы");
		GroupList.Add(&tg);
	}
}

void WriteString(HANDLE hFile, LPTSTR s)
{
	WORD w;
	if (s) w=lstrlen(s);
	else w=0;
	WriteFile(hFile,&w,2,&dw,NULL);
	if (s)
	{
		WORD w2=0;
		while (w2<w) s[w2++]+=30;
		WriteFile(hFile,s,w,&dw,NULL);
	}
}

void SaveNode(HTREEITEM hi, HANDLE hFile)
{
	TGROUP tg;
	TTreeNode tn;
	DWORD dw;
	trView->GetTreeItem(hi,&tn);
	if (tn.ImageIndex==0)
	{
		//Сохраняем группу
		if (FindGroup(tn.Data,&tg)==-1) return;
		WriteFile(hFile,&tg,sizeof(TGROUP),&dw,NULL);
		WriteString(hFile,tg.St);
	}
}

void SaveGroup(HTREEITEM hi, HANDLE hFile)
{
	while (hi)
	{
		SaveNode(hi,hFile);
		HTREEITEM hic=trView->GetChildItem(hi);
		if (hic) SaveGroup(hic,hFile);
		hi=trView->GetNextItem(hi);
	}
}

void SaveUnits()
{
	SaveInfo();
	if (!Modif) return;
	lstrcpy(buf,MainDir);
	lstrcat(buf,NOTEPAD_FILE);
	HANDLE hFile = CreateFile(buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD dw;
		buf[0]='p';
		buf[1]='a';
		buf[2]='d';
		buf[3]='4';
		WriteFile(hFile,buf,4,&dw,NULL);
		int Count=GroupList.count();

		WriteFile(hFile,&Count,4,&dw,NULL);
		SaveGroup(trView->GetRootItem(), hFile);
		CloseHandle(hFile);
	}
	Modif=FALSE;
}

void RenameUnit()
{	
	TGROUP tg;
	int index=FindGroup(ShowedID,&tg);

	strcpy(buf,tg.Name);
	if (!InputQuery(hwndMain,"Переименование","Введите новое название",&buf[0],&buf[0],200)) return;
	
	HTREEITEM hsi=trView->GetSelItem();
	trView->SetCaption(hsi,buf);

	strcopy(tg.Name,buf,50);
	GroupList[index]=tg;
	Modif=TRUE;
}

void DelNode(HTREEITEM hi, int ID)
{
	TTreeNode tnc;
	while (trView->GetChildNode(hi,&tnc)) DelNode(tnc.hHandle, tnc.Data);
	TGROUP tg;
	int index=FindGroup(ID, &tg);
	GroupList.Del(index);
	trView->DelItem(hi);
	if (tg.St) free(tg.St);
}

void DeleteUnit()
{
	ShowInfo(NULL);
	TTreeNode tn;
	trView->GetTreeItem(trView->GetSelItem(),&tn);
	//Удаление группы
	if (MessageBox(0,"Действительно удалить раздел?","Подтверждение",MB_ICONQUESTION | MB_YESNO)==IDNO) return;
	DelNode(tn.hHandle, tn.Data);

	Modif=FALSE;
}

const byte win2koi[]={
	0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,
     0x94,0x94,0x94,0x94,0x94,0x95,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0x94,
     0x9A,0x94,0x94,0x94,0x94,0x94,0x94,0x94,0xB3,0xBF,0x94,0x94,0x94,0x94,0x94,0x94,
     0x9C,0x94,0x94,0x94,0x94,0x94,0x94,0x9E,0xA3,0x94,0x94,0x94,0x94,0x94,0x94,0x94,
     0xE1,0xE2,0xF7,0xE7,0xE4,0xE5,0xF6,0xFA,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,
     0xF2,0xF3,0xF4,0xF5,0xE6,0xE8,0xE3,0xFE,0xFB,0xFD,0xFF,0xF9,0xF8,0xFC,0xE0,0xF1,
     0xC1,0xC2,0xD7,0xC7,0xC4,0xC5,0xD6,0xDA,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,
     0xD2,0xD3,0xD4,0xD5,0xC6,0xC8,0xC3,0xDE,0xDB,0xDD,0xDF,0xD9,0xD8,0xDC,0xC0,0xD1
	};

void Koi2Win(char* s)
{
	while (*s)
	{
		byte b=*s;
		if (b>127)
		{
			byte i=0;
			while (win2koi[i]!=b && i<128) i++;
			if (i==128) i=0x3f;
			*s=i+128;
		}
		s++;
	}
}

void Win2Koi(char *s)
{
	while (*s)
	{
		byte b=*s;
		if (b>127) *s=win2koi[b-128];
		s++;
	}
}

void Win2Dos(char *s)
{
	while (*s)
	{
		byte b=*s;
		if (b>191)
		{
			if (b>239) *s=b-16;
			else *s=b-64;
		}
		s++;
	}
}

void Dos2Win(char *s)
{
	while (*s)
	{
		byte b=*s;
		if (b>127 && b<176) *s=b+64;
		if (b>223 && b<240) *s=b+16;
		s++;
	}
}

void Convert(DWORD operation, char *s)
{
	switch (operation)
	{
	case ID_WINDOS:
		Win2Dos(s);
		break;
	case ID_DOSWIN:
		Dos2Win(s);
		break;
	case ID_DOSKOI:
		Dos2Win(s);
	case ID_WINKOI:
		Win2Koi(s);
		break;
	case ID_KOIWIN:
		Koi2Win(s);
		break;
	case ID_KOIDOS:
		Koi2Win(s);
		Win2Dos(s);
		break;
	}
	TextModif=TRUE;
}

void UpdateFindBtn()
{
	int len=SendMessage(GetDlgItem(hDlg,IDC_EDIT1),WM_GETTEXTLENGTH,0,0);
	EnableWindow(GetDlgItem(hDlg,IDOK),len>0);
}

void UpdateReplaceBtn()
{
	int len=SendMessage(GetDlgItem(hDlg,IDC_EDIT1),WM_GETTEXTLENGTH,0,0);
	EnableWindow(GetDlgItem(hDlg,IDOK),len>0);
	if (len) len=SendMessage(GetDlgItem(hDlg,IDC_EDIT2),WM_GETTEXTLENGTH,0,0);
	EnableWindow(GetDlgItem(hDlg,IDC_REPLACE),len>0);
	EnableWindow(GetDlgItem(hDlg,IDC_REPLACEALL),len>0);
}

BOOL FindNext()
{
	int len=SendMessage(hRich,WM_GETTEXTLENGTH,0,0)+1;
	char *buf=(char*)malloc(len);
	SendMessage(hRich,WM_GETTEXT,len,(LPARAM)buf);
	DWORD st,en;
	SendMessage(hRich,EM_GETSEL,(WPARAM)&st,(LPARAM)&en);
	int i=-1;
	if (bDirUp)
	{
		int start=0;
		while (TRUE)
		{
			int j=FindInString(buf+start,findstr,bCase);
			if (j==-1) break;
			start+=j;
			if (start>=(int)st) break;
			i=start;
		}
	}
	else 
	{
		if (en) st++;
		i=FindInString(buf+st,findstr,bCase);
		if (i!=-1) i+=st;
	}
	free (buf);
	if (i==-1) return FALSE;
	SendMessage(hRich,EM_SETSEL,i-strlen(findstr),i);
	return TRUE;
}

BOOL Replace(BOOL bAll)
{
	DWORD st,en;
	SendMessage(hRich,EM_GETSEL,(WPARAM)&st,(LPARAM)&en);
	if (bAll)
	{
		if (st!=en) st=st+bDirUp ? 1 : -1;
		SendMessage(hRich,EM_SETSEL,(WPARAM)st,st);
		while (FindNext())
		{
			SendMessage(hRich,EM_REPLACESEL,TRUE,(LPARAM)replace);
		}
	}
	else
	{
		if (st==en) return FindNext();
		SendMessage(hRich,EM_REPLACESEL,TRUE,(LPARAM)replace);
		FindNext();
	}
	TextModif=TRUE;
	return TRUE;
}

LONG FAR PASCAL NewFindEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CHAR:
		if (wParam==VK_RETURN)
		{
			if (SendMessage(hwnd,WM_GETTEXTLENGTH,0,0))
			PostMessage(GetParent(hwnd),WM_COMMAND,(BN_CLICKED<<16)+IDOK,0);
			return 0;
		}
		if (wParam==VK_TAB)
		{
			SetFocus(GetDlgItem(GetParent(hwnd),bFindDlg ? IDOK:IDC_EDIT2));
			return 0;
		}
		break;
	}
    return(CallWindowProc(OldEditProc,hwnd, msg, wParam, lParam));
}

BOOL FAR PASCAL FindDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		OldEditProc=(WNDPROC)SetWindowLong(GetDlgItem(hDlg,IDC_EDIT1),GWL_WNDPROC,(long)NewFindEditProc);
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam)==EN_CHANGE) UpdateFindBtn();
		if (HIWORD(wParam)==BN_CLICKED)
		switch (LOWORD(wParam))
		{
		case IDOK:
			SendMessage(GetDlgItem(hDlg,IDC_EDIT1),WM_GETTEXT,100,(LPARAM)findstr);
			bDirUp=SendMessage(GetDlgItem(hDlg,IDC_RADIO1),BM_GETCHECK,0,0)==BST_CHECKED;
			bCase=SendMessage(GetDlgItem(hDlg,IDC_CHECK1),BM_GETCHECK,0,0)==BST_CHECKED;
			if (!FindNext()) MessageBox(hDlg,CANFINDSTR,"Поиск",MB_OK | MB_ICONEXCLAMATION);
			break;
		case IDCANCEL:
			EndDialog(hDlg,TRUE);
			return TRUE;
		}
	}
	return FALSE;
}

LONG FAR PASCAL NewReplaceEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CHAR:
		if (wParam==VK_RETURN)
		{
			if (SendMessage(GetDlgItem(GetParent(hwnd),IDC_EDIT1),WM_GETTEXTLENGTH,0,0)
			&& SendMessage(hwnd,WM_GETTEXTLENGTH,0,0))
			PostMessage(GetParent(hwnd),WM_COMMAND,(BN_CLICKED<<16)+IDC_REPLACE,0);
			return 0;
		}
		if (wParam==VK_TAB)
		{
			SetFocus(GetDlgItem(GetParent(hwnd),IDOK));
			return 0;
		}
		break;
	}
    return(CallWindowProc(OldEditProc2,hwnd, msg, wParam, lParam));
}

BOOL FAR PASCAL ReplaceDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		OldEditProc=(WNDPROC)SetWindowLong(GetDlgItem(hDlg,IDC_EDIT1),GWL_WNDPROC,(long)NewFindEditProc);
		OldEditProc2=(WNDPROC)SetWindowLong(GetDlgItem(hDlg,IDC_EDIT2),GWL_WNDPROC,(long)NewReplaceEditProc);
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam)==EN_CHANGE) UpdateReplaceBtn();
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDC_REPLACE:
		case IDC_REPLACEALL:
			SendMessage(GetDlgItem(hDlg,IDC_EDIT1),WM_GETTEXT,100,(LPARAM)findstr);
			SendMessage(GetDlgItem(hDlg,IDC_EDIT2),WM_GETTEXT,100,(LPARAM)replace);
			bDirUp=SendMessage(GetDlgItem(hDlg,IDC_RADIO1),BM_GETCHECK,0,0)==BST_CHECKED;
			bCase=SendMessage(GetDlgItem(hDlg,IDC_CHECK1),BM_GETCHECK,0,0)==BST_CHECKED;
			if (LOWORD(wParam)==IDOK)
			{
				if (!FindNext()) MessageBox(hDlg,CANFINDSTR,"Поиск",MB_OK | MB_ICONEXCLAMATION);
			}
			else if (!Replace(LOWORD(wParam)==IDC_REPLACEALL)) MessageBox(hDlg,CANFINDSTR,"Замена",MB_OK | MB_ICONEXCLAMATION);
			break;
		case IDCANCEL:
			EndDialog(hDlg,TRUE);
			return TRUE;
		}
	}
	return FALSE;
}

void Find(DWORD operation)
{
	DWORD st,en;
	SendMessage(hRich,EM_GETSEL,(WPARAM)&st,(LPARAM)&en);
	if (st!=en)
	{
		if (en-st>99) en=st+99;
		TEXTRANGE tr;
		tr.chrg.cpMin=st;
		tr.chrg.cpMax=en;
		tr.lpstrText=findstr;
		findstr[0]=0;
		SendMessage(hRich,EM_GETTEXTRANGE,0,(LPARAM)&tr);
		int i=0;
		while (i<99 && findstr[i] && findstr[i]>31) i++;
		findstr[i]=0;
	}
	if (!findstr[0] && operation==ID_SEARCHNEXT) operation=ID_SEARCH;
	if (operation==ID_SEARCHNEXT)
	{
		if (!FindNext()) MessageBox(hDlg,CANFINDSTR,"Поиск",MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	if (operation==ID_SEARCH)
	{
		bFindDlg=TRUE;
		hDlg=CreateDialog(hInstance,"FINDDLG",hwndMain,FindDlgProc);
		SendMessage(GetDlgItem(hDlg,IDC_RADIO2),BM_SETCHECK,1,0);
		SendMessage(GetDlgItem(hDlg,IDC_EDIT1),WM_SETTEXT,0,(LPARAM)findstr);
		SendMessage(GetDlgItem(hDlg,IDC_EDIT1),EM_SETSEL,0,-1);
		UpdateFindBtn();
		ShowWindow(hDlg,SW_SHOW);
	}
	else
	{
		bFindDlg=FALSE;
		hDlg=CreateDialog(hInstance,"REPLACEDLG",hwndMain,ReplaceDlgProc);
		SendMessage(GetDlgItem(hDlg,IDC_RADIO2),BM_SETCHECK,1,0);
		SendMessage(GetDlgItem(hDlg,IDC_EDIT1),WM_SETTEXT,0,(LPARAM)findstr);
		SendMessage(GetDlgItem(hDlg,IDC_EDIT1),EM_SETSEL,0,-1);
		SendMessage(GetDlgItem(hDlg,IDC_EDIT2),WM_SETTEXT,0,(LPARAM)replace);
		SendMessage(GetDlgItem(hDlg,IDC_EDIT2),EM_SETSEL,0,-1);
		UpdateReplaceBtn();
		ShowWindow(hDlg,SW_SHOW);
	}
}

int GetActiveGroup()
{
	int groupID=1;
	HTREEITEM hsi=trView->GetSelItem();
	if (hsi)
	{
		TTreeNode tn;
		trView->GetTreeItem(hsi,&tn);
		while (tn.hHandle)
		{
			if (tn.ImageIndex==0)
			{
				groupID=tn.Data;
				break;
			}
			if (!trView->GetParentNode(&tn,&tn)) break;
		}
	}
	return groupID;
}


int GetNextGroupID()
{
	int max=0;
	int i=0;
	while (i<GroupList.count())
	{
		TGROUP tg=GroupList[i];
		if (tg.ID>max) max=tg.ID;
		i++;
	}
	return max+1;
}

void FillTree()
{
	trView->Clear();
	int iu=0;
	int ig=0;
	int LoadIndex=1;
	while (ig<GroupList.count())
	{
		if (ig<GroupList.count())
		{
			tg=GroupList[ig];
			if (tg.LoadIndex==LoadIndex)
			{
				TTreeNode tn;
				PTreeNode ptn=NULL;
				if (tg.Group_ID)
				{
					if (trView->FindNode(0,tg.Group_ID,&tn))
					ptn=&tn;
				}
				trView->AddChildNode(ptn,tg.Name,tg.ID,0,0);
				LoadIndex++;
				ig++;
			}
		}
	}
}

void AddGroup()
{
	SaveInfo();
	buf[0]=0;
	if (!InputQuery(hwndMain,"Добавление группы","Введите название группы","",buf,200)) return;
	int groupID=GetActiveGroup();
	TGROUP newg;
	newg.ID=GetNextGroupID();
	newg.Group_ID=groupID;
	newg.St=NULL;
	strcopy(newg.Name,buf,50);

	TTreeNode tn;
	trView->FindNode(0,groupID,&tn);	
	tn=trView->AddChildNode(&tn,newg.Name,newg.ID,0,0);
	trView->SelItem(tn.hHandle);
	GroupList.Add(&newg);
	ShowInfo(tn.hHandle);
	Modif=TRUE;
}

void OnDropped(int ID1, int ID2)
{
	int index2;
	TGROUP tg2;
	index2=FindGroup(ID1,&tg2);
	if (index2==-1) return;
	
	tg2.Group_ID=ID2;
	GroupList[index2]=tg2;
	Modif=TRUE;
}

BOOL CanDelete()
{
	return ShowedID!=1;
}
