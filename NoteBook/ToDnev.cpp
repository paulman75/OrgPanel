#include <windows.h>
#include "ListView.h"
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "res/resource.h"
#include "Units.h"
#include "../Headers/Exchange.h"

extern LPNOTEUNIT	FirstUnit;
extern LPNOTEUNIT	nNote;

void CopyString(LPTSTR s1,LPCTSTR s2,int Size)
{
	if (s2)
	{
		int l=lstrlen(s2);
		if (l>Size-1) l=Size-1;
		strncpy_s(s1, Size, s2,l);
	}
	else s1[0]=0;
}

void OnInitExchange()
{
	HWND hFrom=FindWindow(DiaryWindow,NULL);
	if (!hFrom) return;
	PEXCHANGE Humans;
	BOOL bOk=FALSE;
	nNote=FirstUnit;
	int Count=0;
	while (nNote)
	{
		Count++;
		nNote=nNote->Next;
	}
	HANDLE hMap=NULL;
	if (Count) hMap=CreateFileMapping((HANDLE)0xffffffff,NULL,PAGE_READWRITE,0,Count*sizeof(EXCHANGE),SHARE_NAME);
	if (hMap)
	{
		Humans=(PEXCHANGE)MapViewOfFile(hMap,FILE_MAP_WRITE,0,0,0);
		if (Humans)
		{
			nNote=FirstUnit;
			int i=0;
			while (nNote)
			{
				Humans[i].BDate=nNote->BDate;
				Humans[i].Key=nNote->Key;
				CopyString(&Humans[i].FName[0],nNote->Fields[UN_NAME],30);
				CopyString(&Humans[i].OName[0],nNote->Fields[UN_MNAME],30);
				CopyString(&Humans[i].SName[0],nNote->Fields[UN_SNAME],40);
				i++;
				nNote=nNote->Next;
			}
			bOk=TRUE;
			SendMessage(hFrom,WM_STARTEXCHANGE,Count,0);
			UnmapViewOfFile(Humans);
		}
		CloseHandle(hMap);
	}
	if (!bOk) SendMessage(hFrom,WM_STARTEXCHANGE,0,0);
}

