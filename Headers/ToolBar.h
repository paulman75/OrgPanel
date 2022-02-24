#ifndef __TOOLBAR_H
#define __TOOLBAR_H

#include "MemBit.h"

typedef struct CToolButton
{
	char		ButType;
	WORD		ID;
	WORD		x,y;
	WORD		Width;
	LPTSTR		Tips;
	CMemoryBitmap*		hBit;
	CMemoryBitmap*		hBit2;
	LPTSTR		strMenu;
	BOOL		Visible;
	BOOL		Enable;
	char		State; //2- NO, 0- Above, 1 - Press
	BOOL		MenuVis;
	CToolButton	*Next;
} CToolButton,*PToolButton;

class CToolBar
{
private:
	PToolButton			FirstButton;
	PToolButton			ActiveButton;
	PToolButton			ClickButton;
	HWND hWnd;
	HWND hToolTipWnd;
	char DefButw;
	char FButh;
	char FirstPos;
	char KolTips;
	WORD TimerID;
	BOOL Pressed;
	char FDelta;
	BOOL FShowCaption;
	HBITMAP	hBackBitmap;
	CMemoryBitmap*	hBackAll;
	CMemoryBitmap*	hMenuBitmap;
	BOOL	MenuShowed;
	PToolButton DetectButton(int x,int y);
	void LButtonDown(int x,int y);
	void DeleteToolTips();
	void MouseMove(PToolButton b,BOOL Always=FALSE);
	void TimerOn();
	void DrawToolBar(PAINTSTRUCT *ps);
	void DrawButton(PToolButton b, HDC hdc);
	void DrawSepLine(HDC hdc,HPEN hPen,PToolButton b,char d);
	void PaintBitmap(CMemoryBitmap* b, CMemoryBitmap* Mask, HDC hdc, LONG xc, LONG yc, BOOL Enable);
	void UpdateBar();
	void Update2Bitmap(PToolButton b);
	void CreateMenuButtonImage(CMemoryBitmap* b);
	void GetButtonRect(PToolButton b, LPRECT rc);
	void FindButton(WORD ID,PToolButton *ppBut, PToolButton *ppPrev);
public:
    static LRESULT CALLBACK ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	CToolBar(HWND hWnd, WORD x, WORD y,char ButSizeX, char ButSizeY, char Delta, LPCTSTR strBack, BOOL ShowCaption);
	~CToolBar();

	BOOL AddButton(LPCTSTR Tips, WORD ID, LPCTSTR strName, char ButType, BOOL Visible=TRUE,BOOL Enable=TRUE, LPCTSTR sMenu=NULL);
	BOOL DeleteButton(WORD ID);
	BOOL ShowButton(WORD ID, BOOL Visible);
	BOOL EnableButton(WORD ID, BOOL Enable);
	BOOL UpdateParam(WORD ID, LPCTSTR Tips);
	BOOL SetMenu(WORD ID, LPCTSTR strName);
	void OnSize(WORD w, WORD h);
};

#endif