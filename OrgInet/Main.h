#ifndef __MAIN_H
#define __MAIN_H

#define MYAPI2 __declspec(dllexport)

extern "C" MYAPI2 BOOL __stdcall InetTimeSync(BOOL bShow, HWND hwnd);
extern "C" MYAPI2 BOOL __stdcall InetCheckUpdate(HWND hwnd);

#define UPDATESERVER_COUNT 2

BOOL InitUpdate(HWND hOwner);
void ServerGotIP();
void ServerDNS(WORD error);
void Cleanup();
void ServerStart(char* name);
LONGLONG GetMillisecondsSince1900();
void ServerStop(int status);
void ServerConnected(int err);

void MakeUpdateStart();
void MakeTimeStart();
void ServerUpdateRead(int err);
void ServerTimeRead(int err);


void OnEndUpdateRequest(BOOL bOK);
void OnEndTimeRequest(BOOL bOK);

enum 
{
	eStatusNothing,
	eStatusNetError,
	eStatusNotResponding,
	eStatusSlowResponding,
	eStatusRefused,
	eStatusNetDown,
	eStatusNoDNSEntry,
	eStatusDisabled,
	eStatusInterrupted,
	eStatusThrownOut = 100,
	eLogic,
	eStatusUsed         // This is the good status
};

enum
{
	eProgressNot,
	eProgressLookup,
	eProgressConnecting,
	eProgressSending,
	eProgressReading
};

#define A_NOTEPAD	0
#define A_APPBAR	1
#define A_NOTEBOOK	2
#define A_ORGPANEL	3
#define A_ORGINET	4
#define A_DIARY		5
#define A_PASSKEAPER	6
#define A_INITCONV	7

#define ID_RICH 10

typedef struct
{
	int Version;
	int Build[8];
	char Text[500];
	char Url[4][100];
	char Prim[200];
	int Reserved;
} UPDATEINFO;

#endif