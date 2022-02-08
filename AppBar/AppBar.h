
#if !defined(__AppBar_H)
#define __AppBar_H

#define ABE_DESKTOP		5

typedef struct {    // rc 

    LONG left;
    LONG top;
    LONG width;
    LONG height;
} SIZERECT,*pSIZERECT; 

#define WM_APPBAR_EVENT     WM_USER + 1010

const int TitSize=20;

class CAppBar  
{
private:
	void AppBarCallBack (UINT uMsg, WPARAM wParam, LPARAM lParam);
	void MoveMainWindow(BOOL XChange,int Pos);
    BOOL	CheckOtherPanel();
    BOOL	CheckEndPanelTrack(int XDelta,int YDelta);
    void	MesActivate(WORD wParam);
    void	CheckMouseOver();
    void	Opening();
    void	Closing();
    void	StartMoveWindow(WORD, WORD);
    byte	DetectEdge(pSIZERECT);
    void	WindowToBottom();
    void	WindowPosChang();
    void	AppBarQuerySetPos();
    void	MouseMove(WPARAM wParam=MK_LBUTTON);
    void	UnRegisterBar();
	void	UpdateOnDesktop();
	void	OnDelayTimer();
	HWND	Main_hwnd;
    BOOL	FAutoHide;
    int		OpenPanelX,ClosePanelX;
    int		OpenPanelY,ClosePanelY;
    byte	FEdge;
    BOOL	Registered;
    byte	FDelta;
    BOOL	FMouseDown;
    byte	FOpenSpeed,FCloseSpeed;
	RECT	rc;
    BOOL	ReMoved;
    BOOL	PanelOpenning;
	BOOL	CanOpen;
  	int		OpenDelay;
	int		Direct;
    APPBARDATA	Dat;
    POINT	MovePos,StartPos;
	HRGN	rgn;
    BOOL	SetedHide;
    BOOL	MouseOver;
    WORD	FWidthOnDesktop,FHeightOnDesktop;
    WORD	FullBarWidth,FullBarHeight;
	void	Timer1Event();
    void	EndMoveWindow();
	void	ChangeBarRgn(BOOL bOpening);

public:
	POINT	ptbar[10];
	BOOL	bFullScreen;

	void	SetWidthOnDesktop(WORD);
	void	SetHeightOnDesktop(WORD);
	inline WORD	GetWidthOnDesktop() {return FWidthOnDesktop;};
	byte	MoveEdge;
	void	SetEdge(byte);
    WORD	BarWidth,BarHeight;
	BOOL	bNeedTitle;
	int		XTitle;
	int		YTitle;
	void	AppBarWinProc(HWND  hwnd,  UINT  uMsg,  WPARAM  wParam, LPARAM  lParam);
	HWND	GetMainWindow();
    BOOL	Moveable;
	BOOL	GetTitleVisible();
    void	RegisterBar();
    void	UpdateBar();
	void	SetSpeed(byte os,byte cs);
	void	SetAutoHide(BOOL);
	BOOL	GetAutoHide();
	void	SetOpenDelay(int od);
	CAppBar(HWND hWnd);
	virtual ~CAppBar();
};
    
#endif