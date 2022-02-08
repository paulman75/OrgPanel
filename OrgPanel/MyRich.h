#ifndef __MYRICH_H
#define __MYRICH_H

#include "RichEdit.h"

#define MAX_UNDO 30

class CMyRichEdit
{
private:
	HWND		hWnd;
	WNDPROC		hOldProc;
	HMODULE		hRichModule;
	CHARRANGE	crOld;
	int			UndoCount;
	LPTSTR		pUndo[MAX_UNDO];
	DWORD		UndoCur[MAX_UNDO];
	BOOL		bChanging;
	void	SetUndo();
	void	ClearUndo();
	void	MakeUndo();

	void	OnChange();
	void	DetectUrls(CHARRANGE* cr);
	void	PasteFromClipboard();
	void	SetAttrib(int iStart, int iEnd, BOOL bLink, BOOL bAlways=FALSE);
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LONG	OwnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	CMyRichEdit*	next;
	HWND GetWnd() { return hWnd; };
	void SetText(LPCTSTR lpstrText);
	void AddText(LPCTSTR lpstrText);
	LONG RichWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	CMyRichEdit(HWND Owner, WORD x, WORD y, WORD cx, WORD cy, byte MyRichEditID);
	~CMyRichEdit();
};

#endif