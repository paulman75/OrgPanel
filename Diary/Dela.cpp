#include <windows.h>
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "res/resource.h"
#include "Pages.h"
#include "Days.h"
#include "../Headers/DynList.h"
#include "Dela.h"
#include "IntEvent.h"
#include "DelaList.h"

DynArray<TDelo> DeloList;
TDelo editdl;

#define DL_BEGIN 1
#define DL_END	2
#define DL_FACT	3

extern HWND hwndMain;
extern HINSTANCE hInstance;		  // Global instance handle for application
extern TPage LeftPage;
extern BOOL    bDnevModif;
extern int DelDay[];
extern int iDeleteDelo;
extern TMode	Mode;
extern int ActiveIDInListView;

void DnevToScreen(BOOL bSetNoActive);

void AddDelo(PPage pag)
{
	for (int i=0; i<DeloList.count(); i++)
	{
		TDelo dl=DeloList[i];
		TDate rdate;
		BOOL bNormalColor;
		if (GetDeloAfterDate(&pag->Date,&dl,&rdate,&bNormalColor))
		{
			byte ColorType=0;
			if (bNormalColor) ColorType=1;
			if (pag->Date==dl.factDate) ColorType=2;
			if (rdate==pag->Date)
				AddStrings(dl.Text,pag,ID_DELA,(void*)dl.ID,ColorType);
		}
	}
}

int GetDeloByID(int ID, PDelo pdl)
{
	int i=0;
	while (i<DeloList.count())
	{
		if (DeloList[i].ID==ID)
		{
			*pdl=DeloList[i];
			return i;
		}
		i++;
	}
	return -1;
}

void UpdateEnableAlarm(HWND hDlg)
{
	BOOL b=(BOOL)(SendMessage(GetDlgItem(hDlg,IDC_RDDELO1),BM_GETSTATE,0,0)&1);
	EnableWindow(GetDlgItem(hDlg,IDC_CBDELOHOUR),!b);
	EnableWindow(GetDlgItem(hDlg,IDC_CBDELOMIN),!b);
}

BOOL FAR PASCAL DeloDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	int i;
	char ch[5];
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		for (i=0; i<60; i++)
		{
			wsprintf(ch,"%d",i);
			if (i<24) SendMessage(GetDlgItem(hDlg,IDC_CBDELOHOUR),CB_ADDSTRING,0,(LPARAM)(ch));
			SendMessage(GetDlgItem(hDlg,IDC_CBDELOMIN),CB_ADDSTRING,0,(LPARAM)(ch));
		}
		SendMessage(GetDlgItem(hDlg,IDC_CBDELOHOUR),CB_SETCURSEL,editdl.Hour,0);
		SendMessage(GetDlgItem(hDlg,IDC_CBDELOMIN),CB_SETCURSEL,editdl.Min,0);
		SendMessage(GetDlgItem(hDlg,IDC_CHDELOHOL),BM_SETCHECK,!editdl.bHoliday,0);

		RECT rc;
		GetClientRect(hDlg,&rc);
		CreateDateLabel(hDlg,rc.right/8*6+10,(int)(rc.bottom*0.06),&(editdl.begDate),DL_BEGIN,FALSE);
		CreateDateLabel(hDlg,rc.right/8*6+10,(int)(rc.bottom*0.12),&(editdl.endDate),DL_END,FALSE);
		CreateDateLabel(hDlg,rc.right/8*6+10,(int)(rc.bottom*0.18),&(editdl.factDate),DL_FACT,TRUE);

		SetWindowText(GetDlgItem(hDlg,IDC_EDITDELOTEXT),editdl.Text);
		SendMessage(GetDlgItem(hDlg,IDC_RDDELO1),BM_SETCHECK,!editdl.NeedAlarm,0);
		SendMessage(GetDlgItem(hDlg,IDC_RDDELO2),BM_SETCHECK,editdl.NeedAlarm==1,0);
		SendMessage(GetDlgItem(hDlg,IDC_RDDELO3),BM_SETCHECK,editdl.NeedAlarm==2,0);
		UpdateEnableAlarm(hDlg);

		return TRUE;
	case WM_CALENDARDATECHANGE:
		if (wParam==DL_BEGIN)
		{
			GetDate(&editdl.begDate,DL_BEGIN);
			GetDate(&editdl.endDate,DL_END);
			if (editdl.endDate<editdl.begDate)
			{
				editdl.endDate=editdl.begDate;
				SetDate(&editdl.endDate,DL_END);
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RDDELO1:
		case IDC_RDDELO2:
		case IDC_RDDELO3:
			UpdateEnableAlarm(hDlg);
			break;
		case IDOK:
			editdl.Hour=(char)SendMessage(GetDlgItem(hDlg,IDC_CBDELOHOUR),CB_GETCURSEL,0,0);
			editdl.Min=(char)SendMessage(GetDlgItem(hDlg,IDC_CBDELOMIN),CB_GETCURSEL,0,0);
			editdl.bHoliday=(BOOL)((SendMessage(GetDlgItem(hDlg,IDC_CHDELOHOL),BM_GETSTATE,0,0)&1)==0);
			GetDate(&editdl.begDate,DL_BEGIN);
			GetDate(&editdl.endDate,DL_END);
			GetDate(&editdl.factDate,DL_FACT);
			GetWindowText(GetDlgItem(hDlg,IDC_EDITDELOTEXT),editdl.Text,199);

			editdl.NeedAlarm=0;
			if (SendMessage(GetDlgItem(hDlg,IDC_RDDELO2),BM_GETSTATE,0,0)&1) editdl.NeedAlarm=1;
			if (SendMessage(GetDlgItem(hDlg,IDC_RDDELO3),BM_GETSTATE,0,0)&1) editdl.NeedAlarm=2;
			if (editdl.begDate>editdl.endDate ||
				(editdl.begDate>editdl.factDate && editdl.factDate.Month))
			{
				MessageBox(hDlg,"Неверное задание дат","Ошибка",MB_OK | MB_ICONSTOP);
				break;
			}
			if (!lstrlen(editdl.Text))
			{
				MessageBox(hDlg,"Необходимо ввести текст сообщения","Ошибка",MB_OK | MB_ICONSTOP);
				break;
			}
			DeleteAllDateLabelsOnWindow(hDlg);
			EndDialog(hDlg, TRUE);
			return (TRUE);
		break;
		case IDCANCEL:
			DeleteAllDateLabelsOnWindow(hDlg);
			EndDialog(hDlg, FALSE);
			return (TRUE);
		}
	}
	return FALSE;
}

BOOL EditDelo(PDelo pdl)
{
	editdl=*pdl;
	if (!DialogBox(hInstance, "DELODLG", hwndMain, (DLGPROC)DeloDlgProc)) return FALSE;
	*pdl=editdl;
	return TRUE;
}

void DeloExec()
{
	int id;
	if (Mode==mDnev)
	{
		PPage pp;
		int sti;
		GetActiveNote(&pp,&sti);
		if (!pp) return;
		id=(int)pp->st[sti].poi;
	}
	else id=ActiveIDInListView;
	TDelo dl;
	int i=GetDeloByID(id,&dl);
	TDate dd=Now();
	if (dl.begDate>dd)
	{
		MessageBox(hwndMain,"Дата выполнения не может быть меньше даты начала выполнения дела","Ошибка",MB_OK | MB_ICONSTOP);
		return;
	}
	DeloList[i].factDate=dd;
	UpdateIntEvent();

	DnevToScreen(FALSE);
	UpdateDelaList();
	PaintDnev();
	bDnevModif=TRUE;
}

void InsertDelo(PDelo pdl)
{
	bDnevModif=TRUE;
	DeloList.Add(pdl);
}

void NewDelo()
{
	TDelo delo;
	delo.ID=0;
	while (TRUE)
	{
		TDelo dl;
		if (GetDeloByID(delo.ID, &dl)==-1) break;
		delo.ID++;
	}
	delo.begDate=LeftPage.Date;
	delo.bHoliday=FALSE;
	delo.endDate=delo.begDate;
	delo.factDate.Month=0;
	delo.Hour=12;
	delo.Min=0;
	delo.NeedAlarm=0;
	delo.Text[0]=0;

	if (!EditDelo(&delo)) return;

	InsertDelo(&delo);
	UpdateIntEvent();
	DnevToScreen(TRUE);
	UpdateDelaList();
	PaintDnev();
}

void ChangeDelo()
{
	int id;
	if (Mode==mDnev)
	{
		PPage pp;
		int sti;
		GetActiveNote(&pp,&sti);
		if (!pp) return;
		id=(int)pp->st[sti].poi;
	}
	else id=ActiveIDInListView;
	TDelo dl;
	int i=GetDeloByID(id,&dl);

	if (!EditDelo(&dl)) return;

	DeloList[i]=dl;
	DelIntEvent(ID_DELA,id);
	UpdateIntEvent();
	DnevToScreen(FALSE);
	UpdateDelaList();
	PaintDnev();
	bDnevModif=TRUE;
}

void DeleteDelo()
{
	int id;
	if (Mode==mDnev)
	{
		PPage pp;
		int sti;
		GetActiveNote(&pp,&sti);
		if (!pp) return;
		id=(int)pp->st[sti].poi;
	}
	else id=ActiveIDInListView;

	DelIntEvent(ID_DELA,id);
	TDelo dl;
	int i=GetDeloByID(id,&dl);
	DeloList.Del(i);
	DnevToScreen(TRUE);
	UpdateDelaList();
	PaintDnev();
	bDnevModif=TRUE;
}

void DelOldDelo()
{
	if (!DelDay[iDeleteDelo]) return;
	int i=0;
	TDate d=Now();

	DWORD w=EncodeDate(&d);
	while (i<DeloList.count())
	{
		TDelo dl=DeloList[i];
		if (!dl.factDate.Month)
		{
			i++;
			continue;
		}
		TDate d=dl.endDate;
		if (dl.factDate>d) d=dl.factDate;
		DWORD w2=EncodeDate(&d);
		if (w2+DelDay[iDeleteDelo]<w)
			DeloList.Del(i);
		else i++;
	}
}

BOOL GetDeloAfterDate(PDate pdat, PDelo pdl, PDate dldate, BOOL* bND)
{
	TDate d=*pdat;
	if (d<pdl->begDate) d=pdl->begDate;

	if (!pdl->bHoliday) while (GetDayColor(&d)) d++;

	*dldate=d;
	*bND=TRUE;

	if (pdl->factDate.Month)
	{
		if (d<=pdl->endDate && d<=pdl->factDate) return TRUE;
		if (d>pdl->endDate && d<=pdl->factDate)
		{
			*bND=FALSE;
			return TRUE;
		}
	}
	else
	{
		if (d<=Now() || (d>Now() && d<=pdl->endDate))
		{
			*bND=d<=pdl->endDate;
			return TRUE;
		}
	}
	return FALSE;
}
