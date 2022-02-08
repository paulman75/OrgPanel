#include <windows.h>
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "Pages.h"
#include "Event.h"
#include "../Headers/DynList.h"
#include "res/resource.h"
#include "IntEvent.h"
#include "Days.h"

DynArray<TEVENT> EventList;
extern TPage LeftPage;

TEVENT editev;
extern HWND hwndMain;
extern HINSTANCE hInstance;		  // Global instance handle for application
const char Opov[][13]={"точно в срок","за 1 минуту","за 2 минуты","за 5 минут","за 10 минут","за 15 минут","за 20 минут","за 30 минут","за 45 минут","за 1 час","за 1,5 часа","за 2 часа","за 3 часа","за 4 часа"};
int OpovMin[]={0,1,2,5,10,15,20,30,45,60,90,120,180,240};
extern BOOL    bDnevModif;
int DelDay[]={0,2,5,10,20,30,61,91,182,365,365*2};
extern int	iDeleteEvent;
const char chPeriod[][15]={"ќдин раз","≈жедневно","≈женедельно","≈жемес€чно","≈жегодно"};
#define EV_ONE 0
#define EV_DAY 1
#define EV_WEEK 2
#define EV_MONTH 3
#define EV_YEAR 4

void DnevToScreen(BOOL bSetNoActive);

void AddEvent(PPage pp)
{
	char* temp=(char*)malloc(300);
	for (int i=0; i<EventList.count(); i++)
	{
		TEVENT ev=EventList[i];
		TDate rdate;
		if (GetEventAfterDate(&pp->Date,&ev,&rdate))
		{
			if (rdate==pp->Date)
			{
				wsprintf(temp,"%.02d:%.02d %s",ev.Time.Hour,ev.Time.Min,ev.Text);
				AddStrings(temp,pp,ID_EVENT,(void*)ev.ID,TRUE);
			}
		}
	}
	free(temp);
}

int GetEventByID(int ID, PEVENT pev)
{
	int i=0;
	while (i<EventList.count())
	{
		if (EventList[i].ID==ID)
		{
			*pev=EventList[i];
			return i;
		}
		i++;
	}
	return -1;
}

BOOL FAR PASCAL EventDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	int i;
	char ch[5];
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		for (i=0; i<60; i++)
		{
			wsprintf(ch,"%d",i);
			if (i<24) SendMessage(GetDlgItem(hDlg,IDC_CBHOUR),CB_ADDSTRING,0,(LPARAM)(ch));
			SendMessage(GetDlgItem(hDlg,IDC_CBMIN),CB_ADDSTRING,0,(LPARAM)(ch));
		}
		for (i=0; i<14; i++)
			SendMessage(GetDlgItem(hDlg,IDC_CBMIN2),CB_ADDSTRING,0,(LPARAM)(Opov[i]));
		for (i=0; i<5; i++)
			SendMessage(GetDlgItem(hDlg,IDC_CBPERIOD),CB_ADDSTRING,0,(LPARAM)(chPeriod[i]));
		SendMessage(GetDlgItem(hDlg,IDC_CBHOUR),CB_SETCURSEL,editev.Time.Hour,0);
		SendMessage(GetDlgItem(hDlg,IDC_CBMIN),CB_SETCURSEL,editev.Time.Min,0);
		SendMessage(GetDlgItem(hDlg,IDC_CBMIN2),CB_SETCURSEL,editev.NotifyMin,0);
		SendMessage(GetDlgItem(hDlg,IDC_CBPERIOD),CB_SETCURSEL,editev.Period,0);
		SendMessage(GetDlgItem(hDlg,IDC_CHHOL),BM_SETCHECK,!editev.bHoliday,0);
		SetWindowText(GetDlgItem(hDlg,IDC_EDITEV),editev.Text);
		RECT rc;
		GetClientRect(hDlg,&rc);
		CreateDateLabel(hDlg,rc.right/8*5+10,(int)(rc.bottom*0.15),&(editev.Time.Date),1,FALSE);

		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam)==ID_OK)
		{
			editev.Time.Hour=(char)SendMessage(GetDlgItem(hDlg,IDC_CBHOUR),CB_GETCURSEL,0,0);
			editev.Time.Min=(char)SendMessage(GetDlgItem(hDlg,IDC_CBMIN),CB_GETCURSEL,0,0);
			editev.NotifyMin=SendMessage(GetDlgItem(hDlg,IDC_CBMIN2),CB_GETCURSEL,0,0);
			editev.Period=SendMessage(GetDlgItem(hDlg,IDC_CBPERIOD),CB_GETCURSEL,0,0);
			editev.bHoliday=(BOOL)((SendMessage(GetDlgItem(hDlg,IDC_CHHOL),BM_GETSTATE,0,0)&1)==0);
			GetWindowText(GetDlgItem(hDlg,IDC_EDITEV),editev.Text,199);
			GetDate(&editev.Time.Date,1);
			TDateTime dt;
			dt=NowTime();
			if (editev.Time<dt)
			{
				if (MessageBox(hDlg,"—обытие назначаетс€ на прошедшее врем€. ѕродолжить?","—обытие",MB_OKCANCEL | MB_ICONQUESTION)==IDCANCEL) return TRUE;
			}

			DeleteAllDateLabelsOnWindow(hDlg);
			EndDialog(hDlg, TRUE);
			return (TRUE);
		}
		if (LOWORD(wParam)==IDCANCEL)
		{
			DeleteAllDateLabelsOnWindow(hDlg);
			EndDialog(hDlg, FALSE);
			return (TRUE);
	    }
	}
	return FALSE;
}

BOOL EditEvent(PEVENT pev)
{
	editev=*pev;
	if (!DialogBox(hInstance, "EVENTDLG", hwndMain, (DLGPROC)EventDlgProc)) return FALSE;
	*pev=editev;
	return TRUE;
}

void InsertEvent(PEVENT pev)
{
	bDnevModif=TRUE;
	int i=0;
	while (i<EventList.count())
	{
		TEVENT ev=EventList[i];
		if (ev.Time.Hour>pev->Time.Hour ||
			(ev.Time.Hour==pev->Time.Hour && ev.Time.Min>pev->Time.Min)) break;
		i++;
	}
	EventList.Add(pev,i);
}

void NewEvent()
{
	TEVENT event;
	event.ID=0;
	while (TRUE)
	{
		TEVENT ev;
		if (GetEventByID(event.ID, &ev)==-1) break;
		event.ID++;
	}
	event.Time.Date=LeftPage.Date;
	event.Time.Hour=12;
	event.Time.Min=0;
	event.Text[0]=0;
	event.NotifyMin=0;
	event.bHoliday=TRUE;
	event.Period=0;

	if (!EditEvent(&event)) return;

	InsertEvent(&event);
	UpdateIntEvent();
	DnevToScreen(TRUE);
	PaintDnev();
}

void ChangeEvent()
{
	PPage pp;
	int sti;
	GetActiveNote(&pp,&sti);
	if (!pp) return;
	int id=(int)pp->st[sti].poi;
	TEVENT ev;
	int i=GetEventByID(id,&ev);

	if (!EditEvent(&ev)) return;

	EventList.Del(i);
	InsertEvent(&ev);
	DelIntEvent(ID_EVENT,id);
	UpdateIntEvent();
	DnevToScreen(FALSE);
	PaintDnev();
}

void DeleteEvent()
{
	PPage pp;
	int sti;
	GetActiveNote(&pp,&sti);
	if (!pp) return;
	int id=(int)pp->st[sti].poi;
	DelIntEvent(ID_EVENT,id);
	TEVENT ev;
	int i=GetEventByID(id,&ev);
	EventList.Del(i);
	DnevToScreen(TRUE);
	PaintDnev();
	bDnevModif=TRUE;
}

void DelOldEvent()
{
	if (!DelDay[iDeleteEvent]) return;
	int i=0;
	TDate d=Now();

	DWORD w=EncodeDate(&d);
	while (i<EventList.count())
	{
		TEVENT ev=EventList[i];
		DWORD w2=EncodeDate(&ev.Time.Date);
		if (w2+DelDay[iDeleteEvent]<w && !ev.Period)
			EventList.Del(i);
		else i++;
	}
}

BOOL GetEventAfterDate(PDate pdat, PEVENT pev, PDate evdate)
{
	TDate d=*pdat;
	switch (pev->Period)
	{
	case EV_ONE:
		if (pev->Time.Date<*pdat) return FALSE;
		*evdate=pev->Time.Date;
		break;
	case EV_DAY:
		if (d<pev->Time.Date) d=pev->Time.Date;
		if (!pev->bHoliday) while (GetDayColor(&d)) d++;
		*evdate=d;
		break;
	case EV_MONTH:
		d=pev->Time.Date;
		while (d<*pdat || (!pev->bHoliday && GetDayColor(&d)))
		{
			d.Month++;
			if (d.Month>12)
			{
				d.Month=1;
				d.Year++;
			}
		}
		*evdate=d;
		break;
	case EV_WEEK:
		d=pev->Time.Date;
		while (d<*pdat || (!pev->bHoliday && GetDayColor(&d)))
		DateForward(&d,7);
		*evdate=d;
		break;
	case EV_YEAR:
		d=pev->Time.Date;
		while (d<*pdat || (!pev->bHoliday && GetDayColor(&d)))
		d.Year++;
		*evdate=d;
		break;
	}
	return TRUE;
}
