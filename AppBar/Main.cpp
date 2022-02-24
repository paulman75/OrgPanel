#include "windows.h"
#include "AppBar.h"
#include "BarUnit.h"
#include "..\Headers\general.h"
#include "..\Headers\const.h"
#include "Compon\MyUtils.h"
#include "RegUnit.h"
#include "Option.h"
#include "Compon\Classes.h"
#include "Objbase.h"

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow);
LONG FAR PASCAL HookSampleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------------------
// Global Variables...
//---------------------------------------------------------------------------

HWND	hwndMain;		// Main hwnd.  Needed in callback
HINSTANCE hInstance;		  // Global instance handle for		application
HINSTANCE hModUser32;

HPEN hpnColor1,hGrayPen,hWhitePen,hpnBlack;
HPEN	hpnBlue,hpnRed,hpnGreen,hpnBlue2;
HBRUSH	hGreenBrush,hBlackBrush,hSilverBrush,hWhiteBrush,hBlueBrush;
HFONT	hMainFont;
char	buf[1000],buf2[230];

char MainBarDir[200];
char MainDir[200];
char MainPathFile[200];
//---------------------------------------------------------------------------
// WinMain
//---------------------------------------------------------------------------

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow)
{
    MSG        msgMain;
    WNDCLASS   wc;

    hInstance = hInst;
    //
    // Register the window class if this is the first instance.
    //
		hwndMain=FindWindow("BarWindow",NULL);
		if (hwndMain!=0) 
		{
			SendMessage(hwndMain,WM_ACTIVATEAPP,1,0);
			return 0;
		}

	GetModuleFileName(hInstance, MainPathFile, sizeof(MainPathFile));
	GetModulePath(hInstance,MainDir,200);
	GetDataDir(MainBarDir,200);

	if (!DirectoryExists(MainBarDir))
	{
		// Проверим есть ли файлы данных в старом каталоге
		if (CreateDirectory(MainBarDir,NULL))
		{
			char* OldDir=(char*)malloc(300);
			strcpy_s(OldDir,300,MainDir);
			MyMoveFile(OldDir,MainBarDir,NOTEBOOK_FILE);
			MyMoveFile(OldDir,MainBarDir,NOTEPAD_FILE);
			MyMoveFile(OldDir,MainBarDir,DNEV_FILE);
			MyMoveFile(OldDir,MainBarDir,"col.ini");
			MyMoveFile(OldDir,MainBarDir,"Bar");
			lstrcat(OldDir,"Bar");
			RemoveDirectory(OldDir);
			free(OldDir);
		}
		else strcpy_s(MainBarDir, 200, MainDir);
	}
	lstrcat(MainBarDir,"Bar\\");

		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= "OrgPanel Window_Hidden";
        wc.hInstance        = hInst;
		wc.hIcon			= LoadIcon(hInstance,"AMainIcon");
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= NULL;
        wc.style            = 0;
		wc.lpfnWndProc		= HookSampleWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;

        if (!RegisterClass(&wc))
            return(0);

		wc.lpszClassName	= "BarWindow";
		wc.hIcon			= NULL;
        wc.hCursor          = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+2);
		wc.lpfnWndProc		= BarWndProc;

        if (!RegisterClass(&wc))
            return(0);

	hMainFont=CreateFontIndirect(&lf);
	hpnColor1 = CreatePen(PS_SOLID,1,0x0e0e0e0);
	hWhitePen = CreatePen(PS_SOLID,1,0x0ffffff);
	hGrayPen = CreatePen(PS_SOLID,1,0x808080);
	hpnBlack	= CreatePen(PS_SOLID,1,0);
	hpnGreen	= CreatePen(PS_SOLID,1,0x8000);
	hpnRed		= CreatePen(PS_SOLID,1,0xdc);
	hpnBlue2	= CreatePen(PS_SOLID,1,0xc06810);
	hpnBlue		= CreatePen(PS_SOLID,1,0xff0000);
	hBlackBrush = CreateSolidBrush(0);
	hGreenBrush=CreateSolidBrush(0xc0c000);
	hSilverBrush=CreateSolidBrush(0xc0c0c0);
	hWhiteBrush=CreateSolidBrush(0xffffff);
	hBlueBrush=CreateSolidBrush(0xff0000);
	CoInitialize(NULL);
	hModUser32 = LoadLibrary("user32.dll");
    //
    // Create the main window
    //
    if ((hwndMain = CreateWindowEx(WS_EX_TOOLWINDOW,"OrgPanel Window_Hidden",
				 "OrgPanel MainWindow",
                                 WS_POPUP|WS_SYSMENU,
                                 200, 0,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 NULL, NULL, hInst, NULL)) == NULL)
        return(0);
	HWND BarWin;

    if ((BarWin = CreateWindowEx(WS_EX_ACCEPTFILES,"BarWindow",
				 "OrgPanel Bar",
                                 WS_POPUP | WS_CHILD,
                                 20, 20,
                                 400, 400,
                                 hwndMain, NULL, hInst, NULL)) == NULL)
        return(0);
	//
    // Show the window and make sure it is updated.
    //
    ShowWindow(hwndMain, SW_HIDE);
	ShowWindow(BarWin,SW_SHOW);
    UpdateWindow(hwndMain);
	UpdateWindow(BarWin);
	ShowOptions(FALSE);

	BadRegLnk(hwndMain); //Установлен твикер

    while (GetMessage((LPMSG) &msgMain, NULL, 0, 0))
    {
       TranslateMessage((LPMSG) &msgMain);
       DispatchMessage((LPMSG) &msgMain);
    }
	if (hModUser32) FreeLibrary(hModUser32);
	CoUninitialize();
    DeleteObject(hpnColor1);
	DeleteObject(hGrayPen);
    DeleteObject(hWhitePen);
	DeleteObject(hpnBlack);
	DeleteObject(hpnBlue);
	DeleteObject(hpnBlue2);
	DeleteObject(hpnGreen);
	DeleteObject(hpnRed);
	DeleteObject(hBlackBrush);
	DeleteObject(hBlueBrush);
	DeleteObject(hGreenBrush);
	DeleteObject(hSilverBrush);
	DeleteObject(hWhiteBrush);
	UnregisterClass("OrgPanel Window_Hidden",hInstance);
	UnregisterClass("BarWindow",hInstance);
	DeleteObject(hMainFont);
	ExitFromProgram();
    return(0);
}

LONG FAR PASCAL HookSampleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) {

	case WM_ENDSESSION:
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
            return(DefWindowProc(hwnd, msg, wParam, lParam));
    }
    return(0);
}