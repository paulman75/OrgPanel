#include "windows.h"
#include "../Headers/general.h"
#include "Main.h"
#include "../Addons/Logger/logger.h"

HINSTANCE hInstance;
extern DWORD IP;
extern SOCKET s;
extern LONGLONG g_LocalTimeOffset;
extern HWND g_hDlg;
extern int Progress;
extern LONGLONG g_TimeoutTime;
extern int Port;
extern HANDLE hDNS;
extern char hostentBuf[MAXGETHOSTSTRUCT];
extern DWORD Latency;
extern int CurrentServer;
extern const char* File[UPDATESERVER_COUNT];
extern BOOL bTime;
extern BOOL bAutoShow;
BOOL bCanExit;
HWND hOKBut;
int Timer_ID;

void CALLBACK InetTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	ServerStop(1);
}

void ServerConnected(int err)
{
#ifdef DEBUG
	WriteLog("ServerConnected Err=",err);
	WriteLog("g_hDlg=",(int)g_hDlg);
#endif
	if (!g_hDlg) return;
	if (err)
	{
		ServerStop((err == WSAECONNREFUSED) ? 
			eStatusRefused : eStatusNotResponding);
	}
	else
	{
		AddTextToRichEdit(ID_RICH,"Соединение установлено\r\n");
		Latency = GetTickCount();
		if (Port==80)
		{
			AddTextToRichEdit(ID_RICH,"Посылаем запрос1\r\n");
			char* bb=(char*)malloc(100);
			wsprintf(bb,"GET %s\r\n",File[CurrentServer]);
#ifdef DEBUG
	WriteLog("Посылаем запрос2");
#endif
			send(s, bb, strlen(bb), 0);
			free(bb);
		}
		Progress = eProgressReading;
	}
	g_TimeoutTime = GetMillisecondsSince1900() + 20 * 1000;
	if (!Timer_ID)
	{
#ifdef DEBUG
		WriteLog("Создаем таймер");
#endif
		Timer_ID=SetTimer(NULL,0,15000,&InetTimerProc);
	}
}

void ServerStop(int status)
{
	if (Timer_ID)
	{
		KillTimer(NULL,Timer_ID);
#ifdef DEBUG
	WriteLog("Убиваем таймер");
#endif
		Timer_ID=0;
	}
	if (!g_hDlg) return;
#ifdef DEBUG
	WriteLog("ServerStop status=",status);
#endif
#ifdef DEBUG
	WriteLog("ServerStop progress=",Progress);
#endif

	if (Progress)
	{
#ifdef DEBUG
		WriteLog("ServerStop hDNS=",hDNS ? 1:0);
#endif
		if (hDNS)
		{
			WSACancelAsyncRequest(hDNS);
			hDNS = 0;
		}
#ifdef DEBUG
	WriteLog("ServerStop s=",s);
#endif
		if (s)
		{
			WSAAsyncSelect(s, g_hDlg, 0, 0);
			closesocket(s);
			s = 0;
			WSACleanup();
		}
		Progress = eProgressNot;
		if (status==eStatusUsed) AddTextToRichEdit(ID_RICH,"От сервера получен ответ\r\n");
		else
		{
			switch (status)
			{
			case eStatusSlowResponding: AddTextToRichEdit(ID_RICH,"Таймаут истек\r\n");
				break;
			default: AddTextToRichEdit(ID_RICH,"Ошибка при подключении к серверу\r\n");
			}
		}
		if (Port==80) OnEndUpdateRequest(status==eStatusUsed);
		else OnEndTimeRequest(status==eStatusUsed);
	}
}

void ServerDNS(WORD error)
{
	if (!g_hDlg) return;
#ifdef DEBUG
	WriteLog("ServerDNS error=",error);
#endif
	AddTextToRichEdit(ID_RICH,"Обработка DNS\r\n");
	hDNS=0;
	switch (error)
	{
	case 0:
		IP =*(LPDWORD)(((struct hostent*)hostentBuf)->h_addr_list[0]);
		ServerGotIP();
		break;

	case WSAHOST_NOT_FOUND:
		ServerStop(eStatusNoDNSEntry);
		break;
	default:
		ServerStop(eStatusNetDown);
	}
	return;
}

void ServerGotIP()
{
	if (!g_hDlg) return;
#ifdef DEBUG
	WriteLog("Посылаем запрос3");
#endif
	AddTextToRichEdit(ID_RICH,"Посылаем запрос серверу ");
	SOCKADDR_IN sinClient, sinServer;
	int err;

    sinServer.sin_family = AF_INET;
    sinServer.sin_port = htons(Port);
    sinServer.sin_addr.S_un.S_addr = IP;

	char* ts=(char*)malloc(100);

	wsprintf(ts,"%d.%d.%d.%d\r\n",sinServer.sin_addr.S_un.S_un_b.s_b1,
		sinServer.sin_addr.S_un.S_un_b.s_b2,
		sinServer.sin_addr.S_un.S_un_b.s_b3,
		sinServer.sin_addr.S_un.S_un_b.s_b4);
	AddTextToRichEdit(ID_RICH,ts);
	free(ts);
    sinClient.sin_family = AF_INET;
    sinClient.sin_addr.S_un.S_addr = INADDR_ANY;
    sinClient.sin_port = 0;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
		ServerStop(eStatusNetError);
		return;
    }
    err = bind(s, (const struct sockaddr FAR *)&sinClient, 
		sizeof(SOCKADDR_IN));
    if (err)
    {
		ServerStop(eStatusNetError);
		return;
    }

	err = 1;
	err = setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (LPCSTR)&err, sizeof(err));
    if (err)
    {
		ServerStop(eStatusNetError);
		return;
    }

	err = WSAAsyncSelect(s, g_hDlg, WM_USER + 2, FD_CONNECT | FD_READ | FD_WRITE);
    if (err)
    {
		ServerStop(eStatusNetError);
		return;
	}
	Progress = eProgressConnecting;

    err = connect(s, (const struct sockaddr FAR *)&sinServer, 
		sizeof(SOCKADDR_IN));
    if (err && WSAGetLastError() != WSAEWOULDBLOCK)
    {
		ServerStop(eStatusNetError);
		return;
	}
	// Wait no more than 120 seconds for connect operation
	g_TimeoutTime = GetMillisecondsSince1900() + 120 * 1000;
}

void Cleanup()
{
	if (Progress) ServerStop(eStatusInterrupted);
	WSACleanup();
}

void ServerStart(char* name)
{
	// Wait no more than 120 seconds for user to connect
	g_TimeoutTime = GetMillisecondsSince1900() + 120 * 1000;

#ifdef DEBUG
	WriteLog("Подключаемся к серверу");
#endif

	AddTextToRichEdit(ID_RICH,"Подключаемся к серверу ");
	AddTextToRichEdit(ID_RICH,name);
	AddTextToRichEdit(ID_RICH,"\r\n");

#ifdef DEBUG
	WriteLog(name);
#endif

	IP = ntohl(inet_addr(name));
	if (IP == INADDR_NONE)
	{
		Progress = eProgressLookup;
		hDNS = WSAAsyncGetHostByName(g_hDlg, WM_USER + 1, 
			name, hostentBuf, sizeof(hostentBuf));
		if (!hDNS) ServerStop(eStatusNetError);
	}
	else ServerGotIP();
}

LONG FAR PASCAL UpdateWndProc(HWND hDlg, unsigned uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:
		break;
		case WM_ENDSESSION:
			DestroyWindow(hDlg);
			return TRUE;

		case WM_USER + 1: // Asynch Winsock callback for DNS
#ifdef DEBUG
	WriteLog("FD_DNS",lParam);
#endif
			ServerDNS(WSAGETASYNCERROR(lParam));
			return TRUE;

		case WM_USER + 2: // Asynch Winsock callback for connect/read
			{
				switch (WSAGETSELECTEVENT(lParam))
				{
				case FD_CONNECT:
#ifdef DEBUG
	WriteLog("FD_CONNECT",lParam);
#endif
					ServerConnected(WSAGETSELECTERROR(lParam));
					break;
				case FD_READ:
#ifdef DEBUG
	WriteLog("FD_READ",lParam);
#endif
					if (bTime) ServerTimeRead(WSAGETSELECTERROR(lParam));
					else ServerUpdateRead(WSAGETSELECTERROR(lParam));
					break;
				case FD_WRITE:
#ifdef DEBUG
	WriteLog("FD_WRITE",lParam);
#endif
					AddTextToRichEdit(ID_RICH,"Запрос послан\r\n");
					break;
				}
			}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDCANCEL:
			case IDOK:
				DestroyWindow(hDlg);
				return TRUE;
			}
			break;
		case WM_NOTIFY:
			if (LOWORD(wParam)==ID_RICH)
				return RichEditWndProc(hDlg,uMsg,wParam,lParam,ID_RICH);
			break;

		case WM_DESTROY:
			if (Timer_ID)
			{
				KillTimer(NULL,Timer_ID);
#ifdef DEBUG
	WriteLog("Убиваем таймер");
#endif
				Timer_ID=0;
			}
			g_hDlg = 0;
			Cleanup();
			DeleteAllRichEdit();
			bCanExit=TRUE;
			break;
	}
    return(DefWindowProc(hDlg, uMsg, wParam, lParam));
}

BOOL InitUpdate(HWND hOwner)
{
    WNDCLASS   wc;

	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= "OrgPanelUpdateWindow";
    wc.hInstance        = hInstance;
	wc.hIcon			= LoadIcon(hInstance,"MAIN");
    wc.hCursor          = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
    wc.style            = 0;
	wc.lpfnWndProc		= UpdateWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;

    if (!RegisterClass(&wc)) return TRUE;

	if ((g_hDlg = CreateWindow("OrgPanelUpdateWindow",
		bTime ? "Синхронизация времени" : "Проверка обновлений",
                                 WS_OVERLAPPED |  WS_SYSMENU,
                                 0, 0,
                                 350, 300,
                                 hOwner, NULL, hInstance, NULL)) == NULL)
		return TRUE;

	hOKBut=CreateWindow("BUTTON","Cancel",WS_CHILD | WS_VISIBLE | BS_NOTIFY,145,240,60,25,g_hDlg,NULL,hInstance,NULL);
	HFONT hFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hOKBut,WM_SETFONT,(WPARAM)hFont,0);
	SetWindowLong(hOKBut,GWL_ID,IDOK);
	CreateRichEdit(g_hDlg, 5, 5, 338, 230, ID_RICH);
	PutOnCenter(g_hDlg);
	HWND hRich=GetRichEditWnd(ID_RICH);
	SendMessage(hRich,EM_SETREADONLY,1,0);
	bCanExit=FALSE;
	UpdateWindow(g_hDlg);
	ShowWindow(g_hDlg,bAutoShow ? SW_SHOW : SW_HIDE);
	Timer_ID=0;
	if (bTime) MakeTimeStart();
	else MakeUpdateStart();

	MSG        msgMain;
    while (!bCanExit && GetMessage((LPMSG) &msgMain, NULL, 0, 0))
    {
		TranslateMessage((LPMSG) &msgMain);
		DispatchMessage((LPMSG) &msgMain);
    }
#ifdef DEBUG
	WriteLog("Вышли из цикла");
#endif

	if (g_hDlg) 
	{
		MessageBox(hOwner,"sdf","werwet",0);
		DestroyWindow(g_hDlg);
	}
#ifdef DEBUG
	WriteLog("Выходим из Main",bCanExit);
#endif

	return bCanExit;
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, 
                       LPVOID lpReserved)
{
    hInstance = (HINSTANCE)hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#ifdef DEBUG
		InitLog("c:\\orginet.log");
#endif
		break;
	case DLL_PROCESS_DETACH:
#ifdef DEBUG
	WriteLog("DETACH");
#endif
		break;
	}
    return TRUE;
}

