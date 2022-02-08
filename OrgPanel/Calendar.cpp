#include "windows.h"
#define _MYDLL_
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "Calendar.h"
#include "Res\resource.h"

extern HINSTANCE hInstance;
HBITMAP hbi;
extern HFONT	hMainFont;
//На форме
CSensorLabel*	slDate;
CSensorLabel*	slDay[42];
TDate mDate;
int Month;
int Year;

char srMonth[12][10]={"Января","Февраля","Марта","Апреля","Мая","Июня","Июля","Августа","Сентября","Октября","Ноября","Декабря"};
char sDayWeek[7][3]={"ПН","ВТ","СР","ЧТ","ПТ","СБ","ВС"};
char sMonth[12][10]={"Январь","Февраль","Март","Апрель","Май","Июнь","Июль","Август","Сентябрь","Октябрь","Ноябрь","Декабрь"};
char sWeekDay[7][12]={"Понедельник","Вторник","Среда","Четверг","Пятница","Суббота","Воскресенье"};

CDateLabel* FirstLab=NULL;
CDateLabel* lab;
float CalScale;
BOOL bCanNeopr;

void __stdcall CreateDateLabel(HWND hwnd, int x, int y, PDate lpDate, byte ID, BOOL bCanNo)
{
	if (FirstLab)
	{
		lab=FirstLab;
		while (lab->next) lab=lab->next;
		lab->next=new CDateLabel(hwnd,x,y,lpDate,ID,bCanNo);
		lab->next->next=NULL;
	}
	else
	{
		FirstLab=new CDateLabel(hwnd,x,y,lpDate,ID,bCanNo);
		FirstLab->next=NULL;
	}
}

CDateLabel* FindLabel(byte ID)
{
	lab=FirstLab;
	while (lab)
	{
		if (GetWindowLong(lab->GetWnd(),GWL_ID)==ID) return lab;
		lab=lab->next;
	}
	return NULL;
}

void __stdcall ShowDateLabel(byte ID, BOOL bShow)
{
	lab=FindLabel(ID);
	if (!lab) return;
	ShowWindow(lab->GetWnd(),bShow ? SW_SHOW:SW_HIDE);
}

BOOL __stdcall GetDate(PDate lpDate, byte ID)
{
	lab=FindLabel(ID);
	if (!lab) return FALSE;
	*lpDate=lab->Date;
	return TRUE;
}

BOOL __stdcall SetDate(PDate lpDate, byte ID)
{
	lab=FindLabel(ID);
	if (!lab) return FALSE;
	lab->SetDate(lpDate);
	return TRUE;
}

void __stdcall DeleteAllDateLabelsOnWindow(HWND hwnd)
{
	while (TRUE)
	{
		CDateLabel* pr=NULL;
		lab=FirstLab;
		while (lab)
		{
			if (GetParent(lab->GetWnd())==hwnd) break;
			pr=lab;
			lab=lab->next;
		}
		if (!lab) break;
		if (pr)	pr->next=lab->next;
		else FirstLab=lab->next;
		delete lab;
	}
}

void __stdcall DeleteAllDateLabels()
{
	while (FirstLab)
	{
		lab=FirstLab->next;
		delete FirstLab;
		FirstLab=lab;
	}
}

void __stdcall WeekDayToString(LPTSTR St,byte w)
{
	lstrcpy(St,sWeekDay[w-1]);
}

void __stdcall DateToString(PDate Dat,LPTSTR res)
{
	if (Dat->Month)	wsprintf(res,"%d %s %d",Dat->Day,srMonth[Dat->Month-1],Dat->Year);
	else lstrcpy(res,"Не задана");
}

void __stdcall DateToStringWithoutYear(PDate Dat,LPTSTR res)
{
	wsprintf(res,"%d %s",Dat->Day,srMonth[Dat->Month-1]);
}

byte DaysinMonth(byte Month,WORD Year)
{
const byte Days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
if (Month!=2) return (Days[Month-1]);
if (mod(Year,4)==0 && (mod(Year,100)!=0 || mod(Year,400)==0)) return (29);
return (28);
}

void __stdcall MonthToString(LPTSTR St,byte Mon)
{
	lstrcpy(St,sMonth[Mon-1]);
}

void __stdcall YearToString(LPTSTR St, WORD Y)
{
	St[0]=0;
	if (Y>30000) return;
	byte ml=(byte)mod(Y,10);
	wsprintf(St,"%d",Y);
	if ((Y>4 && Y<20) || (ml>4 && ml<10) || ml==0) lstrcat(St," лет");
	else 
	{
		if (ml==1) lstrcat(St," год");
		else lstrcat(St," года");
	}
}

WORD __stdcall mod(DWORD w,WORD d)
{
return (WORD)(w-d*(DWORD)(w/d));
}

DWORD __stdcall EncodeDate(PDate Dat)
{
if (Dat->Year>9000 || Dat->Month>12 || Dat->Day>31) return (0);
int i;
WORD kDay=Dat->Day;
for (i=1; i<=Dat->Month-1; i++) kDay=kDay+DaysinMonth(i,Dat->Year);
i=Dat->Year-1;
return (i*365+(DWORD)(i/4)-(DWORD)(i/100)+(DWORD)(i/400)+kDay-693594);
}

byte __stdcall DayofWeek(PDate dd)
{
DWORD res=EncodeDate(dd)-1;
res=res-7*(DWORD)(res/7);
if (res==0) res=7;
return (byte)res;
}

void __stdcall DateBefore(PDate d,int len)
{
	int v=d->Day-len;
	while (v<=0)
	{
		d->Month--;
		if (d->Month==0)
		{
			d->Month=12;
	        d->Year--;
		}
		v+=DaysinMonth(d->Month,d->Year);
	}
	d->Day=(byte)v;
}

void __stdcall DateForward(PDate d,int len)
{
	int v=d->Day+len;
	while (TRUE)
	{
		byte x=DaysinMonth(d->Month,d->Year);
		if (v<=x) break;
		v-=x;
		d->Month++;
		if (d->Month==13)
		{
			d->Month=1;
			d->Year++;
		}
	}
	d->Day=v;
}

TDate __stdcall Now()
{
SYSTEMTIME SysTim;
GetLocalTime(&SysTim);
TDate	res;
res.Year=SysTim.wYear;
res.Month=(byte)SysTim.wMonth;
res.Day=(byte)SysTim.wDay;
return res;
}

CDateLabel::CDateLabel(HWND Owner,int XCenter, int YCenter, PDate Dat,byte Id, BOOL bCanNo) : CSensorLabel(Owner, XCenter, YCenter, 0xFF0000, 230, NULL,1,Id)
{
	bFCanNo=bCanNo;
	DateToString(Dat,FCaption);
	Date=*Dat;
	UpdateSize();
}

void CDateLabel::SetDate(PDate Dat)
{
	DateToString(Dat,FCaption);
	Date=*Dat;
	UpdateSize();
}

CDateLabel::~CDateLabel()
{
}

DWORD GetDayColor(byte d,byte m,byte w)
{
const byte praz[14]={1,1,8,3,1,5,2,5,9,5,12,6,7,11};

DWORD res;
if (w<6) res=0;
else res=0x0dc;
byte i;
for (i=0; i<=6; i++)
	if (d==praz[i*2] && m==praz[i*2+1]) res=0x0dc;
return (res);
}

void GetFirstDay(PDate dat)
{
	//Вычисляем первую дату в календаре
	dat->Month=Month;
	dat->Year=Year;
	dat->Day=1;
	byte weekday=DayofWeek(dat);
	DateBefore(dat,weekday-1);
}

void UpdateDays(BOOL NeedReDraw)
{
TDate	dd;
GetFirstDay(&dd);
byte weekday=1;
for (int i=0; i<42; i++)
{
	wsprintf(slDay[i]->FCaption,"%d",dd.Day);
	slDay[i]->UpdateSize();
	if (dd.Day==mDate.Day && dd.Month==mDate.Month) slDay[i]->ColorOff=0xF89515;
	else if (dd.Month!=Month)
	     slDay[i]->ColorOff=0x808080;
	else slDay[i]->ColorOff=GetDayColor(dd.Day,dd.Month,(byte)weekday);

	if (!NeedReDraw) InvalidateRect(slDay[i]->hWnd,NULL,TRUE);
	else ShowWindow(slDay[i]->hWnd,SW_SHOW);
	DateForward(&dd,1);
    weekday++;
    if (weekday>7) weekday=1;
}
DateToString(&mDate,slDate->FCaption);
slDate->UpdateSize();
if (NeedReDraw) InvalidateRect(GetParent(slDate->hWnd),NULL,TRUE);
}

BOOL FAR PASCAL CalendarWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	WORD i,y,weekday;
    switch (message) {
	case WM_INITDIALOG:
		RECT rc;
		GetClientRect(hDlg,&rc);
		CalScale=(float)rc.right/131;
		slDate=new CSensorLabel(hDlg,(WORD)(65*CalScale),(WORD)(8*CalScale),0xFF0000,0xFF0000,"dsdsf",1,0);
		char b[6];
		for (i=1900; i<2021; i++)
		{
			wsprintf(b,"%d",i);
			SendMessage(GetDlgItem(hDlg,IDC_COMBO3),CB_ADDSTRING,0,(LPARAM)b);
		}
		SendMessage(GetDlgItem(hDlg,IDC_COMBO3),CB_SETCURSEL,mDate.Year-1900,0);
		for (i=0; i<12; i++)
		{
			SendMessage(GetDlgItem(hDlg,IDC_COMBO2),CB_ADDSTRING,0,(LPARAM)sMonth[i]);
		}
		SendMessage(GetDlgItem(hDlg,IDC_COMBO2),CB_SETCURSEL,mDate.Month-1,0);
		weekday=1;
		y=43;
		for (i=1; i<=42; i++) 
		{
			slDay[i-1]=new CSensorLabel(hDlg,DWORD((weekday*18-7)*CalScale),(DWORD)(y*CalScale),0,0xe0c125,"1",1,i);
			weekday++;
			if (weekday>7) 
			{
				weekday=1;
				y=y+12;
			}
		}
		PutOnCenter(hDlg);
		UpdateDays(FALSE);
		EnableWindow(GetDlgItem(hDlg,ID_NEOPRED),bCanNeopr);
		return (TRUE);
	case WM_SENSORLABELCLICK:
		TDate dd;
		GetFirstDay(&dd);
		byte d;
		d=(byte)GetWindowLong((HWND)wParam,GWL_ID);
		if (d==0) break;
		DateForward(&dd,d-1);
		mDate=dd;
		UpdateDays(FALSE);
		MySound();
		break;
	case WM_ERASEBKGND:
		GetClientRect(hDlg,&rc);
		FillBackGround((HDC)wParam,&rc,hbi);
		return TRUE;
		break;
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hDlg,&ps);
		for (i=0; i<7; i++)
		{
			HFONT hOldFont=(HFONT)SelectObject(ps.hdc,hMainFont);
			if (i<5) SetTextColor(ps.hdc,0);
			else	 SetTextColor(ps.hdc,0x0dc);
			SetBkMode(ps.hdc,TRANSPARENT);
			TextOut(ps.hdc,(WORD)((i*18+5)*CalScale),WORD(30*CalScale),sDayWeek[i],2);
			SelectObject(ps.hdc,hOldFont);
		}
		EndPaint(hDlg,&ps);
		break;
	case WM_COMMAND:
		if (HIWORD(wParam)==CBN_SELCHANGE)
		{
			i=(WORD)SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
			if (LOWORD(wParam)==IDC_COMBO2)
			{
				mDate.Month=i+1;
				Month=mDate.Month;
			}
			else
			{
				mDate.Year=i+1900;
				Year=mDate.Year;
			}
			i=DaysinMonth(mDate.Month,mDate.Year);
			if (mDate.Day>i) mDate.Day=(char)i;
			UpdateDays(TRUE);
			SetFocus(hDlg);
			break;
		}
		if ( LOWORD(wParam)==ID_EXIT || LOWORD(wParam)==IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam)==ID_EXIT);
			return (TRUE);
	    }
		if (LOWORD(wParam)==ID_TODAY) {
			mDate=Now();
			Month=mDate.Month;
			Year=mDate.Year;
			SendMessage(GetDlgItem(hDlg,IDC_COMBO3),CB_SETCURSEL,mDate.Year-1900,0);
			SendMessage(GetDlgItem(hDlg,IDC_COMBO2),CB_SETCURSEL,mDate.Month-1,0);
			UpdateDays(TRUE);
		}
		if (LOWORD(wParam)==ID_NEOPRED)
		{
			mDate.Month=0;
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
	    break;
    }
    return (FALSE);
}

void __stdcall StartCalendar(PDate Dat,HWND Owner, BOOL bCanNo)
{
	hbi=LoadBitmap(hInstance,"CALBACK");
	mDate=*Dat;
	if (!mDate.Month) mDate=Now();
	Month=mDate.Month;
	Year=mDate.Year;
	bCanNeopr=bCanNo;
	BOOL res=DialogBox(hInstance, "CALENDARDIALOG", Owner, (DLGPROC)CalendarWndProc);
	DeleteObject(hbi);
	if (res) *Dat=mDate;
}

void CDateLabel::BeginWindow(HWND Owner)
{
	StartCalendar(&Date,Owner, bFCanNo);
	DateToString(&Date,FCaption);
	UpdateSize();
	InvalidateRect(GetParent(hWnd),NULL,TRUE);
	SendMessage(GetParent(hWnd),WM_CALENDARDATECHANGE,(WPARAM)GetWindowLong(hWnd,GWL_ID),0);
}

LONG FAR PASCAL CDateLabel::SensorProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg==WM_LBUTTONUP && bDownClick) BeginWindow(GetParent(hWnd));

	return (CSensorLabel::SensorProc(msg,wParam,lParam));
}