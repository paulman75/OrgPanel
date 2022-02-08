#include <windows.h>
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "DateTime.h"
#include "IntEvent.h"
#include "Pages.h"
#include "Praz.h"
#include "Human.h"
#include "Event.h"
#include "Dela.h"
#include "../Headers/DynList.h"
#include "TrayBar.h"
#include "res/resource.h"
#include "../Headers/exChange.h"
#include "../Headers/Bitmap.h"
#include "Days.h"
#include "../Addons/Logger/logger.h"

DynArray<TINTEVENT> IntEventList;
extern DynArray<TEVENT> EventList;
extern DynArray<TDelo> DeloList;
extern BOOL    bDnevModif;
extern int IconMode;
extern HWND hwndMain;
extern HINSTANCE hInstance;		  // Global instance handle for application
extern char cEvTit[8][20];
extern HBRUSH	hWhiteBrush;
extern PEXCHANGE Humans;
extern int HumanCount;
extern CMyBitmap*	Image[6];
extern BOOL bHumanExchangeOK;
HWND hSvodWnd;
HBRUSH hBackBrush;
HRGN rgn;
POINT rgnPoint[11];

#define DiarySvodWindow "DiarySvodWindow"
#define iWidth 350
#define iHeight 250

int IntTimerID=0;
int LastPrEvent;
int CurEv;
BOOL bEvDlgOpen=FALSE;
extern int OpovMin[];
#define LATER_COUNT 19
const char ChLater[LATER_COUNT][10]={"нет","5 минут","10 минут","15 минут","20 минут","30 минут","45 минут","1 час","1,5 часа","2 часа","3 часа","4 часа", "5 часов","6 часов","10 часов","12 часов","1 день","2 дня","3 дня"};
const int  MinLater[LATER_COUNT]={0,5,10,15,20,30,45,60,90,120,180,240,300,360,600,720,1440,2880,4320};
int		*pLaterIndex;
BOOL	*pReaded;
extern TPage LeftPage, RightPage;

void DnevToScreen(BOOL bSetNoActive);

void EventSound(int type)
{
	char sound[200];
	sound[0]=0;
	if (type==ID_DELA)
		ReadRegistry("DoWav",(byte*)&sound,200);
	if (type==ID_EVENT)
		ReadRegistry("EventWav",(byte*)&sound,200);
	if (type==ID_ROV)
		ReadRegistry("RovWav",(byte*)&sound,200);
	if (lstrlen(sound)>0) sndPlaySound(sound,SND_ASYNC);
}

int FindIntEvent(int type, int ID2, PDate pd)
{
	int i=0;
	while (i<IntEventList.count())
	{
		TINTEVENT iev=IntEventList[i];
		if (pd)
		{
			if (iev.ID2==ID2 && iev.type==type && iev.RealDate==*pd) return i;
		}
		else
			if (iev.ID2==ID2 && iev.type==type) return i;
		i++;
	}
	return -1;
}

BOOL CheckExists(PINTEVENT piev)
{
	switch (piev->type)
	{
	case ID_EVENT:
		TEVENT ev;
		return (GetEventByID(piev->ID2,&ev)!=-1);
		break;
	case ID_DELA:
		TDelo dl;
		if (GetDeloByID(piev->ID2,&dl)==-1) return FALSE;
		return !dl.factDate.Month;
		break;
	case ID_ROV:
		if (!bHumanExchangeOK) return TRUE;
		int i;
#ifdef DEBUG
WriteLog("CheckExists id=",piev->ID2);
#endif
		i=GetHumanByKey(piev->ID2);
#ifdef DEBUG
WriteLog("CheckExists i=",i);
#endif
		if (i==-1) return FALSE;
#ifdef DEBUG
WriteLog("CheckExists mon=",piev->RealDate.Month);
WriteLog("CheckExists day=",piev->RealDate.Day);
WriteLog("CheckExists mon=",Humans[i].BDate.Month);
WriteLog("CheckExists day=",Humans[i].BDate.Day);
#endif
		return (piev->RealDate.Month==Humans[i].BDate.Month && piev->RealDate.Day==Humans[i].BDate.Day);
	}
	return FALSE;
}

void __stdcall OnIntTimer(HWND hwnd,unsigned int msg, unsigned int id, DWORD tim)
{
	KillTimer(NULL,id);
	IntTimerID=0;
	EventSound(IntEventList[0].type);
	SetIconMode(IntEventList[0].type);
#ifdef DEBUG
WriteLog("сработал таймер id=",IntEventList[0].ID2);
WriteLog("сработал таймер type=",IntEventList[0].type);
#endif
}

void InsertIntEvent(PINTEVENT pev)
{
	bDnevModif=TRUE;
	int i=0;
	while (i<IntEventList.count())
	{
		TINTEVENT ev=IntEventList[i];
		if (ev.Time>pev->Time) break;
		i++;
	}
#ifdef DEBUG
WriteLog("InsertIntExvent ID=",pev->ID2);
WriteLog("InsertIntExvent type=",pev->type);
#endif
	IntEventList.Add(pev,i);
}

void SearchNear()
{
	if (bEvDlgOpen) return;
	//Ищем событие которое уже прошло
	if (IntTimerID) KillTimer(NULL,IntTimerID);
	IntTimerID=0;
	TINTEVENT iev;
	TDateTime dt=NowTime();
	while (IntEventList.count()>0)
	{
		iev=IntEventList[0];
		if (!CheckExists(&iev))
		{
#ifdef DEBUG
WriteLog("Удаляем enevt ID=",iev.ID2);
WriteLog("Удаляем event type=",iev.type);
#endif
			IntEventList.Del(0);
			continue;
		}
		if (iev.Time<dt)
		{
			SetIconMode(iev.type);
			EventSound(iev.type);
			return;
		}
		int min=(iev.Time-dt)*60;
		SYSTEMTIME st;
		GetLocalTime(&st);
		min-=st.wSecond;
#ifdef DEBUG
WriteLog("Таймер поставили min=",min);
#endif
		IntTimerID=SetTimer(NULL,NULL,min*1000,OnIntTimer);
		break;
	}
}

void DelIntEvent(int type, int ID2)
{
#ifdef DEBUG
WriteLog("DelIntEvent ID=",ID2);
WriteLog("DelIntEvent type=",type);
#endif
	while (TRUE)
	{
		int i=FindIntEvent(type,ID2,NULL);
		if (i==-1) break;
		IntEventList.Del(i);
	}
	bDnevModif=TRUE;
	SearchNear();
}

void UpdateIntEvent()
{
	DelOldEvent();
	DelOldDelo();
	TDateTime prTime;
	if (!ReadRegistry("DnevPrDate",(byte*)&prTime.Date,sizeof(TDate)))
	{
		prTime.Date=Now();
		DateBefore(&prTime.Date,30);
	}
	else
	{
		if (prTime.Date>Now()) prTime=NowTime();
	}
	TDate neDate=Now();
	DateForward(&neDate,30);

	//Добавляем события
	for (int i=0; i<EventList.count(); i++)
	{
		TEVENT ev=EventList[i];
		TDate stDate=prTime.Date;
		TDateTime rd;
		while (GetEventAfterDate(&stDate,&ev,&rd.Date))
		{
			rd.Hour=ev.Time.Hour;
			rd.Min=ev.Time.Min;
			TDateTime evtim=rd;
			evtim-=OpovMin[ev.NotifyMin];
			if (evtim>prTime && evtim.Date<neDate &&
			   FindIntEvent(ID_EVENT,ev.ID,&evtim.Date)==-1)
			{
				TINTEVENT iev;
				iev.Time=evtim;
				iev.type=ID_EVENT;
				iev.ID2=ev.ID;
				iev.RealDate=rd.Date;
				InsertIntEvent(&iev);
			}
			else break;
			stDate=rd.Date;
			stDate++;
		}
	}
	//Добавляем дела
	for (int i=0; i<DeloList.count(); i++)
	{
		TDelo dl=DeloList[i];
		TDateTime rd;
		if (dl.NeedAlarm==1)
		{
			rd.Date=dl.begDate;
			rd.Hour=dl.Hour;
			rd.Min=dl.Min;
			if (rd>prTime && rd.Date<neDate &&
					FindIntEvent(ID_DELA,dl.ID,&rd.Date)==-1)
			{
				TINTEVENT iev;
				iev.Time=rd;
				iev.type=ID_DELA;
				iev.ID2=dl.ID;
				iev.RealDate=rd.Date;
				InsertIntEvent(&iev);
			}
		}
		if (dl.NeedAlarm==2)
		{
			TDate stDate=prTime.Date;
			BOOL b;
			while (GetDeloAfterDate(&stDate,&dl,&rd.Date,&b))
			{
				rd.Hour=dl.Hour;
				rd.Min=dl.Min;
				if (rd>prTime && rd.Date<neDate &&
					FindIntEvent(ID_DELA,dl.ID,&rd.Date)==-1)
				{
					TINTEVENT iev;
					iev.Time=rd;
					iev.type=ID_DELA;
					iev.ID2=dl.ID;
					iev.RealDate=rd.Date;
					InsertIntEvent(&iev);
				}
				else break;
				stDate=rd.Date;
				stDate++;
			}
		}
	}
	//Добавляем дни рождения и именины
	for (int i=0; i<HumanCount; i++)
	{
		TDate dd=Humans[i].BDate;
		if (!dd.Month) continue;
		dd.Year=dd.Month>6 ? prTime.Date.Year : neDate.Year;
		TDate rd=dd;
		DateBefore(&dd,3);

		if (dd>prTime.Date && dd<neDate)
		{
			if (FindIntEvent(ID_ROV,Humans[i].Key,&rd)==-1)
			{
				TINTEVENT iev;
				iev.Time.Date=dd;
				iev.Time.Hour=10;
				iev.Time.Min=0;
				iev.type=ID_ROV;
				iev.ID2=Humans[i].Key;
				iev.RealDate=rd;
				InsertIntEvent(&iev);
			}
		}
	}

	WriteBinToReg("DnevPrDate",(byte*)&neDate,sizeof(TDate));
	SearchNear();
}

void GetEventText(PINTEVENT pie, char* temp2)
{
	temp2[0]=0;
	char* tt=(char*)malloc(500);
	switch (pie->type)
	{
	case ID_ROV:
		int Ind;
		Ind=GetHumanByKey(pie->ID2);
		if (Ind==-1) return;
		DateToString(&pie->RealDate,temp2);
		int d;
		d=pie->RealDate.Year-Humans[Ind].BDate.Year;
		strcat(temp2,"\r\n");
		MakeShortName(Humans[Ind].FName[0] ? Humans[Ind].FName:NULL,Humans[Ind].OName[0] ? Humans[Ind].OName : NULL,Humans[Ind].SName[0] ? Humans[Ind].SName : NULL,tt);
		strcat(temp2,tt);
		YearToString(tt,d);
		lstrcat(temp2," ");
		lstrcat(temp2,tt);
		break;
	case ID_DELA:
		TDelo dl;
		if (GetDeloByID(pie->ID2, &dl)==-1) return;
		DateToString(&dl.begDate,temp2);
		DateToString(&dl.endDate,tt);
		strcat(temp2," - ");
		strcat(temp2,tt);
		strcat(temp2,"\r\n");
		strcat(temp2,dl.Text);
		break;
	case ID_EVENT:
		TEVENT ev;
		if (GetEventByID(pie->ID2, &ev)==-1) return;
		if (!ev.bHoliday && GetDayColor(&pie->RealDate)) return;
		DateToString(&pie->RealDate,temp2);
		strcat(temp2,"\r\n");
		wsprintf(tt,"%.02d:%.02d\r\n%s",ev.Time.Hour,ev.Time.Min,ev.Text);
		strcat(temp2,tt);
		break;
	}
	free(tt);
}

void UpdateEvInfo(HWND hDlg)
{
	TINTEVENT ie=IntEventList[CurEv];
	pReaded[CurEv]=TRUE;
	SetWindowText(GetDlgItem(hDlg,IDC_STATICEV),cEvTit[ie.type]);
	char *temp=(char*)malloc(1000);

	GetEventText(&ie,temp);

	SetWindowText(GetDlgItem(hDlg,IDC_EDITEV),temp);
	free(temp);
	EnableWindow(GetDlgItem(hDlg,IDC_BUTPREV),CurEv>0);
	EnableWindow(GetDlgItem(hDlg,IDC_BUTNEXT),CurEv<LastPrEvent);
	SendMessage(GetDlgItem(hDlg,IDC_COMBOEV),CB_SETCURSEL,pLaterIndex[CurEv],0);
}

void SaveLaterIndex(HWND hDlg)
{
	pLaterIndex[CurEv]=SendMessage(GetDlgItem(hDlg,IDC_COMBOEV),CB_GETCURSEL,0,0);
}

BOOL FAR PASCAL OnEventDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		char* tit;
		tit=(char*)malloc(100);
		strcpy(tit,"События");
		if (LastPrEvent>0)
			wsprintf(tit,"%s (%d)",tit,LastPrEvent+1);
		SetWindowText(hDlg,tit);
		free(tit);
		PutOnCenter(hDlg);
		int i;
		for (i=0; i<LATER_COUNT; i++)
			SendMessage(GetDlgItem(hDlg,IDC_COMBOEV),CB_ADDSTRING,0,(LPARAM)ChLater[i]);

		UpdateEvInfo(hDlg);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam)==IDCANCEL)
		{
			int i;
			BOOL b;
			b=TRUE;
			for (i=0; i<=LastPrEvent; i++) if (!pReaded[i]) b=FALSE;

			if (!b)
			{
				if (MessageBox(hDlg,"Вы не просмотрели все сообщения. Выйти из списка?","Внимание",MB_OKCANCEL | MB_ICONWARNING)==IDCANCEL) break;
			}
			SaveLaterIndex(hDlg);
			EndDialog(hDlg, TRUE);
			return (TRUE);
	    }
		if (LOWORD(wParam)==IDC_BUTNEXT)
		{
			SaveLaterIndex(hDlg);
			CurEv++;
			UpdateEvInfo(hDlg);
		}
		if (LOWORD(wParam)==IDC_BUTPREV)
		{
			SaveLaterIndex(hDlg);
			CurEv--;
			UpdateEvInfo(hDlg);
		}
		break;
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam==GetDlgItem(hDlg,IDC_EDITEV))
		{
			return (int)hWhiteBrush;
		}
		break;
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hDlg,&ps);
		BitBlt(ps.hdc,10,10,20,20,Image[IntEventList[CurEv].type]->hdc,0,0,SRCCOPY);
		EndPaint(hDlg,&ps);
		break;
	}
	return FALSE;
}

void OnIconDblClick()
{
	SetForegroundWindow(hwndMain);
	if (!CheckPassword(FALSE, hwndMain)) return;
	ShowWindow(hwndMain,SW_SHOWNORMAL);
	LeftPage.Date=Now();
	DnevToScreen(TRUE);
	if (IconMode==ID_MAIN) return;
	//Ищем последнее прошедшее событие
	int im=IconMode;
	SetIconMode(ID_MAIN);
	LastPrEvent=0;
	TDateTime dt=NowTime();
	while (LastPrEvent<IntEventList.count())
	{
		TINTEVENT iev=IntEventList[LastPrEvent];
		if (!CheckExists(&iev))
		{
			IntEventList.Del(LastPrEvent);
			continue;
		}
		if (iev.Time>dt) break;
		LastPrEvent++;
	}
	LastPrEvent--;
	if (LastPrEvent>=0)
	{
		CurEv=0;
		bEvDlgOpen=TRUE;
		pLaterIndex=(int*)malloc(sizeof(int)*(LastPrEvent+1));
		pReaded=(BOOL*)malloc(sizeof(BOOL)*(LastPrEvent+1));
		for (int i=0; i<=LastPrEvent; i++) pLaterIndex[i]=0;
		for (int i=0; i<=LastPrEvent; i++) pReaded[i]=FALSE;
		DialogBox(hInstance, "ONEVENTDLG", hwndMain, (DLGPROC)OnEventDlgProc);
		free(pReaded);
		int i=LastPrEvent;
		while (i>=0)
		{
			if (!pLaterIndex[i]) IntEventList.Del(i);
			else
			{
				TINTEVENT ie=IntEventList[i];
				IntEventList.Del(i);
				ie.Time=NowTime();
				ie.Time+=(int)MinLater[pLaterIndex[i]];
				InsertIntEvent(&ie);
			}
			i--;
		}
		free(pLaterIndex);
		bDnevModif=TRUE;
		bEvDlgOpen=FALSE;
		UpdateIntEvent();
	}
}

void DelWindow()
{
		DestroyWindow(hSvodWnd);
		UnregisterClass(DiarySvodWindow,hInstance);
		DeleteObject(hBackBrush);
		DeleteObject(rgn);
}

void PaintSvod(HDC wdc)
{
	RECT rc;
	GetClientRect(hSvodWnd,&rc);
	FillRect(wdc,&rc,hBackBrush);
	MoveToEx(wdc,rgnPoint[2].x-1, rgnPoint[2].y-1,NULL);
	for (int i=3; i<9; i++)
	LineTo(wdc,rgnPoint[i].x-1, rgnPoint[i].y-1);
}

LONG FAR PASCAL SvodWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		DelWindow();
		return 0;
	case WM_ERASEBKGND:
		PaintSvod((HDC)wParam);
		return 0;
	}
    return(DefWindowProc(hwnd, msg, wParam, lParam));
}

void ShowSvod()
{
	HWND hPrev=FindWindow(DiarySvodWindow,NULL);
	if (hPrev) return;

    WNDCLASS   wc;

	hBackBrush = CreateSolidBrush(0xffffdb);

	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= DiarySvodWindow;
	wc.hInstance        = hInstance;
	wc.hIcon			= NULL;
	wc.hCursor          = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= hBackBrush;
	wc.style            = NULL;
	wc.lpfnWndProc		= SvodWndProc;
	wc.cbClsExtra       = 0;
	wc.cbWndExtra       = 0;

	if (!RegisterClass(&wc)) return;

	RECT rc;
	SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);

	if ((hSvodWnd = CreateWindowEx(WS_EX_TOOLWINDOW,DiarySvodWindow,
			 "OrgPanel - сводная",
                             WS_POPUP,
                             rc.right-iWidth-5 , rc.bottom-iHeight-5,
                             iWidth, iHeight,
                             NULL, NULL, hInstance, NULL)) == NULL)
		return;

	rgnPoint[0].x=0;				rgnPoint[0].y=10;
	rgnPoint[1].x=0;				rgnPoint[1].y=iHeight-25;
	rgnPoint[2].x=10;				rgnPoint[2].y=iHeight-15;
	rgnPoint[3].x=(iWidth/2)-10;	rgnPoint[3].y=iHeight-15;
	rgnPoint[4].x=(iWidth/2);		rgnPoint[4].y=iHeight;
	rgnPoint[5].x=(iWidth/2)+10;	rgnPoint[5].y=iHeight-15;
	rgnPoint[6].x=iWidth-10;		rgnPoint[6].y=iHeight-15;
	rgnPoint[7].x=iWidth;			rgnPoint[7].y=iHeight-25;
	rgnPoint[8].x=iWidth;			rgnPoint[8].y=10;
	rgnPoint[9].x=iWidth-10;		rgnPoint[9].y=0;
	rgnPoint[10].x=10;				rgnPoint[10].y=0;
	rgn=CreatePolygonRgn(&rgnPoint[0],11,ALTERNATE);
	SetWindowRgn(hSvodWnd, rgn, FALSE);

	//PutOnCenter(hwndMain);

//	DestroyWindow(hwndMain);
//			UnregisterClass(DiaryWindow,hInstance);

/*	hMainFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hWhitePen = CreatePen(PS_SOLID,1,0x0ffffff);
	hGrayPen = CreatePen(PS_SOLID,1,0x808080);
	hBlueBrush=CreateSolidBrush(0xff0000);
	hpnBlack	= CreatePen(PS_SOLID,1,0);
	hWhiteBrush=CreateSolidBrush(0xffffff);
	hSilverBrush=CreateSolidBrush(0xc0c0c0);
	hBlackBrush = CreateSolidBrush(0);
	hBack = LoadBitmap(hInstance,"BACK");
	hpnColor1 = CreatePen(PS_SOLID,1,0x0e0e0e0);

	InitCommonControls();
	hWndToolTip=CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_CHILD | WS_POPUP | TTS_ALWAYSTIP, 
      CW_USEDEFAULT, CW_USEDEFAULT, 10, 10,
      hwndMain, NULL, hInstance, NULL);
	bar=new CToolBar(hwndMain,0,0,29,TRUE,hWndToolTip);
	bar->AddButton("Добавить дело",	 IDC_ADDDELO, "ADDDELO",	TBSTYLE_BUTTON);
	bar->AddButton("Изменить дело",	 IDC_CHDELO, "CHDELO",	TBSTYLE_BUTTON);
	bar->AddButton("Удалить дело",	 IDC_DELDELO, "DELDELO",	TBSTYLE_BUTTON);
	bar->AddButton("Дело выполнено",	 IDC_DELOEXEC, "DELOEXEC",	TBSTYLE_BUTTON);
	bar->AddButton("",	 IDC_ABOUT+200, "",	TBSTYLE_SEP);
	bar->AddButton("Добавить событие",	 IDC_ADDEVENT, "ADDEVENT",	TBSTYLE_BUTTON);
	bar->AddButton("Изменить событие",	 IDC_CHEVENT, "CHEVENT",	TBSTYLE_BUTTON);
	bar->AddButton("Удалить событие",	 IDC_DELEVENT, "DELEVENT",	TBSTYLE_BUTTON);
	bar->AddButton("",	 IDC_ABOUT+201, "",	TBSTYLE_SEP);
	bar->AddButton("Дневник",	 IDC_VIEWDNEV, "VIEWDNEV",	TBSTYLE_BUTTON);
	bar->AddButton("Список дел",		IDC_VIEWDELO, "VIEWDELO",	TBSTYLE_BUTTON);
	bar->AddButton("",	 IDC_ABOUT+203, "",	TBSTYLE_SEP);
	bar->AddButton("О программе",	 IDC_ABOUT, "ABOUT",	TBSTYLE_BUTTON);
	bar->AddButton("Свернуть",	 IDC_MIN, "MIN",	TBSTYLE_BUTTON);

	InitDelaList();

	HFONT hFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);//=(HFONT)SendMessage(hwndMain,WM_GETFONT,0,0);
	LOGFONT lf;
	GetObject(hFont,sizeof(lf),&lf);
	lstrcpy(lf.lfFaceName,"Arial");
	lf.lfCharSet=RUSSIAN_CHARSET;
	lf.lfHeight=45;
	hFont35=CreateFontIndirect(&lf);
	lf.lfHeight=28;
	hFont14=CreateFontIndirect(&lf);

	OnSize();
	InitPraz();
	InitDnev();
	InitPages();
	SetTrayBar();// Должно быть перед InitHuman
	InitHuman();
	InitPrinter();

	if (!bResident)
	{
*/
	ShowWindow(hSvodWnd,SW_SHOW);
	UpdateWindow(hSvodWnd);

/*	DeleteObject(hFont35);
	DeleteObject(hFont14);
	DeleteObject(hBlueBrush);
	DeleteObject(hWhiteBrush);
	DeleteObject(hSilverBrush);
	DeleteObject(hBlackBrush);
	DeleteObject(hpnBlack);
	delete bar;*/
}