#include "windows.h"
#include "ExTreeView.h"
#include <commctrl.h>
#include "../Headers/general.h"

extern HINSTANCE hInstance;		  // Global instance handle for application
TTreeNode tn;

TTreeNode CExTreeView::AddChildNode(PTreeNode ParentNode, char* Caption, int CapSize, int Data, int ImageIndex, int SelectedIndex)
{
	TV_INSERTSTRUCT tvi;
	tvi.hParent=NULL;
	if (ParentNode) tvi.hParent=ParentNode->hHandle; 
	tvi.hInsertAfter=TVI_LAST;
	tvi.item.mask=TVIF_TEXT	| TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE;
	tvi.item.pszText=Caption;
	tvi.item.cchTextMax=strlen(Caption);
	tvi.item.iImage=ImageIndex;
	tvi.item.iSelectedImage=SelectedIndex;
	tvi.item.lParam=Data;

	strcpy_s(tn.Caption, CapSize, Caption);
	tn.Data=Data;
	tn.ImageIndex=ImageIndex;
	tn.SelectedIndex=SelectedIndex;
	tn.hHandle=(HTREEITEM)SendMessage(hWnd,TVM_INSERTITEM, 0, (LPARAM)&tvi);
	return tn;
}

void CExTreeView::AddImage(HBITMAP hBitmap)
{
	ImageList_Add(hImageList,hBitmap,NULL);
}

void CExTreeView::BeginUpdate()
{
	if (!iRedrawCount++)
	SendMessage(hWnd,WM_SETREDRAW,0,0);
}

void CExTreeView::EndUpdate()
{
	if (!--iRedrawCount)
	{
		SendMessage(hWnd,WM_SETREDRAW,1,0);
		InvalidateRect(hWnd,NULL,TRUE);
	}
}

LONG CALLBACK CExTreeView::StaticTreeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CExTreeView* tv=(CExTreeView*)GetWindowLong(hwnd,GWL_USERDATA);
	if (tv)	return tv->TreeWndProc(hwnd,msg,wParam,lParam);
	return 0;
}

CExTreeView::CExTreeView(HWND Owner, WORD x, WORD y, WORD w, WORD h, WORD ID, HWND hTree)
{
	iRedrawCount=0;
	bDraggingNow=FALSE;
	hDraggedItem=NULL;
	CanDrag=TRUE;
	bDragImage=TRUE;
	bLastOKCursor = false;
	hMainCursor = NULL;
	hMenu = NULL;
	hOKDragCursor=LoadCursor(hInstance,"DRAGCURSOR");
	hBadDragCursor=LoadCursor(NULL,MAKEINTRESOURCE(IDC_NO));

	hBackBitmap=NULL;
	bCreateWindow=FALSE;
	if (!hTree)
	{
		InitCommonControls();
		hWnd=CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE,"SYSTREEVIEW32","", WS_TABSTOP | WS_CHILD | WS_VISIBLE, x,y,w,h,Owner,NULL,hInstance,NULL);
		SetWindowLong(hWnd, GWL_ID, ID);
		DWORD dw;
		dw=GetWindowLong(hWnd, GWL_STYLE);
		SetWindowLong(hWnd, GWL_STYLE, dw | TVS_SHOWSELALWAYS | TVS_NOTOOLTIPS | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT);
		bCreateWindow=TRUE;
	}
	else hWnd=hTree;
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
	hOldTreeProc=(WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC,(LONG)&StaticTreeWndProc);
	hImageList=ImageList_Create(14,14,ILC_COLOR4,0,0);
	SendMessage(hWnd,TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)hImageList);
	ShowWindow(hWnd,SW_SHOW);
};

CExTreeView::~CExTreeView()
{
	SetWindowLong(hWnd, GWL_USERDATA, 0);
	SetWindowLong(hWnd, GWL_WNDPROC,(LONG)hOldTreeProc);
	if (hBackBitmap) DeleteObject(hBackBitmap);
	ImageList_Destroy(hImageList);
	if (bCreateWindow) DestroyWindow(hWnd);
};

void CExTreeView::SetBackBitmap(LPCTSTR strBit)
{
	if (hBackBitmap) DeleteObject(hBackBitmap);
	hBackBitmap=LoadBitmap(hInstance,strBit);
}

void CExTreeView::OnPaint()
{
	RECT rcPaint;

	rcPaint.left=0;
	rcPaint.top=0;
	int i=GetScrollPos(hWnd, SB_VERT );
	if (i>0)
	{
		HTREEITEM it=TreeView_GetRoot(hWnd);
		if (it)
		{
			TreeView_GetItemRect(hWnd, it, &rcPaint, FALSE);
		}
	}
	rcPaint.left = -GetScrollPos(hWnd, SB_HORZ );
	PaintBack(hWnd, hBackBitmap, hOldTreeProc, rcPaint.left, rcPaint.top);
}

void CExTreeView::Notify(int code, LPNMHDR nmhdr)
{
	NM_TREEVIEW* pnm = (NM_TREEVIEW*)nmhdr;
	switch (code)
	{
	case NM_RCLICK:
		POINT point;
		GetCursorPos(&point);
		TV_HITTESTINFO ht;
		ht.pt=point;
		ScreenToClient(hWnd,&ht.pt);
		ht.flags=TVHT_ONITEM;
		HTREEITEM hi;
		hi=TreeView_HitTest(hWnd,&ht);
		if (hi) TreeView_SelectItem(hWnd,hi);
		TrackPopupMenu(hMenu, 0, point.x, point.y, 0, GetParent(hWnd), NULL);
		break;
	case TVN_ITEMEXPANDING:
	  if (hBackBitmap) InvalidateRect(hWnd, NULL, TRUE);
	  break;
	case TVN_BEGINDRAG:
	  if (!bDraggingNow && CanDrag)
	  {
		if (ItemCanBeDragged(pnm->itemNew.hItem))
		{
			SetCapture(hWnd);
			bLastOKCursor=TRUE;
			hMainCursor=SetCursor(hOKDragCursor);
			bDraggingNow = TRUE;
			hDraggedItem = pnm->itemNew.hItem;
			TreeView_SelectItem(hWnd,hDraggedItem);
			if (bDragImage)
			{
				TV_ITEM tvi;
				tvi.hItem=pnm->itemNew.hItem;
				tvi.mask=TVIF_IMAGE;
				SendMessage(hWnd, TVM_GETITEM, 0,(LPARAM)&tvi);
				ImageList_DragEnter(hWnd,pnm->ptDrag.x,pnm->ptDrag.y);
				ImageList_BeginDrag(hImageList,tvi.iImage,5,5);
			}
		}
	  }
	  break;
	}
}

LONG CExTreeView::TreeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TTreeNode nod1;
	TTreeNode nodTo;
	TVHITTESTINFO hi;

	switch (msg)
	{
	case WM_PAINT:
		OnPaint();
		return TRUE;
	case WM_VSCROLL:
	case WM_HSCROLL:
		if (hBackBitmap) InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_ERASEBKGND:
		if (hBackBitmap) return TRUE;
		break;
	case WM_MOUSEMOVE:
		if (bDragImage)
		{
			ImageList_DragEnter(hWnd,LOWORD(lParam),HIWORD(lParam));
			ImageList_DragMove(LOWORD(lParam),HIWORD(lParam));

			hi.pt.x=LOWORD(lParam);
			hi.pt.y=HIWORD(lParam);

			SendMessage(hWnd, TVM_HITTEST, 0, (LPARAM)&hi);

			if (!hi.hItem)  break;

			if (hi.hItem==hDraggedItem) break;

			if (!GetTreeItem(hDraggedItem,&nod1)) break;
			if (!GetTreeItem(hi.hItem,&nodTo)) break;

			if (IsItemCanBeDroppedOn(&nod1, &nodTo))
			{
				if (!bLastOKCursor)
				{
					bLastOKCursor=TRUE;
					SetCursor(hOKDragCursor);
				}
			}
			else
			if (bLastOKCursor)
			{
				bLastOKCursor=FALSE;
				SetCursor(hBadDragCursor);
			}
		}
		break;
	case WM_LBUTTONUP:
		if (bDraggingNow)
		{
			ReleaseCapture();
			SetCursor(hMainCursor);
			bDraggingNow = FALSE;
			if (bDragImage) ImageList_EndDrag();

			hi.pt.x=LOWORD(lParam);
			hi.pt.y=HIWORD(lParam);

			SendMessage(hWnd, TVM_HITTEST, 0, (LPARAM)&hi);

			if (!hi.hItem)  break;
			if (hi.hItem==hDraggedItem) break;

			if (!GetTreeItem(hDraggedItem,&nod1)) break;
			if (!GetTreeItem(hi.hItem,&nodTo)) break;

			if (!IsItemCanBeDroppedOn(&nod1, &nodTo)) break;

			TTreeQuery tq;
			tq.pNode=&nod1;
			tq.pTarget=&nodTo;
			tq.bResult=FALSE;
			SendMessage(GetParent(hWnd),TVM_CAN_AS_CHILD, 0, (LPARAM)&tq);

			HTREEITEM tiAfter=nodTo.hHandle;
			if (tq.bResult)	// Insert as child
			{
				HTREEITEM tiParent1=TreeView_GetParent(hWnd,nod1.hHandle);
				if (tiParent1==nodTo.hHandle) tiAfter=TVI_FIRST;
				else tiAfter=TVI_LAST;
			}
			else nodTo.hHandle=TreeView_GetParent(hWnd,nodTo.hHandle);
			BeginUpdate();
			HTREEITEM hnew=CopyItem(nod1.hHandle, nodTo.hHandle, tiAfter);
			TreeView_DeleteItem(hWnd,hDraggedItem);
			TreeView_SelectItem(hWnd,hnew);
			EndUpdate();
			SendMessage(GetParent(hWnd),TVM_DROPPED, 0, (LPARAM)&tq);
		}
		break;
	}
	return CallWindowProc(hOldTreeProc, hwnd,msg, wParam, lParam);
}

BOOL CExTreeView::ItemCanBeDragged(HTREEITEM it)
{
	HTREEITEM hRoot=TreeView_GetRoot(hWnd);
	if (it==hRoot) return FALSE;

	TTreeNode tn;
	GetTreeItem(it,&tn);
	TTreeQuery tq;
	tq.bResult=TRUE;
	tq.pNode=&tn;
	tq.pTarget=NULL;
	SendMessage(GetParent(hWnd),TVM_CAN_DRAG, 0, (LPARAM)&tq);
	return tq.bResult;
}

BOOL CExTreeView::IsItemCanBeDroppedOn(PTreeNode pDragged, PTreeNode pDropOn)
{
	HTREEITEM hi=pDropOn->hHandle;
	while (hi)
	{
		hi=TreeView_GetParent(hWnd,hi);
		if (hi==pDragged->hHandle) return FALSE;
	}
	TTreeQuery tq;
	tq.pNode=pDragged;
	tq.pTarget=pDropOn;
	tq.bResult=TRUE;
	SendMessage(GetParent(hWnd),TVM_CAN_DROP, 0, (LPARAM)&tq);
	return TRUE;
}

void CExTreeView::SetDragCursor(LPCTSTR strOKCursor, LPCTSTR strBadCursor)
{
	if (strOKCursor)
	{
		if (hOKDragCursor) DeleteObject(hOKDragCursor);
		hOKDragCursor=LoadCursor(hInstance,strOKCursor);
	}
	if (strBadCursor)
	{
		if (hBadDragCursor) DeleteObject(hBadDragCursor);
		hBadDragCursor=LoadCursor(hInstance,strBadCursor);
	}
}

BOOL CExTreeView::GetTreeItem(HTREEITEM ti, PTreeNode ptn)
{
	TV_ITEM tvi;
	tvi.hItem=ti;
	tvi.mask=TVIF_IMAGE | TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE;
	tvi.cchTextMax=200;
	tvi.pszText=ptn->Caption;
	if (!SendMessage(hWnd, TVM_GETITEM, 0,(LPARAM)&tvi)) return FALSE;
	ptn->Data=tvi.lParam;
	ptn->hHandle=ti;
	ptn->ImageIndex=tvi.iImage;
	ptn->SelectedIndex=tvi.iSelectedImage;
	return TRUE;
}

HTREEITEM CExTreeView::CopyItem(HTREEITEM h1, HTREEITEM hParent, HTREEITEM hAfter)
{
	TTreeNode tn;
	GetTreeItem(h1,&tn);
	TV_INSERTSTRUCT tvi;
	tvi.hParent=hParent;
	tvi.hInsertAfter=hAfter;
	tvi.item.mask=TVIF_TEXT	| TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE;
	tvi.item.pszText=tn.Caption;
	tvi.item.cchTextMax=strlen(tn.Caption);
	tvi.item.iImage=tn.ImageIndex;
	tvi.item.iSelectedImage=tn.SelectedIndex;
	tvi.item.lParam=tn.Data;
	HTREEITEM ti=(HTREEITEM)SendMessage(hWnd,TVM_INSERTITEM, 0, (LPARAM)&tvi);
	
	HTREEITEM hChild=TreeView_GetChild(hWnd, h1);
	while (hChild)
	{
		TTreeNode tn;
		GetTreeItem(hChild,&tn);
		CopyItem(hChild,ti,TVI_LAST);
		hChild=TreeView_GetNextSibling(hWnd,hChild);
	}
	return ti;
}

BOOL CExTreeView::GetParentNode(PTreeNode ptnChild, PTreeNode ptnParent)
{
	HTREEITEM ti=TreeView_GetParent(hWnd,ptnChild->hHandle);
	if (!ti) return FALSE;
	return GetTreeItem(ti,ptnParent);
}

void CExTreeView::Clear()
{
	TreeView_DeleteAllItems(hWnd);
}

BOOL CExTreeView::FindNode(int iImageIndex, int Data, PTreeNode pnode)
{
	HTREEITEM ti=TreeView_GetRoot(hWnd);
	return CheckNode(ti,iImageIndex,Data,pnode);
}

BOOL CExTreeView::CheckNode(HTREEITEM ti, int iImageIndex, int Data, PTreeNode pnode)
{
	GetTreeItem(ti,pnode);
	if (pnode->ImageIndex==iImageIndex && pnode->Data==Data) return TRUE;
	HTREEITEM ti2=TreeView_GetChild(hWnd,ti);
	if (ti2)
	{
		if (CheckNode(ti2,iImageIndex,Data,pnode)) return TRUE;
	}
	ti=TreeView_GetNextSibling(hWnd,ti);
	if (!ti) return FALSE;
	return CheckNode(ti,iImageIndex,Data,pnode);
}

void CExTreeView::SelItem(HTREEITEM ti)
{
	TreeView_SelectItem(hWnd,ti);
}

void CExTreeView::DelItem(HTREEITEM ti)
{
	TreeView_DeleteItem(hWnd,ti);
}

void CExTreeView::SetCaption(HTREEITEM ti, LPTSTR Caption)
{
	TV_ITEM tvi;
	tvi.mask=TVIF_TEXT;
	tvi.pszText=Caption;
	tvi.cchTextMax=strlen(Caption)+1;
	tvi.hItem=ti;
	SendMessage(hWnd,TVM_SETITEM,0,(LPARAM)&tvi);
}

BOOL CExTreeView::GetChildNode(HTREEITEM ti, PTreeNode ptn)
{
	HTREEITEM ti2=TreeView_GetChild(hWnd,ti);
	if (!ti2) return FALSE;
	GetTreeItem(ti2,ptn);
	return TRUE;
}
