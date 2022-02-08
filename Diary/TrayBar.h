#ifndef _INCLUDED_SYSTEMTRAY_H_
#define _INCLUDED_SYSTEMTRAY_H_

#define WM_TRAYBARCALLBACK WM_USER+22

#define ID_MAIN 6

void SetIconMode(int NewMode);
void SetTrayBar();
void RemoveIcon();

#endif