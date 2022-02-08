#if !defined __DIALOGS__
#define __DIALOGS__

byte MessageDlg(HWND,LPCTSTR,LPCTSTR,LPCTSTR);
BOOL InputQuery(HWND,LPCTSTR,LPCTSTR,LPTSTR);
void PutOnCenter(HWND hh);

#endif