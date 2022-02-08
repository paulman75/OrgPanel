#include "windows.h"
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "ListView.h"
#include "units.h"
#include "Column.h"
#include "res/resource.h"
#include "../Headers/toolbar.h"
#include "EditUnit.h"
#include "EditFirm.h"
#include "Filter.h"
#include "ToDnev.h"
#include "oldVers.h"
#include "Group.h"

extern	CSortListView*	cList;
extern	HINSTANCE hInstance;
extern	char MainDir[200];
extern	DWORD	dw;
extern	CToolBar*		bar;
extern	BOOL	bHumanMode;
extern  HWND hwndMain;
extern  HWND hStatusBar;
extern  BOOL GroupChanged;

LPNOTEUNIT	FirstUnit;
LPNOTEUNIT	ActiveUnit=NULL;
LPNOTEUNIT	nNote;
LPFIRMUNIT	FirstFirm;
LPFIRMUNIT	ActiveFirm=NULL;
LPFIRMUNIT	nfNote;
BOOL		g_bUnitModified;
const		DWORD EndFlag=0xffffffff;

extern  COLUMN	Columns[UN_COUNT];
extern  COLUMN	FirmCol[UNF_COUNT];
extern 	int		iOldOrder[UN_COUNT];
extern  DWORD	iSortUnit;
extern  DWORD	iSortFirm;
extern  DWORD	FilterGroup;
extern  HWND	hFilterEdit[4];

int CompareStrings(LPTSTR s,LPTSTR s2)
{
	if (!s && !s2) return 0;
	if (!s) return 1;
	if (!s2) return -1;
	return lstrcmpi(s,s2);
}

LPFIRMUNIT FindFirm(DWORD k)
{
	LPFIRMUNIT nf=FirstFirm;
	while (nf)
	{
		if (nf->Key==k) return nf;
		nf=nf->Next;
	}
	return NULL;
}

int MainCompareFunc(LPARAM lParam1, LPARAM lParam2, BOOL bsd, int isc)
{
	int iIndex=iOldOrder[isc];
	if (iIndex==-1) return 0;
	int res;
	if (bHumanMode)
	{
		nNote=(LPNOTEUNIT)lParam1;
		LPNOTEUNIT nNote2=(LPNOTEUNIT)lParam2;
		if (!iIndex) iIndex=2;
		switch (iIndex)
		{
		case UN_BDATE:
			res=nNote->BDate.Day-nNote2->BDate.Day+40*(nNote->BDate.Month-nNote2->BDate.Month)+1000*(nNote->BDate.Year-nNote2->BDate.Year);
			if (res<0 || !nNote->BDate.Month) res=-1;
			if (res>0 || !nNote2->BDate.Month) res=1;
			break;
		case  UN_SEX:
			if (nNote->Sex==nNote2->Sex) res=0;
			else res=nNote->Sex>nNote2->Sex ? -1 : 1;
			break;
		case UN_FIRM:
			if (nNote->Firm==0xffffffff && nNote2->Firm==0xffffffff) res=0;
			else
			{
				res=0;
				if (nNote->Firm==0xffffffff) res=1;
				if (nNote2->Firm==0xffffffff) res=-1;
				if (!res)
				{
					nfNote=FindFirm(nNote->Firm);
					LPFIRMUNIT nf2=FindFirm(nNote2->Firm);
					res=CompareStrings(nfNote->Fields[UNF_NAME],nf2->Fields[UNF_NAME]);
				}
			}
			break;
		case UN_GROUP:
			char* t;
			char* t2;
			t=GetGroupName(nNote->Group);
			t2=GetGroupName(nNote2->Group);
			res=0;
			if (t && !t2) res=-1;
			if (!t && t2) res=1;
			if (t && t2) res=CompareStrings(t,t2);
			break;
		default: res=CompareStrings(nNote->Fields[iIndex],nNote2->Fields[iIndex]);
		}
	}
	else
	{
		nfNote=(LPFIRMUNIT)lParam1;
		LPFIRMUNIT nfNote2=(LPFIRMUNIT)lParam2;
		res=CompareStrings(nfNote->Fields[iIndex],nfNote2->Fields[iIndex]);
	}
	if (!bsd) res=1-res;
	return res;
}

void UpdateToolBar()
{
	BOOL  b=(ActiveUnit && bHumanMode) || (ActiveFirm && !bHumanMode);
	bar->EnableButton(IDC_CHANGE,b);
	bar->EnableButton(IDC_DELETE,b);
	HMENU hMenu=GetMenu(hwndMain);
	HMENU hSub=GetSubMenu(hMenu,1);
	EnableMenuItem(hSub,IDC_CHANGE,b ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hSub,IDC_DELETE,b ? MF_ENABLED : MF_GRAYED);
	hSub=GetSubMenu(hMenu,2);
	EnableActionMenu(hSub);
}

void SetActiveUnit(NM_LISTVIEW* nm)
{
	if (!(nm->uNewState&LVIS_SELECTED || nm->uOldState&LVIS_SELECTED)) return;
	if (bHumanMode)
	{
		if (!(nm->uNewState&LVIS_SELECTED)) ActiveUnit=NULL;
		else
			ActiveUnit=(LPNOTEUNIT)nm->lParam;
	}
	else
	{
		if (!(nm->uNewState&LVIS_SELECTED)) ActiveFirm=NULL;
		else
			ActiveFirm=(LPFIRMUNIT)nm->lParam;
	}
	UpdateToolBar();
}

void SetItemSelect(LPARAM nu)
{
	LPARAM lParam;
	if (bHumanMode) lParam=(LPARAM)ActiveUnit;
	else lParam=(LPARAM)ActiveFirm;
	LV_ITEM lv;
	lv.mask=LVIF_STATE;
	lv.iSubItem=0;
	lv.stateMask=LVIS_SELECTED | LVIS_FOCUSED;
	int i=cList->FindDataIndex(lParam);
	if (i!=-1)
	{
		lv.iItem=i;
		lv.state=0;
		SendMessage(cList->GetWnd(),LVM_SETITEM,0,(LPARAM)&lv);
	}
	if (nu) i=cList->FindDataIndex(nu);
	else i=0;
	if (i!=-1)
	{
		lv.iItem=i;
		lv.state=lv.stateMask;
		SendMessage(cList->GetWnd(),LVM_SETITEM,0,(LPARAM)&lv);
	}
	SendMessage(cList->GetWnd(),LVM_ENSUREVISIBLE,i,0);
	SetFocus(cList->GetWnd());
}

void AddFirmToList(LPFIRMUNIT nf)
{
	int c=SendMessage(cList->GetWnd(),LVM_GETITEMCOUNT,0,0);
	int sub=0;
	for (int i=0; i<UNF_COUNT; i++)
	{
		int j=0;
		while (j<UNF_COUNT && FirmCol[j].iOrder!=i) j++;
		if (j<UNF_COUNT)
		if (FirmCol[j].bVisible)
			cList->AddText(nf->Fields[j],c,sub++,(LPARAM)nf,-1);
	}
}

void MakeUnitName(LPNOTEUNIT nu,LPTSTR s)
{
	MakeShortName(nu->Fields[UN_NAME],nu->Fields[UN_MNAME],nu->Fields[UN_SNAME],s);
}

int DaysBeetwen(PDate d1, PDate d2)
{
	TDate dd=*d1;
	d2->Year=dd.Year;
	int en=EncodeDate(&dd)-EncodeDate(d2);
	if (en<0)
	{
		dd.Year++;
		en=EncodeDate(&dd)-EncodeDate(d2);
	}
	return en;
}

void AddUnitToList(LPNOTEUNIT nu)
{
	int c=SendMessage(cList->GetWnd(),LVM_GETITEMCOUNT,0,0);
	int sub=0;
	for (int i=0; i<UN_COUNT; i++)
	{
		int j=0;
		while (j<UN_COUNT && Columns[j].iOrder!=i) j++;
		if (j<UN_COUNT)
		if (Columns[j].bVisible)
		{
			char* s=(char*)malloc(1000);
			s[0]=0;
			if (j==UN_NAME)
			{
				int iImage=-1;
				if (nu->BDate.Month)
				{
					TDate tod=Now();
					int en=DaysBeetwen(&nu->BDate,&tod);
					if (en>=0 && en<15) iImage=0;
					if (nu->Fields[UN_NAME] && iImage==-1)
					{
						int NameIndex=GetNameIndex(nu->Fields[UN_NAME]);
						if (NameIndex!=-1)
						{
							TDate imen;
							if (GetHumanMainImenDate(NameIndex,&nu->BDate,&imen))
							{
								imen.Year=tod.Year;
								en=DaysBeetwen(&imen,&tod);
								if (en>=0 && en<15) iImage=1;
							}
						}
					}
				}
				MakeUnitName(nu,s);
				cList->AddText(s,c,sub++,(LPARAM)nu,iImage);
			}
			else
			{
				switch (j)
				{
				case UN_BDATE:
					if (nu->BDate.Month!=0) DateToString(&nu->BDate,s);
					break;
				case UN_SEX:
					if (nu->Sex==1) lstrcpy(s,"муж");
					if (nu->Sex==2) lstrcpy(s,"жен");
					break;
				case UN_FIRM:
					if (nu->Firm!=0xffffffff)
					{
						nfNote=FindFirm(nu->Firm);
						if (nfNote->Fields[UNF_NAME]) lstrcpy(s,nfNote->Fields[UNF_NAME]);
					}
					break;
				case UN_GROUP:
					char *t;
					t=GetGroupName(nu->Group);
					if (t) lstrcpy(s,t);
					break;
				default:	lstrcpy(s,nu->Fields[j]);
				}
				cList->AddText(s,c,sub++,0,0);
			}
			free(s);
		}
	}
}

void DeleteNote()
{
	if (bHumanMode)
	{
		if (!ActiveUnit) return;
		if (MessageBox(0,"Действительно удалить данные о человеке?","Подтверждение",MB_ICONQUESTION | MB_YESNO)==IDNO) return;
		if (ActiveUnit==FirstUnit) FirstUnit=ActiveUnit->Next;
		else
		{
			nNote=FirstUnit;
			while (nNote->Next!=ActiveUnit) nNote=nNote->Next;
			nNote->Next=ActiveUnit->Next;
		}
		LPNOTEUNIT nu=ActiveUnit;
		cList->DeleteNote((LPARAM)ActiveUnit);
		for (int i=0; i<UN_BDATE; i++)
			if (nu->Fields[i]) free(nu->Fields[i]);
		free(nu);
		OnInitExchange();
	}
	else
	{
		if (!ActiveFirm) return;
		if (MessageBox(0,"Действительно удалить данные о фирме?","Подтверждение",MB_ICONQUESTION | MB_YESNO)==IDNO) return;
		if (ActiveFirm==FirstFirm) FirstFirm=ActiveFirm->Next;
		else
		{
			nfNote=FirstFirm;
			while (nfNote->Next!=ActiveFirm) nfNote=nfNote->Next;
			nfNote->Next=ActiveFirm->Next;
		}
		LPFIRMUNIT nf=ActiveFirm;
		cList->DeleteNote((LPARAM)ActiveFirm);
		for (int i=0; i<UNF_COUNT; i++)
			if (nf->Fields[i]) free(nf->Fields[i]);
		nNote=FirstUnit;
		while (nNote)
		{
			if (nNote->Firm==nf->Key) nNote->Firm=0xffffffff;
			nNote=nNote->Next;
		}
		free(nf);
	}
	UpdateToolBar();
	g_bUnitModified=TRUE;
}

void ChangeNote()
{
	if (bHumanMode)
	{
		GroupChanged=FALSE;
		if (!ActiveUnit) return;
		LPNOTEUNIT nu=UnitProperty(ActiveUnit);
		if (!nu) return;
		if (GroupChanged) UpdateAllData();
		else
		{
			cList->DeleteNote((LPARAM)nu);
			AddUnitToList(nu);
			cList->ReSort();
		}
		SetItemSelect((LPARAM)nu);
		OnInitExchange();
	}
	else 
	{
		if (!ActiveFirm) return;
		LPFIRMUNIT nf=FirmProperty(ActiveFirm);
		if (!nf) return;
		cList->DeleteNote((LPARAM)nf);
		AddFirmToList(nf);
		cList->ReSort();
		SetItemSelect((LPARAM)nf);
	}
	g_bUnitModified=TRUE;
	SetFocus(cList->GetWnd());
}

void AddHuman()
{
	LPNOTEUNIT nn=UnitProperty(NULL);
	if (!nn) return;
	g_bUnitModified=TRUE;
	if (FirstUnit) nn->Key=FirstUnit->Key+1;
	nn->Next=FirstUnit;
	FirstUnit=nn;
	if (!CheckUnitFilter(nn)) return;
	AddUnitToList(nn);
	cList->ReSort();
	SetItemSelect((LPARAM)nn);
	OnInitExchange();
}

void AddFirm()
{
	DWORD dwNewKey=0;
	if (FirstFirm) dwNewKey=FirstFirm->Key+1;
	LPFIRMUNIT nf=FirmProperty(NULL,dwNewKey);
	if (!nf) return;
	g_bUnitModified=TRUE;
	if (FirstFirm) nf->Key=FirstFirm->Key+1;
	nf->Next=FirstFirm;
	FirstFirm=nf;
	if (!CheckFirmFilter(nf)) return;
	AddFirmToList(nf);
	cList->ReSort();
	SetItemSelect((LPARAM)nf);
}

void EnableItem(void* act, LPTSTR s, int ID, HMENU hSub)
{
	BOOL b=(act!=NULL && s!=NULL);
	if (b) b=lstrlen(s)>4;
	EnableMenuItem(hSub,ID,b ? MF_ENABLED : MF_GRAYED);
}

void EnableActionMenu(HMENU hSub)
{
	if (ActiveUnit && bHumanMode)
	{
		EnableItem(ActiveUnit,ActiveUnit->Fields[UN_HOMETEL],IDC_HOMERING,hSub);
		EnableItem(ActiveUnit,ActiveUnit->Fields[UN_WORKTEL],IDC_WORKRING,hSub);
		EnableItem(ActiveUnit,ActiveUnit->Fields[UN_HOMEPAGE],IDC_HOMEPAGE,hSub);
		EnableItem(ActiveUnit,ActiveUnit->Fields[UN_EMAIL],IDC_SENDEMAIL,hSub);
		EnableMenuItem(hSub,IDC_PRINT,ActiveUnit ? MF_ENABLED : MF_GRAYED);
		return;
	}
	if (ActiveFirm && !bHumanMode)
	{
		EnableItem(ActiveFirm,ActiveFirm->Fields[UNF_TEL1],IDC_HOMERING,hSub);
		EnableItem(ActiveFirm,ActiveFirm->Fields[UNF_TEL2],IDC_WORKRING,hSub);
		EnableItem(ActiveFirm,ActiveFirm->Fields[UNF_HOMEPAGE],IDC_HOMEPAGE,hSub);
		EnableItem(ActiveFirm,ActiveFirm->Fields[UNF_EMAIL],IDC_SENDEMAIL,hSub);
		EnableMenuItem(hSub,IDC_PRINT,ActiveFirm ? MF_ENABLED : MF_GRAYED);
		return;
	}
	EnableMenuItem(hSub,IDC_PRINT,MF_GRAYED);
	EnableMenuItem(hSub,IDC_HOMERING,MF_GRAYED);
	EnableMenuItem(hSub,IDC_WORKRING,MF_GRAYED);
	EnableMenuItem(hSub,IDC_HOMEPAGE,MF_GRAYED);
	EnableMenuItem(hSub,IDC_SENDEMAIL,MF_GRAYED);
}

void SortHuman()
{
	nNote=FirstUnit;
	BOOL b=FALSE;
	//Ищем запись в которой ключ меньше ключа в следующей записи
	DWORD dwKey=1;
	while (nNote)
	{
		dwKey++;
		if (!nNote->Next) break;
		if (nNote->Key<nNote->Next->Key) b=TRUE;
		nNote=nNote->Next;
	}
	if (!b) return;

	nNote=FirstUnit;
	while (nNote)
	{
		nNote->Key=dwKey--;
		nNote=nNote->Next;
	}
	g_bUnitModified=TRUE;
}

void SortFirm()
{
	nfNote=FirstFirm;
	BOOL b=FALSE;
	//Ищем запись в которой ключ меньше ключа в следующей записи
	DWORD dwKey=1;
	while (nfNote)
	{
		dwKey++;
		if (!nfNote->Next) break;
		if (nfNote->Key<nfNote->Next->Key) b=TRUE;
		nfNote=nfNote->Next;
	}
	if (!b) return;
	//Предварительно изменяем ключи у людей чтобы потом не было
	//повторных изменений
	nNote=FirstUnit;
	while (nNote)
	{
		if (nNote->Firm!=-1) nNote->Firm+=100000;
		nNote=nNote->Next;
	}

	nfNote=FirstFirm;
	while (nfNote)
	{
		nNote=FirstUnit;
		while (nNote)
		{
			if (nNote->Firm-100000==nfNote->Key) nNote->Firm=dwKey;
			nNote=nNote->Next;
		}
		nfNote->Key=dwKey--;
		nfNote=nfNote->Next;
	}
	g_bUnitModified=TRUE;
}

LPTSTR ReadField(HANDLE hFile)
{
	WORD l;
	ReadFile(hFile,&l,2,&dw,NULL);
	if (!l) return NULL;
	LPTSTR s=(LPTSTR)malloc(l+1);
	ReadFile(hFile,s,l,&dw,NULL);
	s[l]=0;
	return s;
}

void LoadUnits()
{
	FirstUnit=NULL;
	FirstFirm=NULL;
	ActiveUnit=NULL;
	g_bUnitModified=FALSE;
	char* temp=(char*)malloc(220);
	lstrcpy(temp,MainDir);
	lstrcat(temp,NOTEBOOK_FILE);
	HANDLE hNoteFile=CreateFile(temp,GENERIC_READ, 0,0, OPEN_EXISTING,0,NULL);
	if (hNoteFile!=INVALID_HANDLE_VALUE)
	{
		ReadFile(hNoteFile,temp,5,&dw,NULL);
		if (temp[0]!='B' || temp[1]!='o' || temp[2]!='o' || temp[3]!='k')
		{
			MessageBox(hwndMain,"Файл не является файлом записной книжки","Ошибка",MB_ICONSTOP);
			CloseHandle(hNoteFile);
			return;
		}
		byte ver=temp[4];
		if (ver==51 || ver==31)
		{
			DWORD key;
			LPNOTEUNIT Prev=NULL;
			while (TRUE) 
			{
				ReadFile(hNoteFile,&key,4,&dw,NULL);
				if (key==EndFlag || !dw) break;
				nNote=(LPNOTEUNIT)malloc(sizeof(TNOTEUNIT));
				nNote->Key=key;
				if (Prev) Prev->Next=nNote;
				else FirstUnit=nNote;
				Prev=nNote;
				nNote->Next=NULL;
				ReadFile(hNoteFile,&nNote->BDate,sizeof(TDate),&dw,NULL);
				ReadFile(hNoteFile,&nNote->Firm,4,&dw,NULL);
				ReadFile(hNoteFile,&nNote->Sex,2,&dw,NULL);
				if (ver==31) ReadFile(hNoteFile,&nNote->Group,4,&dw,NULL);
				else nNote->Group=-1;
				for (int i=0; i<UN_BDATE; i++)
					nNote->Fields[i]=ReadField(hNoteFile);
			}
			LPFIRMUNIT Prevf=NULL;
			while (TRUE)
			{
				ReadFile(hNoteFile,&key,4,&dw,NULL);
				if (key==EndFlag || !dw) break;
				nfNote=(LPFIRMUNIT)malloc(sizeof(TFIRMUNIT));
				nfNote->Key=key;
				if (Prevf) Prevf->Next=nfNote;
				else FirstFirm=nfNote;
				nfNote->Next=NULL;
				Prevf=nfNote;
				for (int i=0; i<UNF_COUNT; i++)
					nfNote->Fields[i]=ReadField(hNoteFile);
			}
			LoadGroups(hNoteFile);
		}
		else
		{
			if (temp[4]=='2' || temp[4]=='1')
			{
				LoadOldVersion(hNoteFile,temp[4]-'0');
			}
			else MessageBox(hwndMain,"Файл создан в другой версии программы","Ошибка",MB_ICONSTOP);
		}
		CloseHandle(hNoteFile);
	}
	free(temp);
	SortHuman();
	SortFirm();
}

void WriteField(HANDLE hFile, LPCTSTR s)
{
	WORD l=0;
	if (s) l=lstrlen(s);
	WriteFile(hFile,&l,2,&dw,NULL);
	if (l) WriteFile(hFile,s,l,&dw,NULL);
}

void SaveUnits()
{
	if (!g_bUnitModified) return;
	char* temp=(char*)malloc(220);
	lstrcpy(temp,MainDir);
	lstrcat(temp,NOTEBOOK_FILE);
	HANDLE hNoteFile=CreateFile(temp,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hNoteFile!=INVALID_HANDLE_VALUE)
	{
		temp[0]='B';
		temp[1]='o';
		temp[2]='o';
		temp[3]='k';
		temp[4]=31;
		WriteFile(hNoteFile,temp,5,&dw,NULL);
		nNote=FirstUnit;
		while (nNote)
		{
			WriteFile(hNoteFile,&nNote->Key,4,&dw,NULL);
			WriteFile(hNoteFile,&nNote->BDate,sizeof(TDate),&dw,NULL);
			WriteFile(hNoteFile,&nNote->Firm,4,&dw,NULL);
			WriteFile(hNoteFile,&nNote->Sex,2,&dw,NULL);
			WriteFile(hNoteFile,&nNote->Group,4,&dw,NULL);
			for (int i=0; i<UN_BDATE; i++)
				WriteField(hNoteFile, nNote->Fields[i]);
			nNote=nNote->Next;
		}
		WriteFile(hNoteFile,&EndFlag,4,&dw,NULL);
		nfNote=FirstFirm;
		while (nfNote)
		{
			WriteFile(hNoteFile,&nfNote->Key,4,&dw,NULL);
			for (int i=0; i<UNF_COUNT; i++)
				WriteField(hNoteFile, nfNote->Fields[i]);
			nfNote=nfNote->Next;
		}
		WriteFile(hNoteFile,&EndFlag,4,&dw,NULL);
		SaveGroups(hNoteFile);
		CloseHandle(hNoteFile);
	}
	free(temp);
	g_bUnitModified=FALSE;
}

void UpdateAllData()
{
	LoadGroupToCombo(hFilterEdit[3],&FilterGroup);
	cList->BeginUpdate();
	cList->ClearAllItems();
	UpdateColumns();
	int iCountAll=0;
	int iView=0;
	int i;
	if (bHumanMode)
	{
		nNote=FirstUnit;
		while (nNote)
		{
			iCountAll++;
			if (CheckUnitFilter(nNote))
			{
				iView++;
				AddUnitToList(nNote);
			}
			nNote=nNote->Next;
		}
		i=iSortUnit;
	}
	else
	{
		nfNote=FirstFirm;
		while (nfNote)
		{
			iCountAll++;
			if (CheckFirmFilter(nfNote))
			{
				AddFirmToList(nfNote);
				iView++;
			}
			nfNote=nfNote->Next;
		}
		i=iSortFirm;
	}
	int j=0;
	while (iOldOrder[j]!=(i & 0x7fff)) j++;
	cList->SetSortInfo(j, (iSortUnit & 0x8000)!=0 );
	cList->ReSort();
	cList->EndUpdate();
	char* temp=(char*)malloc(100);
	wsprintf(temp,"Показано записей: %d",iView);
	SendMessage(hStatusBar,SB_SETTEXT,0,(LPARAM)temp);
	wsprintf(temp,"Всего записей: %d",iCountAll);
	SendMessage(hStatusBar,SB_SETTEXT,1,(LPARAM)temp);
	free(temp);
	UpdateToolBar();
}

void FillBioCombo(HWND hwnd)
{
	nNote=FirstUnit;
	int i;
	BOOL bSet=FALSE;
	char* temp=(char*)malloc(200);
	while (nNote)
	{
		MakeUnitName(nNote,temp);
		i=SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)temp);
		SendMessage(hwnd,CB_SETITEMDATA,i,(LPARAM)&nNote->BDate);
		if (nNote==ActiveUnit)
		{
			SendMessage(hwnd,CB_SETCURSEL,i,0);
			bSet=TRUE;
		}
		nNote=nNote->Next;
	}
	if (!bSet) SendMessage(hwnd,CB_SETCURSEL,0,0);
}

void DeleteUnits()
{
	while (FirstUnit)
	{
		for (int i=0; i<UN_BDATE; i++)
			if (FirstUnit->Fields[i]) free(FirstUnit->Fields[i]);
		nNote=FirstUnit->Next;
		free(FirstUnit);
		FirstUnit=nNote;
	}
	while (FirstFirm)
	{
		for (int i=0; i<UNF_COUNT; i++)
			if (FirstFirm->Fields[i]) free(FirstFirm->Fields[i]);
		nfNote=FirstFirm->Next;
		free(FirstFirm);
		FirstFirm=nfNote;
	}
}
