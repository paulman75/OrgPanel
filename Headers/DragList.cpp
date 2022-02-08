#include "windows.h"
#include "DragList.h"
#include "const.h"

extern HINSTANCE hInstance;

void CDragListBox::Init()
{
	bMoved=FALSE;
	bDraged=FALSE;
	hDragCursor=LoadCursor(GetModuleHandle("OrgPanel.dll"),"DRAG");
	hNoDragCursor=LoadCursor(GetModuleHandle("OrgPanel.dll"),"NODRAG");
	cAttached=NULL;
}

CDragListBox::CDragListBox(HWND Parent, LPRECT rect, byte Id, BOOL CheckBox, byte byIconSize) : CIconListBox(Parent, rect, Id, CheckBox, byIconSize)
{
	Init();
}

CDragListBox::CDragListBox(HWND Parent, BOOL CheckBox, byte byIconSize) : CIconListBox(Parent, CheckBox, byIconSize)
{
	Init();
}

void CDragListBox::OnDraged(LPARAM lParam, BOOL bEndDrag)
{
	RECT rc;
	GetClientRect(hWnd,&rc);
	int x=LOWORD(lParam);
	if (x>30000) x-=65536;
	int y=HIWORD(lParam);
	if (y>30000) y-=65536;
	int Index=CalcItemIndexFromPoint(y);
	BOOL b=x>0 && x<rc.right && Index!=-1;
	BOOL Att=FALSE;

	if (!b && cAttached)
	{
		RECT rect;
		GetWindowRect(cAttached->hWnd,&rect);
		RECT rect1;
		GetWindowRect(hWnd,&rect1);
		rect.bottom-=rect1.bottom-rc.bottom;
		rect.left-=rect1.left-rc.left;
		rect.right-=rect1.right-rc.right;
		rect.top-=rect1.top-rc.top;
		Index=cAttached->CalcItemIndexFromPoint(y-rect.top);
		b=x>rect.left && x<rect.right && Index!=-1;
		Att=TRUE;
	}

	if (!bEndDrag && b!=bOldOkDragCursor)
	{
		bOldOkDragCursor=b;
		SetCursor(b ? hDragCursor : hNoDragCursor);
	}
	if (b && bEndDrag)
	{
		DWORD dwId=Att ? cAttached->ID : ID;
		SendMessage(GetParent(hWnd),WM_LISTBOXENDDRAG,ID | iDragIndex,dwId | Index);
	}
}

LONG FAR PASCAL CDragListBox::ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONUP:
		SendMessage(GetParent(hWnd),WM_COMMAND,LBN_SELCHANGE <<16 | ID >>16, (LPARAM)hWnd);
		if (bDraged)
		{
			OnDraged(lParam,TRUE);
			bDraged=FALSE;
			SetCursor(hOldCursor);
		}
		if (bMoved)
		{
			bMoved=FALSE;
			ReleaseCapture();
		}
		break;
	case WM_LBUTTONDOWN:
		hStartPoint.x=LOWORD(lParam);
		hStartPoint.y=HIWORD(lParam);

		iDragIndex=CalcItemIndexFromPoint(hStartPoint.y);
		if (iDragIndex==-1) break;
		bMoved=TRUE;
		SetCapture(hwnd);
		break;
	case WM_MOUSEMOVE:
		if (bDraged)
		{
			OnDraged(lParam,FALSE);
			return 0;
		}
		if (bMoved && !bDraged)
		{
			int x=hStartPoint.x-LOWORD(lParam);
			int y=hStartPoint.y-HIWORD(lParam);
			if ((x*x+y*y)>100)
			{
				bDraged=TRUE;
				bOldOkDragCursor=TRUE;
				hOldCursor=SetCursor(hDragCursor);
			}
			return 0;
		}
		break;
	}
	return CIconListBox::ObjectProc(hwnd, msg,wParam, lParam);
}

void CDragListBox::AttachDrag(CDragListBox* cBox)
{
	cAttached=cBox;
}

CDragListBox::~CDragListBox()
{
	DeleteObject(hDragCursor);
	DeleteObject(hNoDragCursor);
}
