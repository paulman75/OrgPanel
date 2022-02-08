#include "windows.h"
#define _MYDLL_
#include "../Headers/general.h"
#include "res/resource.h"
#include "regUnit.h"

HINSTANCE hInstance;
HFONT	hMainFont;
HPEN hGrayPen, hWhitePen,hpnBlack,hpnColor1;
LPVOID	lpBeepWave;
HANDLE	hImen;
LPVOID	lpImen;
BOOL	bShowFullName;

CONST LOGFONT lf = {
    16,                        //int   lfHeight;
    0,                         //int   lfWidth;
    0,                         //int   lfEscapement;
    0,                         //int   lfOrientation;
    FW_DONTCARE,               //int   lfWeight;
    0,                         //BYTE  lfItalic;
    0,                         //BYTE  lfUnderline;
    0,                         //BYTE  lfStrikeOut;
    RUSSIAN_CHARSET,           //BYTE  lfCharSet;
    OUT_DEFAULT_PRECIS,        //BYTE  lfOutPrecision;
    CLIP_DEFAULT_PRECIS,       //BYTE  lfClipPrecision;
    DEFAULT_QUALITY,           //BYTE  lfQuality;
    DEFAULT_PITCH | FF_SWISS,  //BYTE  lfPitchAndFamily;
    "MS Sans Serif",           //BYTE  lfFaceName[LF_FACESIZE];
    };

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, 
                       LPVOID lpReserved)
{
    hInstance = (HINSTANCE)hModule;
	HRSRC hr;
	HGLOBAL	hGlob;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ChangeSoundConfig();
		hMainFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
		hWhitePen = CreatePen(PS_SOLID,1,0x0ffffff);
		hGrayPen = CreatePen(PS_SOLID,1,0x808080);
		hpnColor1 = CreatePen(PS_SOLID,1,0x0e0e0e0);
		hpnBlack	= CreatePen(PS_SOLID,1,0);
		hr=FindResource(hInstance,(LPCTSTR)IDR_BEEPWAVE,"WAVE");
		hGlob=LoadResource(hInstance,hr);
		lpBeepWave=LockResource(hGlob);
	//	waveOutGetNumDevs(); почему падает при вызове 30.10.11
		hImen=LoadResource(hInstance, FindResource(hInstance, (LPCTSTR)IDR_IMEN, "DAT"));
		lpImen=LockResource(hImen);
		CheckCodePage();
		DWORD dw;
		ReadRegistry("ShowFullName",(byte*)&dw,4);
		bShowFullName=(byte)dw;
		break;
	case DLL_PROCESS_DETACH:
	    FreeResource(hImen);
		DeleteObject(hWhitePen);
		DeleteObject(hGrayPen);
		DeleteObject(hMainFont);
		DeleteObject(hpnBlack);
		DeleteObject(hpnColor1);
		break;
	}
    return TRUE;
}
