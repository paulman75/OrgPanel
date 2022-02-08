#include "windows.h"
#include "mytapi.h"
#include "res/resource.h"
#include <tapi.h>
#include "../Headers/general.h"

HLINEAPP		lineApp;
HLINE		line;
HCALL		call;
LINECALLPARAMS	CallParams;

extern HINSTANCE hInstance;		  // Global instance handle for application
LPTSTR Telefon;

typedef LONG (WINAPI *LINEMAKECALL)(HLINE hLine, LPHCALL lphCall, LPCSTR lpszDestAddress, 
    DWORD dwCountryCode, LPLINECALLPARAMS const lpCallParams);
typedef LONG (WINAPI *LINEDEALLOCATECALL)(HCALL hCall);
typedef LONG (WINAPI *LINEDROP)(HCALL hCall, LPCSTR lpsUserUserInfo, DWORD dwSize);
typedef LONG (WINAPI *LINEOPENA)(HLINEAPP hLineApp, DWORD dwDeviceID, LPHLINE lphLine, 
    DWORD dwAPIVersion, DWORD dwExtVersion, DWORD dwCallbackInstance, 
    DWORD dwPrivileges, DWORD dwMediaModes, 
    LPLINECALLPARAMS const lpCallParams);
typedef LONG (WINAPI *LINECLOSE)(HLINE hLine);
typedef LONG (WINAPI *LINESHUTDOWN)(HLINEAPP hLineApp);
typedef LONG (WINAPI *LINEINITIALIZE)(LPHLINEAPP lphLineApp, HINSTANCE hInstance, 
    LINECALLBACK lpfnCallback, LPCSTR lpszAppName, LPDWORD lpdwNumDevs);
typedef LONG (WINAPI *LINENEGOTIATEAPIVERSION)(HLINEAPP hLineApp, DWORD dwDeviceID, 
    DWORD dwAPILowVersion, DWORD dwAPIHighVersion, 
    LPDWORD lpdwAPIVersion, LPLINEEXTENSIONID lpExtensionID);


LINEMAKECALL		plineMakeCall=NULL;
LINEDEALLOCATECALL	plineDeallocateCall=NULL;
LINEDROP			plineDrop=NULL;
LINEOPENA			plineOpen=NULL;
LINECLOSE			plineClose=NULL;
LINESHUTDOWN		plineShutdown=NULL;
LINEINITIALIZE		plineInitialize=NULL;
LINENEGOTIATEAPIVERSION	plineNegotiateAPIVersion=NULL;

void __stdcall lineCallback(DWORD hDevice, DWORD dwMsg, DWORD dwCallbackInstance,
		DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	HCALL	hCall;

	if (dwMsg!=LINE_REPLY)
	if (dwMsg==LINE_CALLSTATE)
	{
		hCall = HCALL(hDevice);
		switch (dwParam1)
		{
			case LINECALLSTATE_IDLE:
				if (hCall)
				{
					plineDeallocateCall(hCall);
					plineMakeCall(line, &call, Telefon, 0, &CallParams);
				}
				break;
			case LINECALLSTATE_DISCONNECTED:
					plineDrop(call, NULL, 0);
					break;			
		}
	}
}

BOOL TapiInit()
{
	DWORD				nDevs, tapiVersion;
	LINEEXTENSIONID		extid;

	FillMemory(&CallParams, sizeof(CallParams), 0);
	CallParams.dwTotalSize=sizeof(CallParams);
	CallParams.dwBearerMode = LINEBEARERMODE_VOICE;
//	CallParams.dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
	CallParams.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
	if (plineInitialize(&lineApp, hInstance,
			lineCallback, NULL, &nDevs)<0)	lineApp = 0;
	else if (!nDevs)
	{
		plineShutdown(lineApp);
		lineApp = 0;
	}
	else if (plineNegotiateAPIVersion(lineApp, 0, 0x00010000, 0x10000000,
			&tapiVersion, &extid)<0)
	{
		plineShutdown(lineApp);
		lineApp = 0;
	}
	else if (plineOpen(lineApp, LINEMAPPER, &line, tapiVersion, 0, 0,
			LINECALLPRIVILEGE_NONE, 0, &CallParams)<0)
	{
		plineShutdown(lineApp);
		lineApp = 0;
		line = 0;
	}
	return (line!=0);
}
	
BOOL FAR PASCAL DialDialog(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:		  // message: initialize dialog box
		PutOnCenter(hDlg);
		SendMessage(GetDlgItem(hDlg,IDC_STATICNOMER),WM_SETTEXT,0,(LPARAM)Telefon);
		return (TRUE);
	case WM_COMMAND:		  // message: received a command
		if ( LOWORD(wParam) == IDOK		  // "OK" box selected?
		||	 LOWORD(wParam) == IDCANCEL)
		{	
			EndDialog(hDlg, TRUE);	  // Exits the dialog box
			return (TRUE);
	    }
	    break;
    }
    return (FALSE);			  // Didn't process a message
}

void MakeCall(HWND hwnd, LPCTSTR number)
{
	HMODULE hLib=LoadLibrary("TAPI32.DLL");

	if (hLib)
	{
		plineOpen=(LINEOPENA) GetProcAddress(hLib, "lineOpen");
		plineMakeCall=(LINEMAKECALL) GetProcAddress(hLib, "lineMakeCall");
		plineDeallocateCall=(LINEDEALLOCATECALL) GetProcAddress(hLib, "lineDeallocateCall");
		plineDrop=(LINEDROP) GetProcAddress(hLib, "lineDrop");
		plineClose=(LINECLOSE) GetProcAddress(hLib, "lineClose");
		plineShutdown=(LINESHUTDOWN) GetProcAddress(hLib, "lineShutdown");
		plineInitialize=(LINEINITIALIZE) GetProcAddress(hLib, "lineInitialize");
		plineNegotiateAPIVersion=(LINENEGOTIATEAPIVERSION) GetProcAddress(hLib, "lineNegotiateAPIVersion");
		if (!plineOpen || !plineMakeCall || !plineDeallocateCall || !plineDrop ||
			!plineClose || !plineShutdown || !plineInitialize || !plineNegotiateAPIVersion)
		{
			MessageBox(hwnd,"Невозможно установить связи с библиотекой tapi32.dll","Ошибка",MB_ICONSTOP);
			FreeLibrary(hLib);
			return;
		}
	}
	else
	{
		MessageBox(hwnd,"Невозможно загрузить динамическую библиотеку tapi32.dll","Ошибка",MB_ICONSTOP);
		return;
	}

	if (!TapiInit())
	{
		MessageBox(hwnd,"Невозможно инициализировать соединение","Ошибка",MB_ICONSTOP);
		return;
	};
	Telefon=(char*)malloc(lstrlen(number)+1);
	lstrcpy(Telefon,number);
	if (plineMakeCall(line, &call, Telefon, 0, &CallParams)<0)
	{
		MessageBox(hwnd,"Невозможно выполнить набор номера","Ошибка",MB_ICONSTOP);
		free(Telefon);
		return;
	};
	DialogBox(hInstance,		 // current instance
				"DIAL",			 // resource to use
				hwnd,			 // parent handle
				(DLGPROC)DialDialog);			  // About() instance address
	free(Telefon);
	plineDrop(call, NULL, 0);
	if (line) plineClose(line);
	if (lineApp) plineShutdown(lineApp);
	FreeLibrary(hLib);
}
