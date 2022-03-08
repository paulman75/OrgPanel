#if !defined __BARUNIT_H
#define __BARUNIT_H

LONG FAR PASCAL BarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ShowOptions(BOOL);
void UpdateIcon();
void UpdateIconSize();
void ExitFromProgram();
void SetHourTimer();
void	BarUsePassword();
void SetSwitchValue(int Ind);
void RepaintBar();
void SetAlpha(byte pAlpha);
void UpdateAlwaysOnTop();

#endif