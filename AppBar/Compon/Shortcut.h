#ifndef _INC_SHORTCUT
#define _INC_SHORTCUT

#include <shlobj.h>
BOOL GetIconFromLnk(BarUnit*);
BOOL GetIconExt(LPCTSTR, BarUnit*);
void ReadUrl(BarUnit* uni);

typedef struct {
    CMyString strPath;
    CMyString strTarget;
    CMyString strStartDir;
    CMyString strDescription;
    CMyString strIconLocation;
    CMyString strArgs;
    int     nIconIndex;
    WORD    wHotkey;
    int     nShowCmd;
	CMyString	strLnkPath;
} TLnk, *LPLnk;

BOOL GetLnkData(LPLnk);
BOOL SaveLnkData(LPLnk);

#endif
