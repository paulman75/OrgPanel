#ifndef __EXCHANGE_H
#define __EXCHANGE_H

typedef struct
{
	DWORD Key;
	char  FName[30];
	char  OName[30];
	char  SName[40];
	TDate BDate;
} EXCHANGE, *PEXCHANGE;

#define WM_STARTEXCHANGE	WM_USER+456
//wParam-размер шаре
#define WM_INITEXCHANGE		WM_USER+457

#define WM_OPENHUMAN		WM_USER+458
//wParam-Key

#define SHARE_NAME	"NoteBookShareMem"

#endif