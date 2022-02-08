#ifndef __FLTEXT_H
#define __FLTEXT_H

#define WM_FLOATTEXTEVENT WM_USER+163
//(wparam = 0-текст ушел в небеса, 1- Spin закончился)

typedef struct String
{
	LPTSTR		St;
	WORD		YDelta;
	WORD		FontH;
	COLORREF	Color;
	COLORREF	OColor;
	String		*Next;
} String, *LPString;

class CFloatText
{
private:
	int		FirstY;
	HANDLE	hHandle;
	BOOL	bSpin;
	BOOL	Work;

	HBITMAP hFlTextBack;
	HWND	hwnd;
	LPString	St;
	WORD Ftopw;
	WORD FSmallStep;
	WORD FTimerInterval;
	HFONT hFont;
	COLORREF	BackCol;
	void Free();
	void FreeSt(LPString* lpst);
	void ClearPoints();
	void Paint(HDC dc);
	DWORD dwTop,dwBot;
	LPPOINT lpTop,lpBot;

	HBITMAP hBitmap;
	int xBit;
	int yBit;
	BOOL BitVisible;

	HDC		hMemDC;
	HBITMAP	hMemBit;
	void RePaint();

	int xs;
	int ys;
	float sw1,sh1;
	WORD sw2,sh2;
	LPString SpinSt;
	float dl;
	float dh;
	float dw;
	float Angle;
	WORD wFrames;
	BOOL Start();
	BOOL Stop();
	BOOL NeedStop;
public:
	BOOL ThreadFunc();
	void AddString(WORD YDelta, WORD FontHeight, COLORREF Col, COLORREF oCol, LPCTSTR st);
	void SetParam(WORD topw, WORD SmallStep);
	CFloatText(HWND Owner, int x, int y, WORD w, WORD h, LPCTSTR BackBitmap, COLORREF BackColor);
	~CFloatText();
	BOOL Go(WORD TimerInterval);
	void LoadBitmap2(LPCTSTR Name, int x, int y);
	void ShowBitmap(BOOL Show);
	BOOL MakeSpin(int x, int y, WORD w1, WORD w2, WORD h1, WORD h2, COLORREF Col, COLORREF oCol, LPCTSTR st, byte spins, WORD Frames, WORD TimerInterval);
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif