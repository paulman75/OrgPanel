#include <windows.h>
#include "BitMenu.h"
#include "../Headers/general.h"

void CBitMenu::SetOwnerDraw(HMENU hm)
{
	int count=GetMenuItemCount(hm);
	int i=0;
	while (i<count)
	{
		int ID=GetMenuItemID(hm,i);
		if (ID==-1)	SetOwnerDraw(GetSubMenu(hm,i));
		else if (ID)
		{
			TMENUUNIT mu;
			GetMenuString(hm, i, mu.Caption,100,MF_BYPOSITION);
			mu.hBitmap=NULL;
			mu.hMenu=hm;
			mu.ID=ID;
			UnitList.Add(&mu);
			ModifyMenu(hm,i,MF_BYPOSITION|MF_OWNERDRAW,ID,(LPCTSTR)hm);
		}
		i++;
	}
}

CBitMenu::CBitMenu(HMENU hm, HWND hOwner)
{
	hMenu=hm;
	hWnd=hOwner;
	SetOwnerDraw(hMenu);
}

CBitMenu::~CBitMenu()
{
	UnitList.Clear();
}

void CBitMenu::OnMeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
	TMENUUNIT mu;
	FindMenuUnit((HMENU)lpmis->itemData, lpmis->itemID, &mu);
	HDC dc=GetDC(hWnd);
	SIZE siz;
	GetTextExtentPoint32(dc,mu.Caption,strlen(mu.Caption),&siz);
	ReleaseDC(hWnd,dc);
	lpmis->itemHeight=BITMAP_HEIGHT+2;
	lpmis->itemWidth=BITMAP_WIDTH+siz.cx+5;
}

void CBitMenu::OnDrawItem(LPDRAWITEMSTRUCT lpdis)
{
	FillRect(lpdis->hDC,&lpdis->rcItem, GetSysColorBrush(COLOR_MENU));

	TMENUUNIT mu;
	FindMenuUnit((HMENU)lpdis->itemData, lpdis->itemID, &mu);

	RECT rc=lpdis->rcItem;
	rc.right=rc.left+BITMAP_WIDTH+2;
	RECT rcText=lpdis->rcItem;
	rcText.left=rc.right;
	RECT rcSel=lpdis->rcItem;
	if (mu.hBitmap) rcSel.left=rc.left+BITMAP_WIDTH+3;

	if (mu.hBitmap) DrawTransparentBitmap(lpdis->hDC,lpdis->rcItem.left+1,lpdis->rcItem.top+1, mu.hBitmap,0xffffff);

	SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
	if ((lpdis->itemState & ODS_SELECTED) &&
	(lpdis->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		if (mu.hBitmap) DrawEdge(lpdis->hDC,&rc,BDR_RAISEDINNER ,BF_RECT);
		FillRect(lpdis->hDC,&rcSel, GetSysColorBrush(COLOR_HIGHLIGHT));//hSelBrush);
		SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	SetBkMode(lpdis->hDC,TRANSPARENT);
	rcText.left+=2;

	if (lpdis->itemState & ODS_GRAYED)
	DrawState(lpdis->hDC, NULL, NULL, (LPARAM)mu.Caption, NULL, rcText.left, rcText.top, 0, 0, DST_TEXT |DSS_DISABLED);
	else DrawText(lpdis->hDC,mu.Caption,strlen(mu.Caption),&rcText, DT_LEFT|DT_NOCLIP|DT_SINGLELINE|DT_VCENTER);
}

int CBitMenu::FindMenuUnit(HMENU hMenu, int ID, PMENUUNIT pmu)
{
	int i=0;
	while (i<UnitList.count())
	{
		*pmu=UnitList[i];
		if ((pmu->hMenu==hMenu || !hMenu) && pmu->ID==ID) return i;
		i++;
	}
	return -1;
}

void CBitMenu::SetBitmapToItem(HBITMAP hb, int ID)
{
	TMENUUNIT mu;
	int index=FindMenuUnit(NULL,ID,&mu);
	if (index==-1) return;
	mu.hBitmap=hb;
	UnitList[index].hBitmap=hb;
}
