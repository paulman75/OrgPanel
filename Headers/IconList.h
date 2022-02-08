#if !defined __ICONLIST__
#define __ICONLIST__

// Version 15.08.2000

typedef struct
{
	HDC		dc;
	BOOL	State;
} 
TListBoxData, *PListBoxData;

class CIconListBox
{
// Constructors
public:
	CIconListBox(HWND, LPRECT,byte,BOOL, byte);
	CIconListBox(HWND, BOOL, byte);

	~CIconListBox();
	// Attributes
	HWND	hWnd;
	void SetCheck(int nIndex, BOOL nCheck);
	BOOL GetCheck(int nIndex);
	LONG OnChildNotify(UINT, WPARAM, LPARAM);

	void BeginUpdate();
	void EndUpdate();
	void AddItem(LPCTSTR, BOOL, HDC);
	void InsertItem(int, LPCTSTR, BOOL, HDC);
	void UpdateItem(int, BOOL, HDC);
	void DeleteItem(int Index);
	void ClearAll();
	virtual LONG FAR PASCAL ObjectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	DWORD	ID;
	void PreDrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void PreMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	int	 CalcItemIndexFromPoint(int Y);

private:
	int		iRedrawCount;
	byte	ItemH;
	BOOL	bCheckBox;
	BOOL	bIcon;
	byte	IconSize;
	WNDPROC	OldListBoxProc;
	void	OnLButtonUp(LPARAM);
	void	OnKeyDown(WPARAM nChar);
	void	InvalidateCheck(int);
	void	PreCreate(byte,BOOL);
	void	PostCreate();
};

#endif