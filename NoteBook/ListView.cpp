#include "windows.h"
#include "ListView.h"
#include "res/resource.h"
#include "../Headers/const.h"
#include "../Headers/general.h"

extern HINSTANCE hInstance;
static int ListViewCounts=0;
static HMENU hHeadMenu=NULL;
static HCURSOR	hArrowCursor=NULL;

int CSortListView::AddText(LPTSTR lpstrText, int iItem, int iSubItem, LPARAM lParam, int iImage)
{
	LV_ITEM lvi;
	lvi.pszText=lpstrText;
	lvi.lParam=lParam;
	lvi.iItem=iItem;
	lvi.iSubItem=iSubItem;
	int res=iItem;
	if (iSubItem)
	{
		lvi.mask=LVIF_TEXT;
		SendMessage(hWnd,LVM_SETITEM,0,(LPARAM)&lvi);
	}
	else
	{
		lvi.mask=LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
		lvi.iImage=iImage;
		res=SendMessage(hWnd,LVM_INSERTITEM,0,(LPARAM)&lvi);
	}
	return res;
}

void CSortListView::ReSort()
{
	SendMessage(hWnd,LVM_SORTITEMS,(WPARAM)this,(LPARAM)StaticCompareFunc);
}

void CSortListView::AddColumn(LPTSTR lpstrCaption, WORD Width)
{
	LV_COLUMN lvc;
	lvc.mask=LVCF_WIDTH | LVCF_TEXT;
	lvc.pszText=lpstrCaption;
	lvc.cx=Width;
	SendMessage(hWnd,LVM_INSERTCOLUMN,wColumns++,(LPARAM)&lvc);
	HD_ITEM hd;
	ZeroMemory(&hd,sizeof(hd));
	hd.mask=HDI_FORMAT;
	SendMessage(hHeader,HDM_GETITEM,wColumns-1,(LPARAM)&hd);
	hd.fmt |= HDF_OWNERDRAW;
	SendMessage(hHeader,HDM_SETITEM,wColumns-1,(LPARAM)&hd);
}

void CSortListView::ClearColumns()
{
	SendMessage(hWnd,LVM_DELETEALLITEMS,0,0);
	while (wColumns)
	SendMessage(hWnd,LVM_DELETECOLUMN,(wColumns--)-1,0);
}

int	CSortListView::FindDataIndex(LPARAM lData)
{
	int c=SendMessage(hWnd,LVM_GETITEMCOUNT,0,0);
	int i=0;
	while (i<c)
	{
		LV_ITEM lv;
		lv.mask=LVIF_PARAM;
		lv.iItem=i;
		lv.iSubItem=0;
		SendMessage(hWnd,LVM_GETITEM,0,(LPARAM)&lv);
		if (lv.lParam==lData) return i;
		i++;
	}
	return -1;
}

void CSortListView::DeleteNote(LPARAM lParam)
{
	int i=FindDataIndex(lParam);
	if (i==-1) return;
	SendMessage(hWnd,LVM_DELETEITEM,i,0);
}

void CSortListView::AddImage(HBITMAP hBitmap)
{
	ImageList_Add(hImageList,hBitmap,NULL);
}

void CSortListView::SetCompareFunc(UserCompareFunc* cf)
{
	FCompFunc=cf;
}

void CSortListView::SetSortInfo(int isc, BOOL bsd)
{
	iSortCol=isc;
	bSortDir=bsd;
}

void CSortListView::GetSortInfo(int* pisc, BOOL* pbsd)
{
	*pisc=iSortCol;
	*pbsd=bSortDir;
}

void CSortListView::BeginUpdate()
{
	if (!iRedrawCount++)
	SendMessage(hWnd,WM_SETREDRAW,0,0);
}

void CSortListView::EndUpdate()
{
	if (!--iRedrawCount)
	{
		SendMessage(hWnd,WM_SETREDRAW,1,0);
		InvalidateRect(hWnd,NULL,TRUE);
	}
}

int CALLBACK CSortListView::StaticCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSortListView* li=(CSortListView*)lParamSort;
	if (li->FCompFunc) return li->FCompFunc(lParam1,lParam2,li->bSortDir,li->iSortCol);
	return 0;
}

void CSortListView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	HDC dc=lpDrawItemStruct->hDC;

	RECT rcLabel=lpDrawItemStruct->rcItem;
	
	// Draw the background
    FillRect(dc, &rcLabel, (HBRUSH)COLOR_WINDOW);

	SIZE siz;
	GetTextExtentPoint32(dc," ",1,&siz);
	int offset=siz.cx*2;

	// Get the column text and format
	char buf[256];
	HD_ITEM hditem;	

	hditem.mask = HDI_TEXT | HDI_FORMAT;
	hditem.pszText = buf;
	hditem.cchTextMax = 255;
	SendMessage(hHeader,HDM_GETITEM,lpDrawItemStruct->itemID, (LPARAM)&hditem);

	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS ;

	if (hditem.fmt & HDF_CENTER)
		uFormat |= DT_CENTER;
	else if (hditem.fmt & HDF_RIGHT)
		uFormat |= DT_RIGHT;
	else
		uFormat |= DT_LEFT;

	// Adjust the rect if the mouse button is pressed on it
	if (lpDrawItemStruct->itemState == ODS_SELECTED)
	{
		rcLabel.left++;
		rcLabel.top += 2;
		rcLabel.right++;
	}
	// Adjust the rect further if Sort arrow is to be displayed
	if (lpDrawItemStruct->itemID==iSortCol)
	{
		rcLabel.right -= 3 * offset;
	}
	rcLabel.left += offset;
	rcLabel.right -= offset;
	DrawText(dc,buf, -1, &rcLabel, uFormat);

	// Draw the Sort arrow
	if (lpDrawItemStruct->itemID==iSortCol)
	{
		RECT rcIcon=lpDrawItemStruct->rcItem;

		// Set up pens to use for drawing the triangle
		HPEN penLight=CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
		HPEN penShadow=CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		HPEN pOldPen=(HPEN)SelectObject(dc,penLight);

		if (bSortDir)
		{
			// Draw triangle pointing upwards
			MoveToEx(dc,rcIcon.right - 2*offset, offset - 1,NULL);
			LineTo(dc,rcIcon.right - 3*offset/2, rcIcon.bottom - offset);
			LineTo(dc,rcIcon.right - 5*offset/2-2, rcIcon.bottom - offset);
			MoveToEx(dc,rcIcon.right - 5*offset/2-1, rcIcon.bottom - offset - 1,NULL);

			SelectObject(dc,penShadow);
			LineTo(dc,rcIcon.right - 2*offset, offset-2);
		}
		else
		{
			// Draw triangle pointing downwards
			MoveToEx(dc,rcIcon.right - 3*offset/2, offset-1,NULL);
			LineTo(dc,rcIcon.right - 2*offset-1, rcIcon.bottom - offset + 1);
			MoveToEx(dc,rcIcon.right - 2*offset-1, rcIcon.bottom - offset,NULL);

			SelectObject(dc,penShadow);
			LineTo(dc,rcIcon.right - 5*offset/2-1, offset - 1);
			LineTo(dc,rcIcon.right - 3*offset/2, offset - 1);
		}
		SelectObject(dc,pOldPen);
		DeleteObject(penLight);
		DeleteObject(penShadow);
	}
}

void CSortListView::ClearAllItems()
{
	SendMessage(hWnd,LVM_DELETEALLITEMS,0,0);
}

CSortListView::CSortListView(HWND Owner, WORD x, WORD y, WORD cx, WORD cy, byte ID, int iImageSize)
{
	if (!ListViewCounts++)
	{
		hHeadMenu=LoadMenu(hInstance,MAKEINTRESOURCE(IDR_HEADMENU));
		hArrowCursor=LoadCursor(NULL,IDC_ARROW);	
	}
	InitCommonControls();
	wColumns=0;
	iRedrawCount=0;
	iSortCol=0;
	bSortDir=FALSE;
	FCompFunc=NULL;
	hWnd=CreateWindowEx(WS_EX_CLIENTEDGE,"SYSLISTVIEW32","", LVS_REPORT | LVS_SINGLESEL /*| LVS_OWNERDRAWFIXED */| WS_TABSTOP | WS_CHILD | WS_VISIBLE | LVS_SHOWSELALWAYS,x,y,cx,cy,Owner,NULL,hInstance,NULL);

	SendMessage(hWnd,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
	SetWindowLong(hWnd, GWL_ID, ID);
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
	hHeader=GetWindow(hWnd,GW_CHILD);
	if (!hHeader) return;
	hOldHeaderProc=(WNDPROC)SetWindowLong(hHeader, GWL_WNDPROC,(LONG)&StaticHeaderWndProc);
	hOldListProc=(WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC,(LONG)&StaticListWndProc);
	hImageList=ImageList_Create(iImageSize,iImageSize,ILC_COLOR4,0,0);
	SendMessage(hWnd,LVM_SETIMAGELIST,LVSIL_SMALL,(LPARAM)hImageList);
};

CSortListView::~CSortListView()
{
	DestroyWindow(hWnd);
	if (!--ListViewCounts)
	{
		DestroyMenu(hHeadMenu);
		DestroyCursor(hArrowCursor);
		ImageList_Destroy(hImageList);
	}
}

LONG CALLBACK CSortListView::StaticHeaderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSortListView* lv=(CSortListView*)GetWindowLong(GetParent(hwnd),GWL_USERDATA);
	return lv->HeaderWndProc(hwnd,msg,wParam,lParam);
}

LONG CSortListView::HeaderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int iColIndex;
	static BOOL bMenuShowing=FALSE;

	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_HEADOPTION:
			PostMessage(GetParent(hWnd),WM_COLUMNOPTION,0,0);
			break;
		case IDC_HEADHIDE:
			PostMessage(GetParent(hWnd),WM_COLUMNHIDE,0,iColIndex);
			break;
		case IDC_HEADUP:
		case IDC_HEADDOWN:
			iSortCol=iColIndex;
			bSortDir=LOWORD(wParam)==IDC_HEADUP;
			ReSort();
			break;
		}
		break;
	case WM_SETCURSOR:
		if (bMenuShowing)
		{
			SetCursor(hArrowCursor);
			return TRUE;
		}
		break;
	case WM_EXITMENULOOP:
		bMenuShowing=FALSE;
		break;
	case WM_RBUTTONDOWN:
		SendMessage(hHeader,WM_CANCELMODE,0,0);
		break;
	case WM_RBUTTONUP:
		POINT point;
		bMenuShowing=TRUE;
		GetCursorPos(&point);
		HD_HITTESTINFO hdinfo;
		hdinfo.pt=point;
		ScreenToClient(hHeader,&hdinfo.pt);
		SendMessage(hHeader,HDM_HITTEST,0,(LPARAM)&hdinfo);
		iColIndex=hdinfo.iItem;
		if (iColIndex!=-1)
		{
			EnableMenuItem(hHeadMenu,IDC_HEADHIDE, iColIndex!=0 ? MF_ENABLED:MF_GRAYED);
			TrackPopupMenu(GetSubMenu(hHeadMenu,0), 0, point.x, point.y, 0, hHeader, NULL);
		}
		break;
	}
	return CallWindowProc(hOldHeaderProc, hwnd,msg, wParam, lParam);
}

LONG CALLBACK CSortListView::StaticListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSortListView* lv=(CSortListView*)GetWindowLong(hwnd,GWL_USERDATA);
	return lv->ListWndProc(hwnd,msg,wParam,lParam);
}

LONG CSortListView::ListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DRAWITEM:
		DrawItem((LPDRAWITEMSTRUCT)lParam);
		return TRUE;
		break;
	case WM_NOTIFY:
		NMHEADER* nmh;
		nmh=(NMHEADER*)lParam;
		switch (nmh->hdr.code)
		{
		case HDN_BEGINDRAG:
			if (!nmh->iItem)
			{
				PostMessage(hHeader,WM_LBUTTONUP,0,0);
				return TRUE;
			}
			return FALSE;
		case HDN_ENDDRAG:
			HD_ITEM* hd;
			hd=nmh->pitem;
			if (!hd->iOrder) return TRUE;
			SendMessage(GetParent(hWnd),WM_COLUMNORDERCHANGE,hd->iOrder,nmh->iItem);
			break;
		case HDN_ENDTRACKW:
		case HDN_ENDTRACK:
			HD_NOTIFY* hdn;
			hdn=(HD_NOTIFY*)lParam;
			SendMessage(GetParent(hWnd),WM_COLUMNWIDTHCHANGE,hdn->pitem->cxy,hdn->iItem);
			break;
		case HDN_ITEMCLICKW:
		case HDN_ITEMCLICK:
			NM_LISTVIEW* nml=(NM_LISTVIEW*)nmh;
			if (nml->iItem==(int)iSortCol) bSortDir=!bSortDir;
		    else iSortCol=nml->iItem;
			SendMessage(hWnd,LVM_SORTITEMS,(WPARAM)this,(LPARAM)StaticCompareFunc);
			break;
		}
	}
	return CallWindowProc(hOldListProc, hwnd,msg, wParam, lParam);
}
