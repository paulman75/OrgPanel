#ifndef _TOOLBAR2_H
#define _TOOLBAR2_H

typedef struct CToolButton
{
	WORD		ID;
	LPVOID		lpvData;
	WORD		x,y;
	LPTSTR		Tips;
	CMyBitmap*	hBit;
	BOOL		Pressed;
	CToolButton	*Next;
	BOOL		Splitter;
} CToolButton,*PToolButton;

class CToolBar2
{
private:
	PToolButton			FirstButton;
	PToolButton			ClickButton;
	PToolButton			AboveButton;
	HWND hWnd;
	HWND hToolTipWnd;
	char FButSize;
	byte FEdge;
	char KolTips;
	WORD TimerID;
	BOOL Pressed;
	HCURSOR	hDragCursor;
	HCURSOR	hOldCursor;
	BOOL CanUpdate;
	byte FToolBarID;
	BOOL bDraged;
	BOOL bMoved;
	WORD StartX;
	WORD StartY;
	PToolButton DetectButton(int x,int y);
	void DeleteToolTips();
	void MouseMove(PToolButton b, BOOL Always=FALSE);
	void TimerOn();
	void DrawToolBar(PAINTSTRUCT *ps);
	void DrawButton(PToolButton b, HDC hdc);
	void UpdateBar();
	void SetAboveButton(PToolButton b);
	void GetButtonRect(PToolButton b, LPRECT rc);
	void FindButton(WORD ID,PToolButton *ppBut, PToolButton *ppPrev);
	BOOL IsHorizontal();
	void GetButtonSize(PToolButton pBut, WORD *px, WORD *py);
public:
    static LRESULT CALLBACK ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	CToolBar2(HWND Owner, WORD x, WORD y,char ButSize, byte iEdge, HWND hToolTip,byte ToolBarID);
	~CToolBar2();

	BOOL bCanDrag;
	inline HWND GetHWND() {return hWnd;};
	void BeginUpdate();
	void EndUpdate();
	void FreeAll();
//	void GetButtonCenter(LPVOID Data, LPPOINT);
	void SetButSize(byte NewSize);
	void OnLButDown(WORD x, WORD y);
	void OnMouseMove(WORD x, WORD y);
	void OnLButUp(WORD x, WORD y);
	void SetNewPosition(WORD x, WORD y);
	LPVOID GetPrevUnit(WORD x, WORD y);
	void SetEdge(byte NewValue);
	BOOL AddButton(LPCTSTR Tips, CMyBitmap* hBit,LPVOID Data, BOOL bSplitter);
};

#endif
