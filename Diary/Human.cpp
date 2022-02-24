#include <windows.h>
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "../Headers/exChange.h"
#include "classes.h"
#include "Pages.h"
#include "Human.h"
#include "IntEvent.h"

extern HWND hwndMain;
extern char	MainDir[200];
extern BOOL	bResident;
extern BOOL bFullImen;
extern BOOL bHumanExchangeOK;

int TimerID;
PEXCHANGE Humans=NULL;
int HumanCount;
extern TDate	OwnDate;

void DnevToScreen(BOOL bSetNoActive);

void AddRov(PPage pp)
{
	char *temp=(char*)malloc(100);
	char *temp2=(char*)malloc(100);
	if (pp->Date.Day==OwnDate.Day && pp->Date.Month==OwnDate.Month)
	{
		if (pp->Date.Year-OwnDate.Year>0)
			AddStrings("С днем рождения !!!",pp,ID_ROV,NULL,TRUE);
	}
	for (int i=0; i<HumanCount; i++)
	{
		if (pp->Date.Day==Humans[i].BDate.Day && pp->Date.Month==Humans[i].BDate.Month)
		{
			MakeShortName(Humans[i].FName[0] ? Humans[i].FName:NULL,Humans[i].OName[0] ? Humans[i].OName : NULL,Humans[i].SName[0] ? Humans[i].SName : NULL,temp);
			int d=pp->Date.Year-Humans[i].BDate.Year;
			if (d>=0)
			{
				if (d==0) strcpy_s(temp2,100,"Рождение!!!");
				else YearToString(temp2,d);
				lstrcat(temp," ");
				lstrcat(temp,temp2);
				AddStrings(temp,pp,ID_ROV,(void*)&Humans[i],TRUE);
			}
		}
	}
	free(temp);
	free(temp2);
}

void AddImen(PPage pp)
{
char* temp=(char*)malloc(200);
temp[0]=0;
char* temp2=(char*)malloc(100);

if (bFullImen)
{
	TDate im;
	int i=0;
	while (TRUE)
	{
		int j=0;
		while (TRUE)
		{
			if (!GetHumanImenDate(i,j,&im)) break;
			if (pp->Date.Day==im.Day && pp->Date.Month==im.Month)
			{
				if (!GetHumanName(i,temp2)) break;
				if (lstrlen(temp)<1) lstrcpy(temp,temp2);
				else
				{
					lstrcat(temp,", ");
					lstrcat(temp,temp2);
				}
			}
			j++;
		}
		if (!j) break;
		i++;
	}
	if (lstrlen(temp)>0) AddStrings(temp,pp,ID_IMEN,NULL,TRUE);
}
else
{
	for (int i=0; i<HumanCount; i++)
	{
		int NameIndex=GetNameIndex(Humans[i].FName);
		if (NameIndex!=-1)
		{
			TDate imen;
			if (GetHumanMainImenDate(NameIndex,&Humans[i].BDate,&imen))
			if (pp->Date.Day==imen.Day && pp->Date.Month==imen.Month)
			{
				MakeShortName(Humans[i].FName[0] ? Humans[i].FName:NULL,Humans[i].OName[0] ? Humans[i].OName : NULL,Humans[i].SName[0] ? Humans[i].SName : NULL,temp);
				AddStrings(temp,pp,ID_IMEN,(void*)&Humans[i],TRUE);
			}
		}
	}
}
free(temp);
free(temp2);
}

void EndInit()
{
	if (!bResident) ShowWindow(hwndMain, SW_SHOW);
}

void CALLBACK OnTimer(HWND hwnd, UINT msg, UINT ID, DWORD dwTime)
{
	KillTimer(NULL,TimerID);
	EndInit();
	UpdateIntEvent();
}

void FreeHuman()
{
	if (Humans)
	{
		free((void*)Humans);
		Humans=NULL;
	}
}

void OnStartExchange(int Size)
{
	FreeHuman();
	if (Size)
	{
		HANDLE hMap=OpenFileMapping(FILE_MAP_READ,FALSE,SHARE_NAME);
		if (hMap)
		{
			LPVOID lpView=MapViewOfFile(hMap,FILE_MAP_READ,0,0,0);
			if (lpView)
			{
				Humans=(PEXCHANGE)malloc(Size*sizeof(EXCHANGE));
				CopyMemory(Humans,lpView,Size*sizeof(EXCHANGE));
				HumanCount=Size;
				UnmapViewOfFile(lpView);
			}
			CloseHandle(hMap);
		}
	}
	long style=GetWindowLong(hwndMain,GWL_STYLE);
	if (style&WS_VISIBLE)
	{
		DnevToScreen(TRUE);
		PaintDnev();
	}
	else
	{
		KillTimer(NULL,TimerID);
		EndInit();
		UpdateIntEvent();
	}
	bHumanExchangeOK=TRUE;
}

void InitHuman()
{
	HWND hNote=FindWindow(NoteBookWindow,NULL);
	if (hNote)
	{
		PostMessage(hNote,WM_INITEXCHANGE,(WPARAM)hwndMain,0);
	}
	else
	{
		char *temp=(char*)malloc(300);
		lstrcpy(temp,MainDir);
		lstrcat(temp,"NoteBook.exe");
		HINSTANCE hIns=ShellExecute(hwndMain,NULL,temp,"/p",NULL,SW_HIDE);
		free(temp);
		if ((int)hIns<32)
		{
			EndInit();
			return;
		}
	}
	TimerID=SetTimer(NULL,0,1000,&OnTimer);
}

void FillBioCombo(HWND hwnd)
{
	char *temp=(char*)malloc(100);
	for (int i=0; i<HumanCount; i++)
	{
		MakeShortName(Humans[i].FName[0] ? Humans[i].FName:NULL,Humans[i].OName[0] ? Humans[i].OName : NULL,Humans[i].SName[0] ? Humans[i].SName : NULL,temp);
		int j=SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)temp);
		SendMessage(hwnd,CB_SETITEMDATA,j,(LPARAM)&Humans[i].BDate);
	}
	free(temp);
	SendMessage(hwnd,CB_SETCURSEL,0,0);
}

int GetHumanByKey(DWORD Key)
{
	int i=0;
	while (i<HumanCount)
	{
		if (Humans[i].Key==Key) return i;
		i++;
	}
	return -1;
}
