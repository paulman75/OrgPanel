#ifndef __SORTLISTVIEW_H
#define __SORTLISTVIEW_H

typedef int UserCompareFunc(LPARAM lParam1, LPARAM lParam2, BOOL bsd, int isc);

#include <commctrl.h>

class CSortListView
{
private:
	//For Header
	HWND	hHeader;
	WNDPROC	hOldHeaderProc;
	static	LONG CALLBACK StaticHeaderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LONG	HeaderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void	DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	UINT	iSortCol;
	BOOL	bSortDir; //TRUE-по возрастанию

	HIMAGELIST	hImageList;
	UserCompareFunc*		FCompFunc;
	HWND		hWnd;
	WORD		wColumns;
	WNDPROC		hOldListProc;
	int			iRedrawCount;
	static int  CALLBACK StaticCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static LONG CALLBACK StaticListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LONG	ListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	void	SetCompareFunc(UserCompareFunc* cf);
	void	SetSortInfo(int isc, BOOL bsd);
	void	GetSortInfo(int* pisc, BOOL* pbsd);
	void	BeginUpdate();
	void	EndUpdate();
	void	ReSort();
	void	ClearColumns();
	void	ClearAllItems();
	HWND	GetWnd() { return hWnd; };
	int		FindDataIndex(LPARAM);
	void	DeleteNote(LPARAM);
	void	AddImage(HBITMAP hBitmap);
	void	AddColumn(LPTSTR lpstrCaption, WORD Width);
	int 	AddText(LPTSTR lpstrText, int iItem, int iSubItem, LPARAM lParam, int iImage);
	CSortListView(HWND Owner, WORD x, WORD y, WORD cx, WORD cy, byte ID,int iImageSize);
	~CSortListView();
};

#endif