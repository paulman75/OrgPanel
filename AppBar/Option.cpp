#include "windows.h"
#include "..\Headers\general.h"
#include "AppBar.h"
#include "Compon\MyUtils.h"
#include "../Headers/const.h"
#include "RegUnit.h"
#include "Option.h"
#include "commctrl.h"
#include "BarUnit.h"
#include "../Headers/IconList.h"
#include "../Headers/DragList.h"
#include "BarStruct.h"
#include "Compon/FolderSw.h"
#include "Compon/ColorDlg.h"
#include "Sounds.h"
#include "res/resource.h"
#include "../Headers/Bitmap.h"

extern HINSTANCE hInstance;
extern PBarFolder	ActiveFolder,MainFolder,FirstFolder,nFolder;
extern BarUnit*		nUnit;
extern HWND BarhWnd;
extern CFolderSwitch*	Switch;
extern char	buf[1000],buf2[230];
extern CAppBar* hBar;

PBarFolder			ActFolder;
BarUnit*			ActUnit;

extern TBarConfig	BarCon;
CDragListBox*		UnitBox;
CDragListBox*		FolderBox;
CDragListBox*		DnevBox;
byte				ActFolderInd,ActUnitInd;
int					ActDnevInd;
BOOL				NeedSaveMainBitmap;
CIconListBox*		DeloBox;
CColorDlg*			ColorDlg;
CColorDlg*			ColorDlg2;
CColorDlg*			ColorTextDlg;
CSoundSelect*		SoundSelect;
BOOL				bToActiveFolder;

int iIntIndex;
HWND	hOptDialog;

void UpdateActUnit();
void UpdateActFolder();
void UpdateFolderBox();
void RefreshUnit();
void UpdateDnevList(byte val);
void FolderChange();

BOOL bLargeIcons;
HBITMAP	hBitPlay, hBitStop, hBitOff;
HICON hIconUp,hIconDown, hTimer;

CMyBitmap *DnevBit[DNEV_COUNT];
const char* DnevCap[]={"Дела","Дни рождения","Именины","Праздники","Приметы","События","Православие"};

void UpdatePasswordPage()
{
	BOOL c=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_CHECK1),BM_GETCHECK,0,0)==1);
	EnableWindow(GetDlgItem(hOptDialog,IDC_EDIT1),c);
	EnableWindow(GetDlgItem(hOptDialog,IDC_EDIT2),c);
	EnableWindow(GetDlgItem(hOptDialog,IDC_COMBO1),c);
}

void UpdateSlider3Visible()
{
	ActFolderInd=(byte)SendMessage(FolderBox->hWnd,LB_GETCURSEL,0,0);
	ShowWindow(GetDlgItem(hOptDialog,IDC_SLIDER3),ActFolderInd? SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hOptDialog,IDC_STATIC_TGR),ActFolderInd? SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hOptDialog,IDC_STATIC_T0),ActFolderInd? SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hOptDialog,IDC_STATIC_T100),ActFolderInd? SW_SHOW:SW_HIDE);
}

void FolderChange()
{
	ActFolderInd=(byte)SendMessage(FolderBox->hWnd,LB_GETCURSEL,0,0);
	UpdateActFolder();
	UpdateSlider3Visible();

	if (ActFolder==MainFolder) return;
	HideButton(ActiveFolder->FirstUnit);
	ActiveFolder=ActFolder;
	SetSwitchValue(ActFolderInd);
}

void UpdateDnevList()
{
	DnevBox->ClearAll();
	for (int i=0; i<DNEV_COUNT; i++)
	{
		int j=BarCon.Dnev[i];
		if (j>=100) j-=100;
		DnevBox->AddItem(DnevCap[j],BarCon.Dnev[i]>=100,DnevBit[j]->hdc);
	}
	if (ActDnevInd!=-1) SendMessage(DnevBox->hWnd,LB_SETCURSEL,ActDnevInd,0);
}

void UpdateDnevBut()
{
	EnableWindow(GetDlgItem(hOptDialog,IDC_BUTUP3),ActDnevInd>0);
	EnableWindow(GetDlgItem(hOptDialog,IDC_BUTDOWN3),ActDnevInd!=-1 && ActDnevInd<6);
}

void DnevMove(DWORD dwFrom, DWORD dwTo)
{
	byte b=BarCon.Dnev[dwFrom];
	int i;
	if (dwFrom>dwTo)
	{
		for (i=dwFrom-1; i>=(int)dwTo; i--) BarCon.Dnev[i+1]=BarCon.Dnev[i];
	}
	else
	{
		for (i=dwFrom+1; i<=(int)dwTo; i++) BarCon.Dnev[i-1]=BarCon.Dnev[i];
	}
	BarCon.Dnev[dwTo]=b;
	ActDnevInd=dwTo;
	UpdateDnevList();
	UpdateDnevBut();
}

void FolderMove(DWORD dwFrom, DWORD dwTo)
{
	if (dwFrom==dwTo) return;
	ActFolderInd=(byte)dwTo;
	nFolder=FirstFolder;
	while (--dwFrom) nFolder=nFolder->NextFolder;
	if (nFolder->PrevFolder) nFolder->PrevFolder->NextFolder=nFolder->NextFolder;
	else FirstFolder=nFolder->NextFolder;
	if (nFolder->NextFolder) nFolder->NextFolder->PrevFolder=nFolder->PrevFolder;
	PBarFolder	Prev=NULL;
	while (--dwTo)
	{
		if (!Prev) Prev=FirstFolder;
		else Prev=Prev->NextFolder;
	}
	if (Prev)
	{
		nFolder->NextFolder=Prev->NextFolder;
		if (Prev->NextFolder) Prev->NextFolder->PrevFolder=nFolder;
		nFolder->PrevFolder=Prev;
		Prev->NextFolder=nFolder;
	}
	else
	{
		nFolder->NextFolder=FirstFolder;
		nFolder->PrevFolder=NULL;
		FirstFolder->PrevFolder=nFolder;
		FirstFolder=nFolder;
	}
	nFolder->Modified=TRUE;
	ActFolder=nFolder;
	int i=1;
	nFolder=FirstFolder;
	while (nFolder)
	{
		nFolder->Index=i++;
		nFolder=nFolder->NextFolder;
	}
	UpdateFolderBox();
	SetSwitchValue(ActFolderInd);
	NeedSaveMainBitmap=TRUE;
}

void FolderMoveBut(DWORD dwFrom, int Delta)
{
	int iNewPos=dwFrom+Delta;

	int cnt=0;
	nFolder=FirstFolder;
	while (nFolder)
	{
		nFolder=nFolder->NextFolder;
		cnt++;
	}

	if (iNewPos<=0) iNewPos=cnt;
	if (iNewPos>cnt) iNewPos=1;
	FolderMove(dwFrom,iNewPos);
}

void UnitMove(DWORD dwFrom, DWORD dwTo, BOOL ToFolder)
{
	if (dwFrom==dwTo && !ToFolder) return;
	if (!ActFolder) return;

	nUnit=ActFolder->FirstUnit;
	while (dwFrom--) nUnit=nUnit->NextUnit;
	if (nUnit->PrevUnit) nUnit->PrevUnit->NextUnit=nUnit->NextUnit;
	else ActFolder->FirstUnit=nUnit->NextUnit;
	if (nUnit->NextUnit) nUnit->NextUnit->PrevUnit=nUnit->PrevUnit;
	nUnit->ParentFolder->Kol--;

	if (ToFolder)
	{
		if (!dwTo) nFolder=MainFolder;
		else
		{
			nFolder=FirstFolder;
			while (--dwTo) nFolder=nFolder->NextFolder;
		}
		/*BarUnit*	mUnit=nFolder->FirstUnit;
		dwTo=0;
		while (mUnit)
		{
			mUnit=mUnit->NextUnit;
			dwTo++;
		}*/
		ActUnitInd=0;
		ActUnit=ActFolder->FirstUnit;

		MoveShortCutToFolder(nUnit, nFolder);
	}
	else 
	{
		nFolder=nUnit->ParentFolder;
		ActUnitInd=(byte)dwTo;
		ActUnit=nFolder->FirstUnit;
	}
	BarUnit*	Prev=NULL;
	while (dwTo--)
	{
		if (!Prev) Prev=nFolder->FirstUnit;
		else Prev=Prev->NextUnit;
	}
	if (Prev)
	{
		nUnit->NextUnit=Prev->NextUnit;
		if (Prev->NextUnit) Prev->NextUnit->PrevUnit=nUnit;
		nUnit->PrevUnit=Prev;
		Prev->NextUnit=nUnit;
	}
	else
	{
		nUnit->NextUnit=nFolder->FirstUnit;
		nUnit->PrevUnit=NULL;
		if (nFolder->FirstUnit) nFolder->FirstUnit->PrevUnit=nUnit;
		nFolder->FirstUnit=nUnit;
	}
	nFolder->Kol++;
	nUnit->ParentFolder=nFolder;
	nFolder->Modified=TRUE;
	ActFolder->Modified=TRUE;
	PlaceIcon();
	RefreshUnit();
	NeedSaveMainBitmap=TRUE;
	if (ToFolder) InvalidateRect(BarhWnd,NULL,TRUE);
}

void UnitMoveBut(DWORD dwFrom, int Delta)
{
	if (!ActFolder) return;
	
	int iNewPos=dwFrom+Delta;

	nUnit=ActFolder->FirstUnit;
	int cnt=0;
	while (nUnit)
	{
		nUnit=nUnit->NextUnit;
		cnt++;
	}

	if (iNewPos<0) iNewPos=cnt-1;
	if (iNewPos>=cnt) iNewPos=0;
	UnitMove(dwFrom,iNewPos,FALSE);
}

void UpdateActUnit()
{
	if (ActFolder==NULL) nUnit=NULL;
	else nUnit=ActFolder->FirstUnit;
	int kk=0;
	ActUnit=NULL;
	while (nUnit!=NULL)
	{
        if (kk++==ActUnitInd) ActUnit=nUnit;
        nUnit=nUnit->NextUnit;
	}
	BOOL c=(ActUnit!=NULL);
	EnableWindow(GetDlgItem(hOptDialog,IDC_BUTUP2),c);
	EnableWindow(GetDlgItem(hOptDialog,IDC_BUTDOWN2),c);
	if (c) 
	{
		EnableWindow(GetDlgItem(hOptDialog,IDC_BUTUP2),ActUnit->PrevUnit!=NULL||ActUnit->NextUnit!=NULL);
		EnableWindow(GetDlgItem(hOptDialog,IDC_BUTDOWN2),ActUnit->PrevUnit!=NULL||ActUnit->NextUnit!=NULL);
	}
}

void RefreshUnit()
{
	UnitBox->BeginUpdate();
	UnitBox->ClearAll();
	if (ActFolder==NULL) return;
	nUnit=ActFolder->FirstUnit;
	while (nUnit!=NULL)
	{
		HDC dc=nUnit->ButImage? nUnit->ButImage->hdc : NULL;
		UnitBox->AddItem(nUnit->Caption->Text,!nUnit->Hidden,dc);
		nUnit=nUnit->NextUnit;
	}
	UnitBox->EndUpdate();
	SendMessage(UnitBox->hWnd, LB_SETCURSEL,ActUnitInd, 0);
	UpdateActUnit();
}

void UpdateActFolder()
{
	ActFolder=NULL;
	if (ActFolderInd==0) ActFolder=MainFolder;
	int i=1;
	nFolder=FirstFolder;
	while (nFolder!=NULL)
	{
        if (i++==ActFolderInd) ActFolder=nFolder;
        nFolder=nFolder->NextFolder;
	}
	EnableWindow(GetDlgItem(hOptDialog,IDC_BUTUP1),ActFolder->PrevFolder!=NULL||ActFolder->NextFolder!=NULL);
	EnableWindow(GetDlgItem(hOptDialog,IDC_BUTDOWN1),ActFolder->PrevFolder!=NULL||ActFolder->NextFolder!=NULL);
	ActUnitInd=0;
	ColorDlg->SetColor(ActFolder->BkColor);
	ColorDlg2->SetColor(ActFolder->BkColor2);
	ColorTextDlg->SetColor(ActFolder->TitColor);
	SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER3),TBM_SETPOS,1,ActFolder->Alpha*100/255);
	RefreshUnit();
}

void UpdateFolderBox()
{
	FolderBox->BeginUpdate();
	FolderBox->ClearAll();
	FolderBox->AddItem(MainFolder->Caption->Text,FALSE,NULL);
	nFolder=FirstFolder;
	while (nFolder!=NULL)
	{
		FolderBox->AddItem(nFolder->Caption->Text,FALSE,NULL);
		nFolder=nFolder->NextFolder;
	}
	UpdateActFolder();
	FolderBox->EndUpdate();
	SendMessage(FolderBox->hWnd,LB_SETCURSEL,ActFolderInd,0);
}

void UpdateTimeSyncLabel()
{
	ReadRegistry("LastTimeUpdate",(LPBYTE)&buf,1000);
	if (!lstrlen(buf)) strcpy(buf,"не синхронизировалось");
	SendMessage(GetDlgItem(hOptDialog,IDC_STATICTIME),WM_SETTEXT,0,(LPARAM)buf);
}

void OptionToScreen()
{
	#define DEL_COUNT 11
	const char chDel[DEL_COUNT][30]={"Не удалять","Удалять через 2 дня","Удалять через 5 дней","Удалять через 10 дней","Удалять через 20 дней","Удалять через месяц","Удалять через 2 месяца","Удалять через 3 месяца","Удалять через полгода","Удалять через год","Удалять через 2 года"};

	LPTSTR st=BarCon.UsedPassword ? BarCon.Password : NULL;
	POINT pt;
	pt.x=pt.y=0;
	ClientToScreen(hOptDialog,&pt);

	switch (iIntIndex)
	{
	case 0:
		SetWindowText(GetDlgItem(hOptDialog,IDC_EDITNAME),BarCon.FName);
		SetWindowText(GetDlgItem(hOptDialog,IDC_EDIT2),BarCon.SName);
		SendMessage(GetDlgItem(hOptDialog,IDC_BOOTCHECK),BM_SETCHECK,IsStartUp(),0);
		SendMessage(GetDlgItem(hOptDialog,IDC_ICONCHECK),BM_SETCHECK,bLargeIcons,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_AUTOCHECK),BM_SETCHECK,BarCon.AutoOpen,0);
		SendMessage(GetDlgItem(hOptDialog, IDC_ALWAYSONTOP), BM_SETCHECK, BarCon.AlwaysOnTop, 0);		
		SendMessage(GetDlgItem(hOptDialog,IDC_CHECKTIT),BM_SETCHECK,BarCon.TitShow,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER1),TBM_SETRANGEMAX,0,16);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER1),TBM_SETRANGEMIN,0,1);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER1),TBM_SETTICFREQ,5,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER1),TBM_SETPOS,1,BarCon.Speed);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER2),TBM_SETRANGEMAX,0,100);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER2),TBM_SETRANGEMIN,0,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER2),TBM_SETTICFREQ,10,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER2),TBM_SETPOS,1,BarCon.OpenDelay);
		RECT rc;
		GetWindowRect(GetDlgItem(hOptDialog,IDC_DATASTATIC),&rc);
		CreateDateLabel(hOptDialog, (rc.left+rc.right)/2-pt.x, 3*rc.bottom-2*rc.top-pt.y, &BarCon.Birth, 1,TRUE);
		break;
	case 1:
		SendMessage(GetDlgItem(hOptDialog,IDC_CHECKDIARYAUTO),BM_SETCHECK,IsDiaryStartUp(),0);
		SendMessage(GetDlgItem(hOptDialog,IDC_CHSHOWFULL),BM_SETCHECK,BarCon.ShowFullName,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_CHFULLIMEN),BM_SETCHECK,BarCon.FullImen,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_CHSKYINFO),BM_SETCHECK,BarCon.ShowSkyInfo,0);
		int i;
		for (i=0; i<DEL_COUNT; i++)
		{
			SendMessage(GetDlgItem(hOptDialog,IDC_CBDELDELA),CB_ADDSTRING,0,(LPARAM)chDel[i]);
			SendMessage(GetDlgItem(hOptDialog,IDC_CBDELEVENT),CB_ADDSTRING,0,(LPARAM)chDel[i]);
		}
		SendMessage(GetDlgItem(hOptDialog,IDC_CBDELDELA),CB_SETCURSEL,BarCon.DeleteDelo,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_CBDELEVENT),CB_SETCURSEL,BarCon.DeleteEvent,0);

		DnevBox=new CDragListBox(GetDlgItem(hOptDialog,IDC_DNEVLIST),TRUE,32);
		DnevBit[ID_DELA]=new CMyBitmap(IDB_BITDELA);
		DnevBit[ID_ROV]=new CMyBitmap(IDB_BITROV);
		DnevBit[ID_IMEN]=new CMyBitmap(IDB_BITIMEN);
		DnevBit[ID_PRAZ]=new CMyBitmap(IDB_BITPRAZ);
		DnevBit[ID_PRIM]=new CMyBitmap(IDB_BITPRIM);
		DnevBit[ID_EVENT]=new CMyBitmap(IDB_BITEVENT);
		DnevBit[ID_PRAV]=new CMyBitmap(IDB_BITPRAV);

		FillCity(GetDlgItem(hOptDialog,IDC_CBCITY));
		SetWindowText(GetDlgItem(hOptDialog,IDC_EDITGEOD),BarCon.GeoD);
		SetWindowText(GetDlgItem(hOptDialog,IDC_EDITGEOS),BarCon.GeoS);

		hIconUp=LoadIcon(hInstance,"UP");
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTUP3),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIconUp);

		hIconDown=LoadIcon(hInstance,"DOWN");
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTDOWN3),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIconDown);
		ActDnevInd=-1;
		UpdateDnevBut();
		UpdateDnevList();
		break;
	case 2:
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTPLAY),BM_SETIMAGE,0,(LPARAM)hBitPlay);
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTSTOP),BM_SETIMAGE,0,(LPARAM)hBitStop);
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTOFF),BM_SETIMAGE,0,(LPARAM)hBitOff);
		
		SendMessage(GetDlgItem(hOptDialog,IDC_CHECK1),BM_SETCHECK,BarCon.bCuckoo,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_CHECK2),BM_SETCHECK,BarCon.Sounds,0);
		SoundSelect=new CSoundSelect(GetDlgItem(hOptDialog,IDC_LIST1),GetDlgItem(hOptDialog,IDC_COMBO1),GetDlgItem(hOptDialog,IDC_BUTPLAY),GetDlgItem(hOptDialog,IDC_BUTSTOP));
		SoundSelect->AddUnit("Дела",BarCon.DoFileName);
		SoundSelect->AddUnit("События",BarCon.EventFileName);
		SoundSelect->AddUnit("Дни рождения",BarCon.RovFileName);
		SoundSelect->AddUnit("Бой часов - часовой",BarCon.HourFileName);
		SoundSelect->AddUnit("Бой часов - получасовой",BarCon.HalfHourFileName);
		SoundSelect->StartWork();
		break;
	case 3:
		UnitBox=new CDragListBox(GetDlgItem(hOptDialog,IDC_LISTUNIT),TRUE,32);
		FolderBox=new CDragListBox(GetDlgItem(hOptDialog,IDC_LISTFOLDER),FALSE,0);
		UnitBox->AttachDrag(FolderBox);

		hIconUp=LoadIcon(hInstance,"UP");
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTUP1),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIconUp);
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTUP2),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIconUp);

		hIconDown=LoadIcon(hInstance,"DOWN");
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTDOWN1),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIconDown);
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTDOWN2),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIconDown);

		GetWindowRect(GetDlgItem(hOptDialog,IDC_COLSTATIC),&rc);
		int x;
//		x=rc.left+(rc.right-rc.left)/2-pt.x-30;
		x=rc.right-pt.x;
		ColorDlg=new CColorDlg(hOptDialog,x,rc.top-pt.y-5,BarCon.BackGr);
//		x=rc.left+(rc.right-rc.left)/2-pt.x;
		x=rc.right+40-pt.x;
		ColorDlg2=new CColorDlg(hOptDialog,x,rc.top-pt.y-5,BarCon.BackGr2);

		GetWindowRect(GetDlgItem(hOptDialog,IDC_LABTEXT),&rc);
//		x=rc.right-pt.x+5;
		x=rc.right-pt.x;
		ColorTextDlg=new CColorDlg(hOptDialog,x,rc.top-pt.y-5,0);

		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER3),TBM_SETRANGEMAX,0,100);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER3),TBM_SETRANGEMIN,0,1);
		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER3),TBM_SETTICFREQ,20,0);
		ActFolderInd=bToActiveFolder ? ActiveFolder->Index : 0 ;
		UpdateFolderBox();

	//	if (ActiveFolder)
	//		SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER3),TBM_SETPOS,1,ActiveFolder->Alpha*100/255);
		UpdateSlider3Visible();
	case 4:
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)"(Нет)");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)"через 2 минуты");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)"через 5 минут");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)"через 10 минут");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)"через 20 минут");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)"через 30 минут");
	    SendMessage(GetDlgItem(hOptDialog,IDC_EDIT1),WM_SETTEXT,0,(LPARAM)st);
		SendMessage(GetDlgItem(hOptDialog,IDC_EDIT2),WM_SETTEXT,0,(LPARAM)st);
		SendMessage(GetDlgItem(hOptDialog,IDC_CHECK1),BM_SETCHECK,BarCon.UsedPassword,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBO1),CB_SETCURSEL,BarCon.RestorePassword,0);
		UpdatePasswordPage();
		SetWindowText(GetDlgItem(hOptDialog,IDC_EDITPREFIX),BarCon.Prefix);
		SendMessage(GetDlgItem(hOptDialog,IDC_PULSECHECK),BM_SETCHECK,BarCon.PulseDial,0);
		hTimer=LoadIcon(hInstance,"TIMER");
		SendMessage(GetDlgItem(hOptDialog,IDC_BUTSYNC),BM_SETIMAGE,IMAGE_ICON,(LPARAM)hTimer);
		UpdateTimeSyncLabel();
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBOTIME),CB_ADDSTRING,0,(LPARAM)"Не синхронизировать");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBOTIME),CB_ADDSTRING,0,(LPARAM)"Каждый день");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBOTIME),CB_ADDSTRING,0,(LPARAM)"Каждые два дня");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBOTIME),CB_ADDSTRING,0,(LPARAM)"Раз в неделю");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBOTIME),CB_ADDSTRING,0,(LPARAM)"Раз в месяц");
		SendMessage(GetDlgItem(hOptDialog,IDC_COMBOTIME),CB_SETCURSEL,BarCon.SyncTime,0);
		break;
	}
}

void OptionFromScreen()
{
	switch (iIntIndex)
	{
	case 0:
		SendMessage(GetDlgItem(hOptDialog,IDC_EDITNAME),WM_GETTEXT,15,(LPARAM)&BarCon.FName);
		SendMessage(GetDlgItem(hOptDialog,IDC_EDIT2),WM_GETTEXT,15,(LPARAM)&BarCon.SName);
		BarCon.AutoOpen=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_AUTOCHECK),BM_GETSTATE,0,0)&1);
		BarCon.AlwaysOnTop = (BOOL)(SendMessage(GetDlgItem(hOptDialog, IDC_ALWAYSONTOP), BM_GETSTATE, 0, 0) & 1);
		BarCon.TitShow=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_CHECKTIT),BM_GETSTATE,0,0)&1);
		bLargeIcons=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_ICONCHECK),BM_GETSTATE,0,0)&1);
		BOOL Start;
		Start=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_BOOTCHECK),BM_GETSTATE,0,0)&1);
		BarCon.Speed=(byte)SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER1),TBM_GETPOS,0,0);
		hBar->SetSpeed(BarCon.Speed,BarCon.Speed);
		BarCon.OpenDelay=(byte)SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER2),TBM_GETPOS,0,0);
		hBar->SetOpenDelay(BarCon.OpenDelay*10);
		SetStartUp(Start);
		GetDate(&BarCon.Birth,1);
		DeleteAllDateLabels();
		break;
	case 1:
		delete DnevBox;
		DnevBox=NULL;
		int i;
		for (i=0; i<DNEV_COUNT; i++) delete DnevBit[i];
		DeleteObject(hIconUp);
		DeleteObject(hIconDown);
		Start=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_CHECKDIARYAUTO),BM_GETSTATE,0,0)&1);
		BarCon.ShowFullName=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_CHSHOWFULL),BM_GETSTATE,0,0)&1);
		BarCon.FullImen=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_CHFULLIMEN),BM_GETSTATE,0,0)&1);
		BarCon.ShowSkyInfo=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_CHSKYINFO),BM_GETSTATE,0,0)&1);
		BarCon.DeleteDelo=(byte)SendMessage(GetDlgItem(hOptDialog,IDC_CBDELDELA),CB_GETCURSEL,0,0);
		BarCon.DeleteEvent=(byte)SendMessage(GetDlgItem(hOptDialog,IDC_CBDELEVENT),CB_GETCURSEL,0,0);

		GetWindowText(GetDlgItem(hOptDialog,IDC_EDITGEOD),BarCon.GeoD,20);
		GetWindowText(GetDlgItem(hOptDialog,IDC_EDITGEOS),BarCon.GeoS,20);
		SetDiaryStartUp(Start);
		break;
	case 2:
		BarCon.Sounds=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_CHECK2),BM_GETSTATE,0,0)&1);
		BarCon.bCuckoo=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_CHECK1),BM_GETSTATE,0,0)&1);
		delete SoundSelect;
		SoundSelect=NULL;
		break;
	case 3:
		DeleteObject(hIconUp);
		DeleteObject(hIconDown);
		delete FolderBox;
		FolderBox=NULL;
		delete UnitBox;
		UnitBox=NULL;
		delete ColorDlg;
		delete ColorDlg2;
		delete ColorTextDlg;
		break;
	case 4:
		char pas1[15];
		char pas2[15];
		SendMessage(GetDlgItem(hOptDialog,IDC_EDIT1),WM_GETTEXT,15,(LPARAM)&pas1);
		SendMessage(GetDlgItem(hOptDialog,IDC_EDIT2),WM_GETTEXT,15,(LPARAM)&pas2);
		BarCon.UsedPassword=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_CHECK1),BM_GETSTATE,0,0)&1);
		if (BarCon.UsedPassword && lstrcmp(pas1,pas2))
		{
			MessageBox(BarhWnd,"Пароль введен не верно!","Ошибка",MB_ICONSTOP);
			BarCon.UsedPassword=FALSE;
		}
		lstrcpy(BarCon.Password,pas1);
		BarCon.RestorePassword=(byte)SendMessage(GetDlgItem(hOptDialog,IDC_COMBO1),CB_GETCURSEL,0,0);
		SendMessage(GetDlgItem(hOptDialog,IDC_EDITPREFIX),WM_GETTEXT,15,(LPARAM)&BarCon.Prefix);
		BarCon.PulseDial=(BOOL)(SendMessage(GetDlgItem(hOptDialog,IDC_PULSECHECK),BM_GETSTATE,0,0)&1);
		DeleteObject(hTimer);
		BarCon.SyncTime=(byte)SendMessage(GetDlgItem(hOptDialog,IDC_COMBOTIME),CB_GETCURSEL,0,0);
	}
}

void OnButOff()
{
	delete SoundSelect;
	SoundSelect=NULL;
	BarCon.bCuckoo=FALSE;
	BarCon.Sounds=FALSE;
	strcpy(BarCon.DoFileName,"");
	strcpy(BarCon.EventFileName,"");
	strcpy(BarCon.RovFileName,"");
	strcpy(BarCon.HourFileName,"");
	strcpy(BarCon.HalfHourFileName,"");
	OptionToScreen();
}

BOOL FAR PASCAL IntDialogProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		hOptDialog=hDlg;
		OptionToScreen();
		return TRUE;
	case WM_DESTROY:
		OptionFromScreen();
		break;
	case WM_COLORDIALOGCHANGE:
		ActFolder->BkColor=ColorDlg->GetColor();
		ActFolder->BkColor2=ColorDlg2->GetColor();
		ActFolder->TitColor=ColorTextDlg->GetColor();
		ActFolder->Modified=TRUE;
		if (ActFolderInd==0) BarCon.BackGr=ColorDlg->GetColor();
		RepaintBar();
		InvalidateRect(BarhWnd,NULL,TRUE);
		break;
	case WM_HSCROLL:
		if (iIntIndex==3)
		{
			int a=((byte)SendMessage(GetDlgItem(hOptDialog,IDC_SLIDER3),TBM_GETPOS,0,0))*255/100;
			if (ActFolder->Alpha!=a)
			{
				ActFolder->Alpha=a;
				SetAlpha(ActiveFolder->Alpha);
			}
		}
		break;
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (LOWORD(wParam)==IDC_BUTPLAY && iIntIndex==2) { SoundSelect->DoPlay(); break; }
			if (LOWORD(wParam)==IDC_BUTSTOP && iIntIndex==2) { SoundSelect->DoStop(); break; }
			if (LOWORD(wParam)==IDC_BUTOFF && iIntIndex==2) { OnButOff(); break;}
			MySound();
			if (LOWORD(wParam)==IDC_CHECK1 && iIntIndex==4) UpdatePasswordPage();
			if (LOWORD(wParam)==IDC_BUTTON1 && iIntIndex==2) SoundSelect->DoBrowse();
			if (LOWORD(wParam)==IDC_BUTDOWN2) UnitMoveBut(ActUnitInd,1);
			if (LOWORD(wParam)==IDC_BUTUP2) UnitMoveBut(ActUnitInd,-1);
			if (LOWORD(wParam)==IDC_BUTDOWN1) FolderMoveBut(ActFolderInd,1);
			if (LOWORD(wParam)==IDC_BUTUP1) FolderMoveBut(ActFolderInd,-1);
			if (LOWORD(wParam)==IDC_BUTDOWN3) DnevMove(ActDnevInd,ActDnevInd+1);
			if (LOWORD(wParam)==IDC_BUTUP3) DnevMove(ActDnevInd,ActDnevInd-1);
			if (LOWORD(wParam)==IDC_BUTSYNC)
			{
				TimeSync(TRUE,hDlg);
				UpdateTimeSyncLabel();
			}
			break;
		case LBN_SELCHANGE:
			if (LOWORD(wParam)==IDC_CBCITY)
				ChangeCity(GetDlgItem(hDlg,IDC_CBCITY),GetDlgItem(hDlg,IDC_EDITGEOD),GetDlgItem(hDlg,IDC_EDITGEOS));
			if (LOWORD(wParam)==IDC_LIST1 && iIntIndex==2) SoundSelect->UpdateAll();
			if (LOWORD(wParam)==IDC_COMBO1 && iIntIndex==2) SoundSelect->ChangeFile();
			if (LOWORD(wParam)==IDC_LISTFOLDER) FolderChange();
			if (LOWORD(wParam)==IDC_LISTUNIT) 
			{
				ActUnitInd=(byte)SendMessage(UnitBox->hWnd,LB_GETCURSEL,0,0);
				UpdateActUnit();
			}
			if (LOWORD(wParam)==IDC_DNEVLIST)
			{
				ActDnevInd=(byte)SendMessage(DnevBox->hWnd,LB_GETCURSEL,0,0);
				UpdateDnevBut();
			}
			break;
		}
		break;
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		switch (wParam)
		{
		case IDC_LIST1:
			if (SoundSelect) return SoundSelect->WndProc(hDlg,message,wParam,lParam);
			break;
		case IDC_LISTUNIT:
			if (UnitBox) return UnitBox->OnChildNotify(message,wParam,lParam);
			break;
		case IDC_LISTFOLDER: 
			if (FolderBox) return FolderBox->OnChildNotify(message,wParam,lParam);
			break;
		case IDC_DNEVLIST:
			if (DnevBox) return DnevBox->OnChildNotify(message,wParam,lParam);
			break;
		}
		break;
	case WM_LISTBOXCHECKCHANGE:
		if (HIWORD(wParam)==IDC_LISTUNIT)
		{
			ActUnitInd=(byte)LOWORD(wParam);
			UpdateActUnit();
			ActUnit->Hidden=!(BOOL)lParam;
			ActUnit->ParentFolder->Modified=TRUE;
			PlaceIcon();
			InvalidateRect(BarhWnd,NULL,TRUE);
			MySound();
		}
		if (HIWORD(wParam)==IDC_DNEVLIST)
		{
			ActDnevInd=(byte)LOWORD(wParam);
			UpdateDnevBut();
			BarCon.Dnev[ActDnevInd]=BarCon.Dnev[ActDnevInd]>=100 ? BarCon.Dnev[ActDnevInd]-100 : BarCon.Dnev[ActDnevInd]+100;
			if (BarCon.Dnev[ActDnevInd]==ID_DELA)
			{
				BarCon.Dnev[ActDnevInd]=ID_DELA+100;
				DnevBox->SetCheck(ActDnevInd,FALSE);
			}
			else MySound();
		}
		break;
	case WM_LISTBOXENDDRAG:
		WORD FromIndex;
		WORD ToIndex;
		FromIndex=LOWORD(wParam);
		ToIndex=LOWORD(lParam);
		if (lParam==(LPARAM)wParam) break;
		switch (HIWORD(wParam))
		{
		case IDC_LISTFOLDER:
			if (!FromIndex) break;
			if (!ToIndex) ToIndex=1;
			FolderMove(FromIndex,ToIndex);
			break;
		case IDC_LISTUNIT:
			UnitMove(FromIndex,ToIndex,HIWORD(lParam)!=HIWORD(wParam));
			break;
		case IDC_DNEVLIST:
			DnevMove(FromIndex,ToIndex);
			break;
		}
		break;
	case MM_MCINOTIFY:
		SoundSelect->OnEndPlay();
		break;
	}
	return FALSE;
}

char*	Captions[]={"Общие","Дневник","Звуки","Кнопки","Разное"};

void UpdateDialog(int iIndex, HWND hDlg)
{
	if (hOptDialog) DestroyWindow(hOptDialog);
	iIntIndex=iIndex;
	if (iIndex==-1) return;
	char	Dia[]="OPTDIA ";
	Dia[6]=iIndex+48;
	CreateDialog(hInstance,Dia,hDlg,IntDialogProc);
	ShowWindow(hOptDialog,SW_SHOW);
}

BOOL FAR PASCAL OptionWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	int i;

    switch (message) {
	case WM_SHOWWINDOW:
		if (!iIntIndex) SetFocus(GetDlgItem(hOptDialog,IDC_EDITNAME));
		break;
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		TC_ITEM	tc;
		ZeroMemory(&tc,sizeof(tc));
		tc.mask=TCIF_TEXT;
		for (i=0; i<5; i++)
		{
			tc.pszText=Captions[i];
			SendMessage(GetDlgItem(hDlg,IDC_TAB1),TCM_INSERTITEM,i,(LPARAM)&tc);
		}
		hOptDialog=NULL;
		hBitPlay=LoadBitmap(hInstance,"PLAY");
		hBitStop=LoadBitmap(hInstance,"STOP");
		hBitOff=LoadBitmap(hInstance,"NOSOUND");
		UpdateDialog(bToActiveFolder ? 3 : 0,hDlg);
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
		if (HIWORD(wParam)==BN_CLICKED)
		switch (LOWORD(wParam))
		{
		case IDOK: 
		case IDCANCEL:
			UpdateDialog(-1,NULL);
			EndDialog(hDlg,LOWORD(wParam)==IDOK);
			return TRUE;
		case IDHELP:
			CheckUpdate(hDlg);
			break;
		case IDC_BUTTON2:
			ShellExecute(0,"open","http://paulman.newmail.ru/orgreg.html",NULL,NULL,SW_SHOW);
			EndDialog(hDlg,TRUE);
			break;
		}
	}
	return FALSE;
}

void ConfigChange(LPCTSTR ClassName)
{
	HWND hh=FindWindow(ClassName,NULL);
	if (hh) SendMessage(hh,WM_CONFIGCHANGE,0,0);
}

void UserInfoToScreen(HWND Mainh, BOOL pbToActiveFolder)
{
	bToActiveFolder=pbToActiveFolder;
	bLargeIcons=BarCon.LargeIcons;
	NeedSaveMainBitmap=FALSE;
	DialogBox(hInstance, "OPTION", Mainh, (DLGPROC)OptionWndProc);
	BOOL bl=(BarCon.LargeIcons!=bLargeIcons);
	BarCon.LargeIcons=bLargeIcons;
	hBar->bNeedTitle=BarCon.TitShow;
	SetHourTimer();
	SaveFolderDat();
	SaveBarStruct();
	if (NeedSaveMainBitmap) SaveMainBitmap();
	if (bl) UpdateIconSize();
	BarUsePassword();
	SaveOptions(&BarCon);
	ConfigChange(DiaryWindow);
	ConfigChange(NoteBookWindow);
	ConfigChange(NotePadWindow);
}
