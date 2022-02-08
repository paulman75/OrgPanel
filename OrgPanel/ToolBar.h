typedef struct CToolButton
{
	char		ButType;
	WORD		ID;
	WORD		x,y;
	LPTSTR		Tips;
	HBITMAP		hBit;
	BOOL		Visible;
	BOOL		Enable;
	BOOL		NeedDelete;
	char		State; //2- NO, 0- Above, 1 - Press
	CToolButton	*Next;
} CToolButton,*PToolButton;

class CToolBar
{
private:
	PToolButton			FirstButton;
	PToolButton			ActiveButton;
	PToolButton			ClickButton;
	HWND hWnd;
	HWND hToolTipWnd;
	char FButSize;
	BOOL FHorz;
	char FirstPos;
	char KolTips;
	WORD TimerID;
	BOOL Pressed;
	PToolButton DetectButton(int x,int y);
	void DeleteToolTips();
	void MouseMove(PToolButton b,BOOL Always=FALSE);
	void TimerOn();
	void DrawToolBar(PAINTSTRUCT *ps);
	void DrawButton(PToolButton b, HDC hdc);
	void DrawSepLine(HDC hdc,HPEN hPen,PToolButton b,char d);
	void UpdateBar();
	void GetButtonRect(PToolButton b, LPRECT rc);
	void FindButton(WORD ID,PToolButton *ppBut, PToolButton *ppPrev);
	BOOL FAddButton(LPCTSTR Tips, WORD ID, HBITMAP hBit, char ButType, BOOL Visible,BOOL Enable, BOOL NeedDel);
public:
    static LRESULT CALLBACK ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	CToolBar(HWND hWnd, WORD x, WORD y,char ButSize,BOOL Horz, HWND hToolTip);
	~CToolBar();

	BOOL AddButton(LPCTSTR Tips, WORD ID, HBITMAP hBit, char ButType, BOOL Visible=TRUE,BOOL Enable=TRUE);
	BOOL AddButton(LPCTSTR Tips, WORD ID, LPCTSTR BitRes, char ButType, BOOL Visible=TRUE,BOOL Enable=TRUE);
	BOOL DeleteButton(WORD ID);
	BOOL ShowButton(WORD ID, BOOL Visible);
	BOOL EnableButton(WORD ID, BOOL Enable);
	BOOL GetEnableButton(WORD ID);
	BOOL UpdateParam(WORD ID, HBITMAP hBit, LPCTSTR Tips);
	LPCTSTR GetButtonTips(WORD ID);
};
