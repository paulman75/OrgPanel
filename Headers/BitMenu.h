#ifndef __BITMENU_H
#define __BITMENU_H

#include "DynList.h"

#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16

typedef struct
{
	int		ID;
	HBITMAP	hBitmap;
	char	Caption[100];
	HMENU	hMenu;
} TMENUUNIT, *PMENUUNIT;

class CBitMenu
{
private:
	HMENU	hMenu;
	HWND	hWnd;
	DynArray<TMENUUNIT> UnitList;
	void SetOwnerDraw(HMENU hm);
	int FindMenuUnit(HMENU hMenu, int ID, PMENUUNIT pmu);
public:
	void OnMeasureItem(LPMEASUREITEMSTRUCT lpmis);
	void OnDrawItem(LPDRAWITEMSTRUCT lpdis);
	void SetBitmapToItem(HBITMAP hb, int ID);
	CBitMenu(HMENU hm, HWND hOwner);
	~CBitMenu();
};

#endif
