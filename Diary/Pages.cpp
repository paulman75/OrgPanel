#include <windows.h>
#include "../Headers/general.h"
#include "Pages.h"
#include "Days.h"
#include "../Headers/Bitmap.h"
#include "res/resource.h"
#include "../Headers/const.h"
#include "Praz.h"
#include "Human.h"
#include "Event.h"
#include "Pravos.h"
#include "Dela.h"

extern HWND hwndMain;
extern HPEN	hpnColor1,hWhitePen,hGrayPen,hpnBlack;
extern HBRUSH	hBlueBrush,hWhiteBrush,hSilverBrush,hBlackBrush;
extern HBITMAP	hBack;
extern HINSTANCE hInstance;		  // Global instance handle for application
HBITMAP	hCal;
HICON	hSky;
HICON	hMoon[4];
extern HFONT	hFont35,hFont14;
extern HFONT	hMainFont;
extern double	GeoD;
extern double	GeoS;
extern TMode	Mode;
extern HWND		hListView;

#define BARHEIGHT 37

TPage LeftPage, RightPage;
int PageWidth;
extern int		TopHeight;
extern BOOL		bSkyInfo;
extern char	Order[DNEV_COUNT];
CMyBitmap*	Image[DNEV_COUNT];

void UpdateToolBar();

void AddStrings(LPTSTR s,PPage pp,byte sType,LPVOID poi, byte ColorType)
{
	int maxw=PageWidth-50;
	//вычисляем индекс нового сообщения
	int	iIndex=0;
	if (pp->CountSt>0) iIndex=pp->st[pp->CountSt-1].Index+1;

	char ss[100]="";
	char* temp=(char*)malloc(1000);
	ZeroMemory(temp,1000);
	int i;
	i=lstrlen(s)-1;
	while (i>=0)
	{
		if ((byte)s[i]<32) s[i]=32;
		i--;
	}
	i=0;
	int si=1;
	HDC		dc=GetDC(hwndMain);
	while (i<lstrlen(s))
	{
        temp[lstrlen(temp)]=s[i];
        SIZE siz;
		HFONT hOldFont=(HFONT)SelectObject(dc,hMainFont);
		GetTextExtentPoint32(dc,temp,lstrlen(temp),&siz);
		SelectObject(dc,hOldFont);
		if (siz.cx>=maxw || i==lstrlen(s)-1)
		{
                if (lstrlen(ss)==0 || i==lstrlen(s)-1)
				{
                        lstrcpy(ss,temp);
                        si=i;
                }
				if (pp->CountSt<60)
				{
	                PStrings	Sstr=&pp->st[pp->CountSt++];
		            Sstr->poi=poi;
			        Sstr->Text=ss;
					Sstr->Index=iIndex;
					Sstr->IconType=sType;
					Sstr->ColorType=ColorType;
				}
                ZeroMemory(temp,1000);
				if (i<lstrlen(s)-1) si--;
				i=si;
                ss[0]=0;
                maxw=PageWidth-30;
		}
        else
		{
                if (s[i]==32)
				{
                        lstrcpy(ss,temp);
                        si=i+1;
                }
        }
        i++;
	}
	free(temp);
}

void AddPrim(PPage pp)
{
	int i=0;
	while (TRUE)
	{
		LPTSTR lpPrim=FindPrim(&pp->Date,i++);
		if (!lpPrim) break;
		AddStrings(lpPrim,pp,ID_PRIM,NULL,1);
	}
}

void AddPraz(PPage pp)
{
	int i=0;
	while (TRUE)
	{
		LPTSTR lpPrim=FindPraz(&pp->Date,i++);
		if (!lpPrim) break;
		AddStrings(lpPrim,pp,ID_PRAZ,NULL,1);
	}
}

void UpdatePage(PPage pag, BOOL bSetNoActive)
{
	pag->CountSt=0;
	if (bSetNoActive)
	{
		pag->TopStIndex=0;
		pag->ActiveIndex=-1;
	}
	for (byte i=0; i<DNEV_COUNT; i++)
	switch (Order[i]-100)
	{
		case ID_DELA:   AddDelo(pag);	break;
		case ID_IMEN:   AddImen(pag);	break;
		case ID_ROV:    AddRov(pag);	break;
		case ID_PRAZ:   AddPraz(pag);	break;
		case ID_PRIM:	AddPrim(pag);	break;
		case ID_EVENT:	AddEvent(pag);	break;
		case ID_PRAV:	AddPrav(pag);	break;
	}
}

void DrawBackPages(HDC dc,byte kol,WORD x,int h, int w)
{
	byte j;
	int y=h;
	for (j=0; j<kol; j++)
	{
		POINT Apoi[4];
		Apoi[0].x=x;		Apoi[0].y=y;
		Apoi[1].x=x+w;		Apoi[1].y=Apoi[0].y;
		Apoi[2].x=x+w-1;	Apoi[2].y=y+3;
		Apoi[3].x=x+1;		Apoi[3].y=y+3;
		Polygon(dc,Apoi,4);
		x++;
		w-=2;
		y+=3;
	}
}

void PaintPage(HDC dc, PPage pag, int h)
{
	int i=DayofWeek(&pag->Date);
	char ch[50];

	HFONT hOldFont=(HFONT)SelectObject(dc,hFont14);
	WeekDayToString(ch,i);
	char xd=(pag==&LeftPage) ? 5 : -5;
	SetTextColor(dc,GetDayColor(&pag->Date));
	DrawAlignText(dc,pag->x0+(PageWidth>>1)+xd,45,ch,TRUE);

	SelectObject(dc,hFont35);

	wsprintf(ch,"%d",pag->Date.Year);
	SetTextColor(dc,0xf5f5f5);
	DrawAlignText(dc,pag->x0+(PageWidth>>1),TopHeight,ch,TRUE);

	SIZE siz;

	GetTextExtentPoint32(dc,ch,lstrlen(ch),&siz);
	DrawAlignText(dc,pag->x0+(PageWidth>>1),h-siz.cy-30,ch,TRUE);

	TDate dd=Now();
	if (dd==pag->Date)
	{
		SetTextColor(dc,0xe8e8b8);
		TextOut(dc,pag->x0+10,20,"*",1);
	}
	
	wsprintf(ch,"%d",pag->Date.Day);
	GetTextExtentPoint32(dc,ch,lstrlen(ch),&siz);

	SetTextColor(dc,0);
	SelectObject(dc,hFont14);
	char ch2[15];
	MonthToString(ch2,pag->Date.Month);
	SIZE siz2;
	GetTextExtentPoint32(dc,ch2,lstrlen(ch2),&siz2);
	DrawAlignText(dc,pag->x0+(PageWidth>>1)+(siz.cx+siz2.cx+5)/2,10+(siz.cy-siz2.cy)/2,ch2,FALSE);

	SetTextColor(dc,GetDayColor(&pag->Date));
	SelectObject(dc,hFont35);
	TextOut(dc,pag->x0+(PageWidth>>1)-(siz.cx+siz2.cx+5)/2,10,ch,lstrlen(ch));

	if (bSkyInfo)
	{
		int m1=(int)((pag->Sun1-(int)pag->Sun1)*60);
		int m2=(int)((pag->Sun2-(int)pag->Sun2)*60);

		if (pag->Sun1==-2) strcpy(ch,"Полярная ночь");
		else
		if (pag->Sun1==-3) strcpy(ch,"Полярный день");
		else
		{
			strcpy(ch,"Восход: ");
			if (pag->Sun1>=0) wsprintf(ch,"%s%.02d:%.02d",ch,(int)pag->Sun1,m1);
			else strcat(ch,"-");
			strcat(ch,", Заход: ");
			if (pag->Sun2>=0) wsprintf(ch,"%s%.02d:%.02d",ch,(int)pag->Sun2,m2);
			else strcat(ch,"-");
		}
		SetTextColor(dc,0);
		SelectObject(dc,hMainFont);
		GetTextExtentPoint32(dc,ch,lstrlen(ch),&siz);
		DrawIcon(dc,pag->x0+(PageWidth-(26+siz.cx))/2,75,hSky);
		TextOut(dc,pag->x0+(PageWidth-(26+siz.cx))/2+26,77,ch,lstrlen(ch));

		m1=(int)((pag->Moon1-(int)pag->Moon1)*60);
		m2=(int)((pag->Moon2-(int)pag->Moon2)*60);

		strcpy(ch,"Восход: ");
		if (pag->Moon1>=0) wsprintf(ch,"%s%.02d:%.02d",ch,(int)pag->Moon1,m1);
		else strcat(ch,"-");
		strcat(ch,", Заход: ");
		if (pag->Moon2>=0) wsprintf(ch,"%s%.02d:%.02d",ch,(int)pag->Moon2,m2);
		else strcat(ch,"-");

		GetTextExtentPoint32(dc,ch,lstrlen(ch),&siz);
		DrawIcon(dc,pag->x0+(PageWidth-(26+siz.cx))/2,97,hMoon[pag->MoonPhase]);
		TextOut(dc,pag->x0+(PageWidth-(26+siz.cx))/2+26,97,ch,lstrlen(ch));

		if (pag->PhaseTime>=0)
		{
			switch (pag->MoonPhase)
			{
			case Newmoon: strcpy(ch,"Новолуние");
				break;
			case FirstQuarter: strcpy(ch,"Первая четверть");
				break;
			case Fullmoon: strcpy(ch,"Полная луна");
				break;
			case LastQuarter: strcpy(ch,"Последняя четверть");
				break;
			}
			int m,h;
			h=(int)pag->PhaseTime;
			m=(int)((pag->PhaseTime-h)*60);
			wsprintf(ch,"%s %.02d:%.02d",ch,h,m);
			DrawAlignText(dc,pag->x0+(PageWidth>>1),113,ch,TRUE);
		}
	}
	HPEN hOldPen=(HPEN)SelectObject(dc,hpnBlack);
	int y=TopHeight+20;
	while (y<h-40)
	{
		MoveToEx(dc,pag->x0+pag->Tab,y,NULL);
		LineTo(dc,pag->x0+pag->Tab+PageWidth-25,y);
		y+=17;
	}
	y=TopHeight+6;
	int j=pag->TopStIndex;
	int CurInd=-1;
	SelectObject(dc,hMainFont);
	while (j<pag->CountSt && y<h-54)
	{
		int w=0;
		if (CurInd!=pag->st[j].Index)
		{
			CurInd=pag->st[j].Index;
			if (j==0 || pag->st[j-1].Index!=CurInd)
			{
				BitBlt(dc,pag->x0+pag->Tab-3,y-2,20,17,Image[pag->st[j].IconType]->hdc,0,0,SRCCOPY);
				w=17;
			}
		}
		switch (pag->st[j].ColorType)
		{
		case 1: if (CurInd==pag->ActiveIndex) SetTextColor(dc,0xff0000);
				else SetTextColor(dc,0);
				break;
		case 0: if (CurInd==pag->ActiveIndex) SetTextColor(dc,0x4040ff);
				else SetTextColor(dc,0x2020cf);
				break;
		case 2: if (CurInd==pag->ActiveIndex) SetTextColor(dc,0xDDDD00);
				else SetTextColor(dc,0x17a001);
				break;
		}
        TextOut(dc,pag->x0+pag->Tab+w+1,y,pag->st[j].Text.Text,lstrlen(pag->st[j].Text.Text));
		j++;
		y+=17;
	}
	pag->TopBut=pag->TopStIndex>0;
	pag->BotBut=(j<pag->CountSt);
	SelectObject(dc,hBlackBrush);
	POINT Apoi[3];
	if (pag->TopBut)
	{
		Apoi[0].x=pag->x0+(PageWidth>>1)+25;	Apoi[0].y=h-36;
		Apoi[1].x=pag->x0+(PageWidth>>1)+20;	Apoi[1].y=h-42;
		Apoi[2].x=pag->x0+(PageWidth>>1)+15;	Apoi[2].y=Apoi[0].y;
		Polygon(dc,Apoi,3);
	}
	if (pag->BotBut)
	{
		Apoi[0].x=pag->x0+(PageWidth>>1)-25;	Apoi[0].y=h-42;
		Apoi[1].x=pag->x0+(PageWidth>>1)-20;	Apoi[1].y=h-36;
		Apoi[2].x=pag->x0+(PageWidth>>1)-15;	Apoi[2].y=Apoi[0].y;
		Polygon(dc,Apoi,3);
	}
}

void PaintBack(HDC dc)
{
	RECT PaintRect;
	GetClientRect(hwndMain,&PaintRect);
	PaintRect.top+=BARHEIGHT;
	FillBackGround(dc,&PaintRect,hBack);
}

void PaintDnev()
{
	HDC dc=GetDC(hwndMain);

	if (Mode==mDelo)
	{
		SelectObject(dc,hMainFont);
		SetBkMode(dc,TRANSPARENT);
		TextOut(dc,10,47,"Список дел с",12);
		TextOut(dc,180,47,"по",2);
		ReleaseDC(hwndMain,dc);
		return;
	}
	RECT PaintRect;
	GetClientRect(hwndMain,&PaintRect);

	int w,h;
	w=PaintRect.right-PaintRect.left;
	h=PaintRect.bottom-PaintRect.top-BARHEIGHT;

	RECT FRect;
	FRect.left=FRect.top=0;
	FRect.bottom=h;
	FRect.right=w;

	HDC hBitdc=CreateCompatibleDC(dc);
	HANDLE hBit=CreateCompatibleBitmap(dc,w,h);
	SelectObject(hBitdc,hBit);

	FillBackGround(hBitdc,&FRect,hBack);

	HBRUSH hOldBrush=(HBRUSH)SelectObject(hBitdc,hBlueBrush);
	HPEN hOldPen=(HPEN)SelectObject(hBitdc,hpnBlack);
	POINT Apoi[5];
	Apoi[0].x=10;			Apoi[0].y=10;
	Apoi[1].x=w-15;			Apoi[1].y=Apoi[0].y;
	Apoi[2].x=Apoi[1].x+5;			Apoi[2].y=Apoi[0].y+5;
	Apoi[3].x=Apoi[2].x;			Apoi[3].y=h-10;
	Apoi[4].x=Apoi[0].x;			Apoi[4].y=Apoi[3].y;
	Polygon(hBitdc,Apoi,5);

	SelectObject(hBitdc,hWhiteBrush);
	Rectangle(hBitdc,10,10,w-15,h-30);
	HBRUSH hTopBrush=CreateSolidBrush(0xffffdb);
	SelectObject(hBitdc,hTopBrush);
	Rectangle(hBitdc,10,10,w-15,TopHeight);
	DeleteObject(hTopBrush);

	MoveToEx(hBitdc,10+PageWidth,10,NULL);
	LineTo(hBitdc,10+PageWidth,h-30);
	byte i=(LeftPage.Date.Month-1)/2;
	SelectObject(hBitdc,hSilverBrush);
	DrawBackPages(hBitdc,i,10,h-30,PageWidth);
	DrawBackPages(hBitdc,5-i,10+PageWidth,h-30,PageWidth);
	HDC dc2=CreateCompatibleDC(hBitdc);
	SelectObject(dc2,hCal);
	BitBlt(hBitdc,10+PageWidth,10,11,13,dc2,0,0,SRCCOPY);
	DeleteDC(dc2);
	SelectObject(hBitdc,hBlackBrush);
	int h2=(h-TopHeight-40)/3+TopHeight;
	Ellipse(hBitdc,PageWidth-5,h2,PageWidth+7,h2+12);
	Ellipse(hBitdc,PageWidth+13,h2,PageWidth+25,h2+12);
	h2=(h-TopHeight-40)/3*2+TopHeight;
	Ellipse(hBitdc,PageWidth-5,h2,PageWidth+7,h2+12);
	Ellipse(hBitdc,PageWidth+13,h2,PageWidth+25,h2+12);

	Apoi[0].x=w-25;		Apoi[0].y=60;
	Apoi[1].x=w-30;		Apoi[1].y=65;
	Apoi[2].x=Apoi[1].x;			Apoi[2].y=55;
	Polygon(hBitdc,Apoi,3);
	Apoi[0].x=18;	Apoi[1].x=23;	Apoi[2].x=Apoi[1].x;
	Polygon(hBitdc,Apoi,3);
	SetBkMode(hBitdc,TRANSPARENT);
	PaintPage(hBitdc,&LeftPage,h);
	PaintPage(hBitdc,&RightPage,h);
	
	SelectObject(hBitdc,hOldBrush);
	SelectObject(hBitdc,hOldPen);
	BitBlt(dc,0,BARHEIGHT,PaintRect.right-PaintRect.left,PaintRect.bottom,hBitdc,0,0,SRCCOPY);
	DeleteObject(hBit);
	DeleteDC(hBitdc);
	ReleaseDC(hwndMain,dc);
}

void OnSize()
{
	RECT PaintRect;
	GetClientRect(hwndMain,&PaintRect);
	MoveWindow(hListView,10,70,PaintRect.right-20,PaintRect.bottom-85,TRUE);

	int pg=PageWidth;
	PageWidth=PaintRect.right-PaintRect.left;
	PageWidth=(PageWidth-25)>>1;
	LeftPage.x0=10;
	RightPage.x0=10+PageWidth;
	if (pg!=PageWidth && pg!=0)
	{
		UpdatePage(&LeftPage, FALSE);
		UpdatePage(&RightPage, FALSE);
		UpdateToolBar();
	}
	RedrawWindow(hwndMain,NULL,NULL,RDW_INVALIDATE);
}

void CalcAstro(TPage* pag)
{
	Sunrise(&pag->Date, GeoS, GeoD, &pag->Sun1, &pag->Sun2);
	MoonRise(&pag->Date, GeoS, GeoD, &pag->Moon1, &pag->Moon2);
	TDate dd;
	double tim;
	pag->MoonPhase=MoonPhase(&pag->Date,&dd,&tim);
	if (dd==pag->Date) pag->PhaseTime=tim;
	else pag->PhaseTime=-1;
}

void InitPages()
{
	hCal = LoadBitmap(hInstance,"Cal");
	hSky = LoadIcon(hInstance,"SKYICON");
	hMoon[0]= LoadIcon(hInstance,"MOONICON0");
	hMoon[1]= LoadIcon(hInstance,"MOONICON1");
	hMoon[2]= LoadIcon(hInstance,"MOONICON2");
	hMoon[3]= LoadIcon(hInstance,"MOONICON3");
	Image[ID_DELA]=new CMyBitmap(IDB_BITDELA);
	Image[ID_PRIM]=new CMyBitmap(IDB_BITPRIM);
	Image[ID_ROV]=new CMyBitmap(IDB_BITROV);
	Image[ID_PRAZ]=new CMyBitmap(IDB_BITPRAZ);
	Image[ID_IMEN]=new CMyBitmap(IDB_BITIMEN);
	Image[ID_EVENT]=new CMyBitmap(IDB_BITEVENT);
	Image[ID_PRAV]=new CMyBitmap(IDB_BITPRAV);
}

void FreePages()
{
	DeleteObject(hSky);
	DeleteObject(hMoon[0]);
	DeleteObject(hMoon[1]);
	DeleteObject(hMoon[2]);
	DeleteObject(hMoon[3]);
	DeleteObject(hCal);

	for (int i=0; i<DNEV_COUNT; i++) delete Image[i];
}

void ActiveUnitFullView()
{
	RECT rc;
	GetClientRect(hwndMain,&rc);
	int h=rc.bottom-rc.top;
	int c=(h-80-TopHeight)/17;
	int ai=-1;
	PPage pp;
	GetActiveNote(&pp,&ai);
	if (ai-pp->TopStIndex<0) pp->TopStIndex=ai;
	else 
	{
		while (ai<pp->CountSt && pp->st[ai].Index==pp->ActiveIndex) ai++;
		ai--;
		if (ai-pp->TopStIndex>=c) pp->TopStIndex=ai-c+1;
	}
}

BOOL SetActiveNote(WORD x,WORD y, int h)
{
	y=(y-TopHeight-6)/17-2;
	PPage pp;
	LeftPage.ActiveIndex=-1;
	RightPage.ActiveIndex=-1;

	if (x<PageWidth+10) pp=&LeftPage;
	else pp=&RightPage;

	y+=pp->TopStIndex;
	if (y>=pp->CountSt) return FALSE;

	pp->ActiveIndex=pp->st[y].Index;
	ActiveUnitFullView();
	MySound();
	PaintDnev();
	UpdateToolBar();
	return TRUE;
}

void GetActiveNote(PPage *ppp, int* StIndex)
{
	*ppp=NULL;
	if (LeftPage.ActiveIndex!=-1) *ppp=&LeftPage;
	if (RightPage.ActiveIndex!=-1) *ppp=&RightPage;

	if (*ppp)
	{
		//Есть активная строка
		*StIndex=0;
		while (*StIndex<(*ppp)->CountSt && (*ppp)->st[*StIndex].Index!=(*ppp)->ActiveIndex) (*StIndex)++;
	}
}
