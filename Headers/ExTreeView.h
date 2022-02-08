#ifndef __EXTREEVIEW_H
#define __EXTREEVIEW_H

#include <commctrl.h>

#define TVM_CAN_AS_CHILD WM_USER+501
#define TVM_CAN_DRAG WM_USER+502
#define TVM_CAN_DROP WM_USER+503
#define TVM_DROPPED  WM_USER+504

typedef struct
{
	char		Caption[200];
	int			Data;
	HTREEITEM	hHandle;
	int			ImageIndex;
	int			SelectedIndex;
} TTreeNode, *PTreeNode;

typedef struct
{
	PTreeNode	pNode;
	PTreeNode	pTarget;
	BOOL		bResult;
} TTreeQuery, *PTreeQuery;

class CExTreeView
{
private:
	WNDPROC		hOldTreeProc;
	HWND		hWnd;
	HBITMAP		hBackBitmap;
	int			iRedrawCount;
	void		OnPaint();
	HIMAGELIST	hImageList;
	BOOL		bDraggingNow;
	HTREEITEM	hDraggedItem;
	HCURSOR		hOKDragCursor;
	HCURSOR		hBadDragCursor;
	HCURSOR		hMainCursor;
	BOOL		bLastOKCursor;
	BOOL		bCreateWindow;

	BOOL ItemCanBeDragged(HTREEITEM it);
	BOOL IsItemCanBeDroppedOn(PTreeNode pDragged, PTreeNode pDropOn);
	HTREEITEM CopyItem(HTREEITEM h1, HTREEITEM hParent, HTREEITEM hAfter);
	BOOL	CheckNode(HTREEITEM ti, int iImageIndex, int Data, PTreeNode pnode);

public:
	BOOL	CanDrag;
	BOOL	bDragImage;
	HMENU	hMenu;

	void	BeginUpdate();
	void	EndUpdate();

	void Clear();
	HWND GetHWND() { return hWnd; };
	BOOL GetParentNode(PTreeNode ptnChild, PTreeNode ptnParent);
	BOOL GetTreeItem(HTREEITEM ti, PTreeNode ptn);
	BOOL GetChildNode(HTREEITEM ti, PTreeNode ptn);
	void SetDragCursor(LPCTSTR strOKCursor, LPCTSTR strBadCursor);
	void AddImage(HBITMAP hBitmap);
	void SelItem(HTREEITEM ti);
	HTREEITEM GetSelItem() { return TreeView_GetNextItem(hWnd, NULL, TVGN_CARET); };
	HTREEITEM GetRootItem() { return TreeView_GetRoot(hWnd); };
	HTREEITEM GetNextItem(HTREEITEM hi) { return TreeView_GetNextItem(hWnd, hi, TVGN_NEXT); };
	HTREEITEM GetChildItem(HTREEITEM hi) { return TreeView_GetNextItem(hWnd, hi, TVGN_CHILD); };
	void DelItem(HTREEITEM ti);
	void SetCaption(HTREEITEM ti, LPTSTR Caption);
	void Notify(int code, LPNMHDR nmhdr);
	void SetBackBitmap(LPCTSTR strBit);
	TTreeNode AddChildNode(PTreeNode ParentNode, char* Caption, int Data, int ImageIndex, int SelectedIndex);
	static LONG CALLBACK StaticTreeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LONG	TreeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL	FindNode(int iImageIndex, int Data, PTreeNode pnode);
	CExTreeView(HWND hWnd, WORD x, WORD y, WORD w, WORD h, WORD ID, HWND hTree);
	~CExTreeView();
};

#endif