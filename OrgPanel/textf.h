#ifndef __TEXTF_H
#define __TEXTF_H

#include <windows.h>

BOOL TextEffect(HDC, LPPOINT, LPPOINT, DWORD, DWORD, LPSTR, COLORREF OCol, COLORREF Color);
BOOL FillOut(LPPOINT *lpPoints, LPDWORD lpdwNumPts);

#endif