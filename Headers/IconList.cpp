#include "windows.h"
#include "IconList.h"
#include "const.h"

extern HINSTANCE hInstance;
extern HFONT hMainFont;

PListBoxData	nData;
	
static HBITMAP  CheckBitmap;
static HBRUSH   BackBrush;
static HBRUSH	SelectBrush;	
static HDC		CheckDC;
static byte		IconListCount=0;

LONG FAR PASCAL ListBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIconListBox* obj = (CIconListBox*)GetWindowLong(hwnd, GWL_USERDATA);
    if (obj) return obj->ObjectProc(hwnd, msg, wParam, lParam);
	return 0;
}

void CIconListBox::PreCreate(byte iSize, BOOL  CheckBox)
{
	if (!IconListCount++)
	{
		CheckBitmap=LoadBitmap(GetModuleHandle("OrgPanel.dll"),"CHECKIMAGE");
		BackBrush=CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		SelectBrush=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
		HDC dc=GetDC(0);
		CheckDC=CreateCompatibleDC(dc);
		ReleaseDC(0,dc);
		SelectObject(CheckDC,CheckBitmap);
	}
	ItemH=36;
	IconSize=iSize;
	iRedrawCount=0;

	if (IconSize) ItemH=IconSize+4;
	else ItemH=16;
	bCheckBox=CheckBox;
	bIcon=IconSize;
}

void CIconListBox::PostCreate()
{
	OldListBoxProc=(WNDPROC)SetWindowLong(hWnd,GWL_WNDPROC,(LONG)ListBoxProc);
	SetWindowLong(hWnd, GWL_USERDATA, (long)this);
}

CIconListBox::CIconListBox(HWND Parent, LPRECT rect, byte Id, BOOL bCheckBox, byte byIconSize)
{
	PreCreate(byIconSize, bCheckBox);
	ID=Id<<16;
	hWnd=CreateWindowEx(WS_EX_CLIENTEDGE,"LISTBOX","",LBS_NOTIFY | LBS_HASSTRINGS | WS_CHILD | WS_VISIBLE | WS_VSCROLL| LBS_OWNERDRAWVARIABLE,
				  rect->left,rect->top, rect->right-rect->left,rect->bottom-rect->top,Parent,NULL,hInstance,NULL);
	SetWindowLong(hWnd,GWL_ID,Id);
	PostCreate();
	SendMessage(hWnd,WM_SETFONT,(WPARAM)hMainFont,0);
}

CIconListBox::CIconListBox(HWND hList, BOOL bCheckBox, byte byIconSize)
{
	PreCreate(byIconSize,bCheckBox);
	ID=GetWindowLong(hList,GWL_ID)<<16;
	hWnd=hList;
	PostCreate();
}

void CIconListBox::ClearAll()
{
	int Kol=SendMessage(hWnd,LB_GETCOUNT,0,0);
	while (Kol!=0)
	{
		nData=(PListBoxData)SendMessage(hWnd,LB_GETITEMDATA,--Kol,0);
		free(nData);
	}
	SendMessage(hWnd,LB_RESETCONTENT,0,0);
}

CIconListBox::~CIconListBox()
{
	SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldListBoxProc);
	if (!(--IconListCount)) 
	{
		DeleteObject(CheckBitmap);
		DeleteObject(BackBrush);
		DeleteObject(SelectBrush);
		DeleteDC(CheckDC);
	}
	ClearAll();
}

void CIconListBox::SetCheck(int nIndex, BOOL nCheck)
{
	LRESULT lRes = SendMessage(hWnd,LB_GETITEMDATA, nIndex, 0);
	if (lRes != LB_ERR)
	{
		nData=(PListBoxData)lRes;
		nData->State=nCheck;
		InvalidateCheck(nIndex);
	}
}

BOOL CIconListBox::GetCheck(int nIndex)
{
	LRESULT lRes = SendMessage(hWnd,LB_GETITEMDATA, nIndex, 0);
	if (lRes == LB_ERR) return FALSE;
	nData=(PListBoxData)lRes;
	return nData->State;
}

void CIconListBox::DeleteItem(int Index)
{
	nData=(PListBoxData)SendMessage(hWnd,LB_GETITEMDATA,Index,0);
	free(nData);
	SendMessage(hWnd,LB_DELETESTRING,Index--,0);
	if (Index<0) Index=0;
	SendMessage(hWnd,LB_SETCURSEL,Index,0);
}

void CIconListBox::UpdateItem(int Index, BOOL State, HDC hdc)
{
	nData=(PListBoxData)SendMessage(hWnd,LB_GETITEMDATA,Index,0);
	nData->dc=hdc;
	nData->State=State;
}

void CIconListBox::AddItem(LPCTSTR Text, BOOL State, HDC dc)
{
	InsertItem(-1,Text,State,dc);
}

void CIconListBox::InsertItem(int Index, LPCTSTR Text, BOOL State, HDC dc)
{
	nData=(PListBoxData)malloc(sizeof(TListBoxData));
	nData->dc=dc;
	nData->State=State;
	Index=SendMessage(hWnd,LB_INSERTSTRING,Index ,(LPARAM)Text);
	if (Index==LB_ERR) return;
	SendMessage(hWnd,LB_SETITEMDATA,Index,(LPARAM)nData);
}

void CIconListBox::BeginUpdate()
{
	if (!iRedrawCount++)
	SendMessage(hWnd,WM_SETREDRAW,0,0);
}

void CIconListBox::EndUpdate()
{
	if (!--iRedrawCount)
	{
		SendMessage(hWnd,WM_SETREDRAW,1,0);
		InvalidateRect(hWnd,NULL,TRUE);
	}
}

void CIconListBox::PreDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	DRAWITEMSTRUCT drawItem;
	memcpy(&drawItem, lpDrawItemStruct, sizeof(DRAWITEMSTRUCT));

	if ((((LONG)drawItem.itemID) >= 0) &&
	   ((drawItem.itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) != 0))
	{
		HBRUSH newBk = drawItem.itemState & ODS_SELECTED ? SelectBrush : BackBrush;

		nData=(PListBoxData)drawItem.itemData;

		RECT rectItem = drawItem.rcItem;
		if (bIcon)
		{
			rectItem.right=IconSize+2;
			FillRect(drawItem.hDC,&rectItem,BackBrush);
			BitBlt(drawItem.hDC,drawItem.rcItem.left+1,drawItem.rcItem.top+2,IconSize,IconSize,nData->dc,0,0,SRCCOPY);
			rectItem.right=drawItem.rcItem.right;
			rectItem.left=IconSize+2;
		}
		FillRect(drawItem.hDC,&rectItem,newBk);

		if (bCheckBox)
		{
			BitBlt(drawItem.hDC,rectItem.left+5,rectItem.top+ItemH/2-6,11,11,CheckDC,11*nData->State,0,SRCCOPY);
			rectItem.left+=18;
		}
		if (!bCheckBox && !bIcon) rectItem.left+=4;
		COLORREF newTextColor = GetSysColor(COLOR_WINDOWTEXT);  // light gray
		SetTextColor(drawItem.hDC,newTextColor);

		COLORREF newBkColor = GetSysColor(COLOR_WINDOW);
		SetBkColor(drawItem.hDC,newBkColor);

		if (newTextColor == newBkColor)
			newTextColor = RGB(0xC0, 0xC0, 0xC0);   // dark gray

		if ((drawItem.itemState & ODS_SELECTED) != 0)
		{
			SetTextColor(drawItem.hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
			SetBkColor(drawItem.hDC,GetSysColor(COLOR_HIGHLIGHT));
		}

		char Text[30];
		SendMessage(hWnd,LB_GETTEXT,drawItem.itemID,(LPARAM)&Text);
		TextOut(drawItem.hDC,rectItem.left,
			rectItem.top+ItemH/2-8,(LPCTSTR)&Text,lstrlen((LPCTSTR)&Text));
	}
	if (bIcon) drawItem.rcItem.left+=IconSize+2;
	if ((drawItem.itemAction & ODA_FOCUS) != 0)
		DrawFocusRect(drawItem.hDC,&(drawItem.rcItem));
}

void CIconListBox::InvalidateCheck(int Index)
{
	nData=(PListBoxData)SendMessage(hWnd,LB_GETITEMDATA,Index,0);
	HDC dc=GetDC(hWnd);
	RECT rc;
	SendMessage(hWnd,LB_GETITEMRECT,Index,(LPARAM)&rc);
	if (bIcon) rc.left+=IconSize+2;
	BitBlt(dc,rc.left+5,rc.top+ItemH/2-6,11,11,CheckDC,11*nData->State,0,SRCCOPY);
	ReleaseDC(hWnd,dc);
}

void CIconListBox::PreMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight=ItemH;
}

LONG CIconListBox::OnChildNotify(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_DRAWITEM:
		PreDrawItem((LPDRAWITEMSTRUCT)lParam);
		break;
	case WM_MEASUREITEM:
		PreMeasureItem((LPMEASUREITEMSTRUCT)lParam);
		break;
	}
	return 1;
}

int CIconListBox::CalcItemIndexFromPoint(int Y)
{
	int res=SendMessage(hWnd,LB_GETTOPINDEX,0,0);
	if (Y<-1) return -1;
	res+=Y/ItemH;
	int Kol=SendMessage(hWnd,LB_GETCOUNT,0,0);
	if (res>=Kol) res=-1;
	return res;
}

void CIconListBox::OnLButtonUp(LPARAM lParam)
{
	if (!bCheckBox) return;
	int x=LOWORD(lParam);
	if (!bIcon && x<18 || bIcon && x>IconSize && x<IconSize+18)
	{
		int Index=CalcItemIndexFromPoint(HIWORD(lParam));
		if (Index==-1) return;
		nData=(PListBoxData)SendMessage(hWnd,LB_GETITEMDATA,Index,0);
		if (!SendMessage(GetParent(hWnd),WM_LISTBOXCHECKCHANGE,ID+Index,!nData->State))
			nData->State=!nData->State;
		InvalidateCheck(Index);
	}
}

void CIconListBox::OnKeyDown(WPARAM nChar)
{
	if (nChar == VK_SPACE)
	{
		int nIndex = SendMessage(hWnd,LB_GETCARETINDEX,0,0);;

		if (nIndex != LB_ERR)
		{
		nData=(PListBoxData)SendMessage(hWnd,LB_GETITEMDATA,nIndex,0);
		if (!SendMessage(GetParent(hWnd),WM_LISTBOXCHECKCHANGE,ID+nIndex,!nData->State))
			nData->State=!nData->State;
		InvalidateCheck(nIndex);
		}
	}
	if (nChar==VK_TAB) PostMessage(GetParent(hWnd),WM_LISTBOXTABDOWN,(WPARAM)ID,0);
}

LONG FAR PASCAL CIconListBox::ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONUP:
		OnLButtonUp(lParam);
		break;
	case WM_KEYDOWN:
		OnKeyDown(wParam);
		break;
	}
	return CallWindowProc(OldListBoxProc, hwnd, msg, wParam, lParam);
}
