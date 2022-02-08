#include <windows.h>
#include "Units.h"
#include "res\resource.h"
#include "Utils.h"
#include "../Headers/general.h"
#include "../OrgPanel/ToolBar.h"
#include "../Headers/ExTreeView.h"
#include "../Headers/DynList.h"

BOOL	GlobModif;
BOOL	Modif;
char	buf[200];
char	buf2[230];
extern char	MainDir[200];
extern HWND hMainDlg;
extern CToolBar*	bar;
extern CExTreeView* trView;
DynArray<TUNIT> UnitList;
DynArray<TGROUP> GroupList;
TGROUP tg;
TUNIT tu;

int GetNextUnitID()
{
	int max=0;
	int i=0;
	while (i<UnitList.count())
	{
		TUNIT tu=UnitList[i];
		if (tu.ID>max) max=tu.ID;
		i++;
	}
	return max+1;
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

int FindUnit(int ID, LPUNIT pu)
{
	int i=0;
	while (i<UnitList.count())
	{
		*pu=UnitList[i];
		if (pu->ID==ID) return i;
		i++;
	}
	return -1;
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

void SaveInfo(HTREEITEM hit)
{
	if (!Modif) return;
	TTreeNode tn;
	if (hit) trView->GetTreeItem(hit,&tn);
	else trView->GetTreeItem(trView->GetSelItem(),&tn);
	TUNIT selu;
	int index=FindUnit(tn.Data,&selu);
	GetWindowText(GetDlgItem(hMainDlg,IDC_URL),selu.URL,100);
	GetWindowText(GetDlgItem(hMainDlg,IDC_LOGIN),selu.Login,30);
	GetWindowText(GetDlgItem(hMainDlg,IDC_PASS),selu.Password,20);
	GetWindowText(GetDlgItem(hMainDlg,IDC_COMMENT),selu.Comment,200);
	GetLocalTime(&selu.Time);

	UnitList[index]=selu;
	GlobModif=TRUE;
	Modif=FALSE;
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

void AddGroup()
{
	SaveInfo(NULL);
	buf[0]=0;
	if (!InputQuery(hMainDlg,"Добавление группы","Введите название группы","",buf,200)) return;
	int groupID=GetActiveGroup();
	TGROUP newg;
	newg.ID=GetNextGroupID();
	newg.Group_ID=groupID;
	strcopy(newg.Name,buf,50);

	TTreeNode tn;
	if (trView->FindNode(0,groupID,&tn))
		tn=trView->AddChildNode(&tn,newg.Name,newg.ID,0,0);
	else
		tn=trView->AddChildNode(NULL,newg.Name,newg.ID,0,0);
	trView->SelItem(tn.hHandle);
	GroupList.Add(&newg);
	ShowInfo(tn.hHandle);
	GlobModif=TRUE;
}

void AddUnit(char* name, char* url)
{
	SaveInfo(NULL);
	int groupID=GetActiveGroup();

	TUNIT newu;
	newu.ID=GetNextUnitID();
	newu.Group_ID=groupID;
	newu.Comment[0]=0;
	newu.Login[0]=0;
	strcopy(newu.Name,name,40);
	newu.Password[0]=0;
	GetLocalTime(&newu.Time);
	strcpy(newu.URL,url);
	TTreeNode tn;
	trView->FindNode(0,groupID,&tn);	
	tn=trView->AddChildNode(&tn,newu.Name,newu.ID,1,1);
	trView->SelItem(tn.hHandle);
	UnitList.Add(&newu);
	ShowInfo(tn.hHandle);
	GlobModif=TRUE;
}

void AddUnit()
{
	SaveInfo(NULL);
	buf[0]=0;
	if (!InputQuery(hMainDlg,"Добавление сайта","Введите название сайта","",buf,200)) return;
	AddUnit(buf,"http://");
}

void DelUnit(PTreeNode ptn)
{
	TUNIT tu;
	int index=FindUnit(ptn->Data, &tu);
	UnitList.Del(index);
	trView->DelItem(ptn->hHandle);
}

void Rename()
{
	TTreeNode tn;
	HTREEITEM hsi=trView->GetSelItem();
	trView->GetTreeItem(hsi,&tn);
	int index;
	TUNIT tu;
	TGROUP tg;
	if (tn.ImageIndex==1) index=FindUnit(tn.Data,&tu);
	else index=FindGroup(tn.Data,&tg);

	strcpy(buf,tn.Caption);
	if (!InputQuery(hMainDlg,"Переименование","Введите новое название",&buf[0],&buf[0],200)) return;
	
	trView->SetCaption(hsi,buf);

	if (tn.ImageIndex==0)
	{
		strcopy(tg.Name,buf,40);
		GroupList[index]=tg;
	}
	else
	{
		strcopy(tu.Name,buf,50);
		UnitList[index]=tu;
	}
	GlobModif=TRUE;
}

void Del()
{
	TTreeNode tn;
	trView->GetTreeItem(trView->GetSelItem(),&tn);
	if (tn.ImageIndex==0)
	{
		//Удаление группы
		if (MessageBox(0,"Действительно удалить группу?","Подтверждение",MB_ICONQUESTION | MB_YESNO)==IDNO) return;
		TTreeNode tnc;
		while (trView->GetChildNode(tn.hHandle,&tnc))
			DelUnit(&tnc);
		TGROUP tg;
		int index=FindGroup(tn.Data, &tg);
		GroupList.Del(index);
		trView->DelItem(tn.hHandle);
	}
	else
	{
		//Удаление сайта
		if (MessageBox(0,"Действительно удалить информацию о сайте?","Подтверждение",MB_ICONQUESTION | MB_YESNO)==IDNO) return;
		DelUnit(&tn);
	}
	Modif=FALSE;
	GlobModif=TRUE;
	ShowInfo(NULL);
}

/*void LowerCase(char* st)
{
	int i=0;
	while (st[i])
	{
		if (st[i]>='A' && st[i]<='Z') st[i]+=32;
		if (st[i]>='А' && st[i]<='Я') st[i]+=32;
		i++;
	}
}*/

void FillTree()
{
	trView->Clear();
	int iu=0;
	int ig=0;
	int LoadIndex=1;
	while (ig<GroupList.count() || iu<UnitList.count())
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
		if (iu<UnitList.count())
		{
			tu=UnitList[iu];
			if (tu.LoadIndex==LoadIndex)
			{
				TTreeNode tn;
				PTreeNode ptn=NULL;
				if (tu.Group_ID)
				{
					if (trView->FindNode(0,tu.Group_ID,&tn))
					ptn=&tn;
				}
				trView->AddChildNode(ptn,tu.Name,tu.ID,1,1);
				iu++;
				LoadIndex++;
			}
		}
	}
}

void ShowInfo(HTREEITEM hit)
{
	TTreeNode tn;
	BOOL b=FALSE;
	if (hit)
	{
		trView->GetTreeItem(hit,&tn);
		b=tn.ImageIndex==1;
	}
	EnableWindow(GetDlgItem(hMainDlg,IDC_URL),b);
	EnableWindow(GetDlgItem(hMainDlg,IDC_LOGIN),b);
	EnableWindow(GetDlgItem(hMainDlg,IDC_PASS),b);
	EnableWindow(GetDlgItem(hMainDlg,IDC_COMMENT),b);
	bar->EnableButton(IDM_RENAME,hit!=NULL);
	if (hit)
	{
		bar->EnableButton(IDM_DEL,tn.ImageIndex!=0 || tn.Data!=1);
		bar->EnableButton(IDC_GOTO,tn.ImageIndex!=0);
	} else
	{
		bar->EnableButton(IDM_DEL,b);
		bar->EnableButton(IDC_GOTO,b);
	}
	ShowWindow(GetDlgItem(hMainDlg,IDC_DATE), b ? SW_SHOW : SW_HIDE);
	if (!b)
	{
		SetWindowText(GetDlgItem(hMainDlg,IDC_URL),"");
		SetWindowText(GetDlgItem(hMainDlg,IDC_LOGIN),"");
		SetWindowText(GetDlgItem(hMainDlg,IDC_PASS),"");
		SetWindowText(GetDlgItem(hMainDlg,IDC_COMMENT),"");
		SetWindowText(GetDlgItem(hMainDlg,IDC_DATE),"");
		Modif=FALSE;
		return;
	}
	TUNIT selu;
	if (FindUnit(tn.Data,&selu)==-1) return;

	SetWindowText(GetDlgItem(hMainDlg,IDC_URL),selu.URL);
	SetWindowText(GetDlgItem(hMainDlg,IDC_LOGIN),selu.Login);
	SetWindowText(GetDlgItem(hMainDlg,IDC_PASS),selu.Password);
	SetWindowText(GetDlgItem(hMainDlg,IDC_COMMENT),selu.Comment);
	wsprintf(buf,"%.02d-%.02d-%d %.02d:%.02d:%.02d",selu.Time.wDay,selu.Time.wMonth,selu.Time.wYear,
		selu.Time.wHour,selu.Time.wMinute,selu.Time.wSecond);
	SetWindowText(GetDlgItem(hMainDlg,IDC_DATE),buf);
	Modif=FALSE;
}

BOOL LoadOldUnits(HANDLE hFile)
{
	tg.Group_ID=0;
	tg.ID=1;
	tg.LoadIndex=1;
	strcpy(tg.Name,"Все сайты");
	GroupList.Add(&tg);
	int ID=1;
	int Count;
	DWORD dw;
	ReadFile(hFile,&Count,4,&dw,NULL);
	byte b;
	ReadFile(hFile,&b,1,&dw,NULL);
	while (Count--)
	{
		TUNITOLD uold;
		ReadFile(hFile,&uold,sizeof(TUNITOLD),&dw,NULL);
		byte crc=Decrypt((byte*)&uold,sizeof(TUNITOLD),b);
		byte crc2;
		ReadFile(hFile,&crc2,1,&dw,NULL);
		if (crc!=crc2) return FALSE;
		tu.ID=ID++;
		tu.LoadIndex=ID;
		tu.Group_ID=1;
		strcopy(tu.Comment,uold.Comment,200);
		strcopy(tu.Login,uold.Login,30);
		strcopy(tu.Name,uold.Name,40);
		strcopy(tu.Password,uold.Password,20);
		strcopy(tu.URL,uold.URL,100);
		tu.Time=uold.Time;
		UnitList.Add(&tu);
	}
	return TRUE;
}

BOOL LoadUnits()
{
	lstrcpy(buf,MainDir);
	lstrcat(buf,PASS_FILE);
	int Count;
	DWORD dw;
	BOOL Res=FALSE;
	HANDLE hFile = CreateFile(buf,GENERIC_READ, 0,0, OPEN_EXISTING,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		ReadFile(hFile,buf,4,&dw,NULL);
		if (buf[0]=='p' && buf[1]=='s' && buf[2]=='k')
		{
			if (buf[3]=='1') Res=LoadOldUnits(hFile);
			else if (buf[3]!='3') MessageBox(0,"Файл данных создан в несовместимой версии программы!","Ошибка загрузки",MB_ICONSTOP);
			else
			{
				int LoadIndex=1;
				ReadFile(hFile,&Count,4,&dw,NULL);
				byte b;
				ReadFile(hFile,&b,1,&dw,NULL);
				while (Count--)
				{
					byte typ;
					TGROUP tg;
					TUNIT tu;
					ReadFile(hFile,&typ,1,&dw,NULL);
					if (typ==0)
					{
						//Загружаем группу
						ReadFile(hFile,&tg,sizeof(TGROUP),&dw,NULL);
						byte crc=Decrypt((byte*)&tg,sizeof(TGROUP),b);
						tg.LoadIndex=LoadIndex++;
						byte crc2;
						ReadFile(hFile,&crc2,1,&dw,NULL);
						if (crc!=crc2)
						{
							CloseHandle(hFile);
							return FALSE;
						}
						GroupList.Add(&tg);
					}
					else
					{
						//Загружаем сайт
						ReadFile(hFile,&tu,sizeof(TUNIT),&dw,NULL);
						byte crc=Decrypt((byte*)&tu,sizeof(TUNIT),b);
						tu.LoadIndex=LoadIndex++;
						byte crc2;
						ReadFile(hFile,&crc2,1,&dw,NULL);
						if (crc!=crc2)
						{
							CloseHandle(hFile);
							return FALSE;
						}
						UnitList.Add(&tu);
					}
				}
				Res=TRUE;
			}
		}
		CloseHandle(hFile);
	}
	else
	{
		if (GetLastError()==2) //File not found
		{
			Res=TRUE;
		}
	}
	GlobModif=FALSE;
	Modif=FALSE;
	return Res;
}

void SaveNode(HTREEITEM hi, HANDLE hFile, byte b)
{
	TUNIT tu;
	TGROUP tg;
	byte typ;
	TTreeNode tn;
	DWORD dw;
	byte crc;
	trView->GetTreeItem(hi,&tn);
	if (tn.ImageIndex==0)
	{
		//Сохраняем группу
		if (FindGroup(tn.Data,&tg)==-1) return;
		typ=0;
		WriteFile(hFile,&typ,1,&dw,NULL);
		crc=Crypt((byte*)&tg,sizeof(TGROUP),b);
		WriteFile(hFile,&tg,sizeof(TGROUP),&dw,NULL);
		WriteFile(hFile,&crc,1,&dw,NULL);
	}
	else
	{
		//Сохраняем сайт
		if (FindUnit(tn.Data,&tu)==-1) return;
		typ=1;
		WriteFile(hFile,&typ,1,&dw,NULL);
		crc=Crypt((byte*)&tu,sizeof(TUNIT),b);
		WriteFile(hFile,&tu,sizeof(TUNIT),&dw,NULL);
		WriteFile(hFile,&crc,1,&dw,NULL);
	}
}

void SaveGroup(HTREEITEM hi, HANDLE hFile, byte b)
{
	while (hi)
	{
		SaveNode(hi,hFile,b);
		HTREEITEM hic=trView->GetChildItem(hi);
		if (hic) SaveGroup(hic,hFile,b);
		hi=trView->GetNextItem(hi);
	}
}

void SaveUnits()
{
	SaveInfo(NULL);
	if (!GlobModif) return;
	lstrcpy(buf,MainDir);
	lstrcat(buf,PASS_FILE);
	HANDLE hFile = CreateFile(buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD dw;
		buf[0]='p';
		buf[1]='s';
		buf[2]='k';
		buf[3]='3';
		WriteFile(hFile,buf,4,&dw,NULL);
		int Count=UnitList.count()+GroupList.count();

		WriteFile(hFile,&Count,4,&dw,NULL);
		SYSTEMTIME st;
		GetLocalTime(&st);
		byte b=(byte)st.wMilliseconds;
		WriteFile(hFile,&b,1,&dw,NULL);

		SaveGroup(trView->GetRootItem(), hFile, b);
		CloseHandle(hFile);
	}
	GlobModif=FALSE;
}

void OnDropped(int image1, int ID1, int image2, int ID2)
{
	if (image2) return;
	TGROUP tg;
	int index=FindGroup(ID2,&tg);
	if (index==-1) return;
	int index2;
	if (image1)
	{
		//Перемещали сайт
		TUNIT tu;
		index2=FindUnit(ID1,&tu);
		if (index2==-1) return;
		tu.Group_ID=tg.ID;
		UnitList[index2]=tu;
	}
	else
	{
		//Перемещали группу
		TGROUP tg2;
		index2=FindGroup(ID1,&tg2);
		if (index2==-1) return;
		tg2.Group_ID=tg.ID;
		GroupList[index2]=tg2;
	}
	GlobModif=TRUE;
}
