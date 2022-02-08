#ifndef __DRAGLIST_H
#define __DRAGLIST_H

#include "IconList.h"

class CDragListBox : public CIconListBox
{
private:
	BOOL	bMoved;
	BOOL	bDraged;
	HCURSOR	hDragCursor;
	HCURSOR	hNoDragCursor;
	HCURSOR	hOldCursor;
	POINT	hStartPoint;
	int		iDragIndex;
	BOOL	bOldOkDragCursor;
	CDragListBox*	cAttached;
	void	OnDraged(LPARAM, BOOL);
	void	Init();
	LONG FAR PASCAL ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	void AttachDrag(CDragListBox*);
	CDragListBox(HWND, LPRECT,byte,BOOL, byte);
	CDragListBox(HWND, BOOL, byte);
	~CDragListBox();
};

#endif