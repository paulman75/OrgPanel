class CFolderSwitch
{
protected:
 private:
	 HMENU  hMenu;
	 HWND	LBut,RBut;
	 HWND	hToolTip;
	 WORD FValue;
	 WORD FMax;
	 void UpdateValue();
	 void DeleteToolTip();
	 BOOL LPressed,RPressed;
	 byte TimerId;
	 byte IncPause;
	 void IncValue(int Offset);
	 void InitMenu();
	 void DrawButton(HWND, HDC hdc, BOOL Left);
	 void DrawNumber(HDC hdc);
	 void DrawLine(BOOL, HDC dc,HPEN NoPen,HPEN YesPen,int x0,int y0,int x1,int y1,int x2,int y2);
 public:
	 HWND  hWnd;
     LONG FAR PASCAL WindowProc(HWND, UINT msg, WPARAM wParam, LPARAM lParam);
	 void UpdatePosition(WORD,WORD,byte,byte,byte,BOOL);
	 void SetValue(byte,byte);
	 void UpdateToolTip();
	 CFolderSwitch(HWND);
	//Owner, Left, Top, Width, Min,Max, Large, IncValue,Color
	~CFolderSwitch();
};
