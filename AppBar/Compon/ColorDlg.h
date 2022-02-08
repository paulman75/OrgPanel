#ifndef _COLORDLG_H
#define _COLORDLG_H

class CColorDlg
{
private:
	void Execute();
	COLORREF	FColor;
	static LRESULT CALLBACK ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	HWND		hWnd;
	inline int GetColor() {return FColor;};
	void	SetColor(COLORREF col);
	CColorDlg(HWND, int, int, COLORREF);
	~CColorDlg();
};

#endif