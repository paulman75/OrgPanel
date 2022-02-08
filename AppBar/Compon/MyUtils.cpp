#include "windows.h"
#include "..\..\Headers\general.h"
#include "MyUtils.h"
#include "..\..\Headers\const.h"
#include "..\RegUnit.h"

extern TBarConfig	BarCon;

char sMonth[12][10]={"������","�������","����","������","���","����","����","������","��������","�������","������","�������"};
char sDayWeek[7][3]={"��","��","��","��","��","��","��"};
char srMonth[12][10]={"������","�������","�����","������","���","����","����","�������","��������","�������","������","�������"};
char sWeekDay[7][12]={"�����������","�������","�����","�������","�������","�������","�����������"};
extern char MainDir[100];

void ExtractCharAfterThis(LPCTSTR path, LPTSTR name, byte simbol, BOOL NeedCase)
{
	name[0]=0;
	WORD i=lstrlen(path)-1;
	if (i>60000) return;
	while (i && path[i--]!=simbol);
	if (i==0) return;
	i++;
	byte k=0;
	byte b;
	while (path[i]!=0) 
	{
		b=path[i++];
		if (NeedCase && b>64 && b<91) b+=32;
		name[k++]=b;
	}
	name[k]=0;
}

void ExtractFileExt(LPCTSTR path,LPTSTR buf)
{
	ExtractCharAfterThis(path,buf,0x2e, TRUE);
}

void ExtractFileName(LPCTSTR path,LPTSTR name)
{
	ExtractCharAfterThis(path,name,0x5c, FALSE);
	WORD i=1;
	do
		name[i-1]=name[i];
	while (name[i++]!=0);
}

void MyMoveFile(LPCTSTR sFrom, LPCTSTR sTo, LPCTSTR name)
{
	char* s1=(char*)malloc(300);
	char* s2=(char*)malloc(300);

	strcpy(s1,sFrom);
	strcat(s1,name);
	strcpy(s2,sTo);
	strcat(s2,name);
	if (s1[lstrlen(s1)-1]!='*')
		if (!FileExists(s1)) return;
	if (s2[lstrlen(s2)-3]=='*')	s2[lstrlen(s2)-2]=0;
	s1[lstrlen(s1)+1]=0;

	SHFILEOPSTRUCT fo;
	fo.hwnd=0;
	fo.wFunc=FO_COPY;
	fo.pFrom=s1;
	fo.pTo=s2;
	fo.fFlags=FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
	fo.fAnyOperationsAborted=FALSE;
	SHFileOperation(&fo);

	fo.hwnd=0;
	fo.wFunc=FO_DELETE;
	fo.pFrom=s1;
	fo.fFlags=FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
	fo.fAnyOperationsAborted=FALSE;
	SHFileOperation(&fo);
	free(s1);
	free(s2);
}

typedef struct
{
	char	Name[40];
	char	Shir[7];
	char	Dolg[7];
} TCITY;

#define CITY_COUNT 52
TCITY City[CITY_COUNT]=
{
{"�����","44.53","37.18"},
{"�����������","64.34","40.32"},
{"������","51.10","71.30"},
{"���������","46.21","48.03"},
{"�������","37.57","58.23"},
{"����","40.22","49.53"},
{"�������","53.22","83.45"},
{"�������� ","50.36","36.34"},
{"������","42.53","74.46"},
{"�����������","43.09","131.53"},
{"��������","56.10","40.25"},
{"���������","48.44","44.25"},
{"�������","67.27","63.58"},
{"������","40.10","44.31"},
{"�������","52.16","104.2"},
{"������","55.45","49.10"},
{"�����������","54.43","20.30"},
{"������","54.31","36.16"},
{"����","50.28","30.29"},
{"�������","47","28.5"},
{"��������� ","45.02","39.00"},
{"����������","56.05","92.46"},
{"������","55.26","65.18"},
{"�����","51.42","36.12"},
{"�������","59.34","150.48"},
{"���������","42.58","47.30"},
{"�����","53.51","27.3"},
{"������", "55.45","37.37"},
{"��������","68.58","33.05"},
{"�������","43.29","43.37"},
{"��������","58.3","31.2"},
{"�����������","55.04","82.55"},
{"��������","69.20","88.06"},
{"������","46.30","30.46"},
{"����","55.00","73.22"},
{"�����","58.0","56.15"},
{"������������","61.47","34.20"},
{"����","56.53","24.05"},
{"������","54.38","39.44"},
{"������","53.1","50.15"},
{"�����-���������","59.55","30.25"},
{"�������","51.34","46.02"},
{"����������","56.52","60.35"},
{"��������","54.47","32.03"},
{"��������","47.12","38.56"},
{"�������","41.43","44.48"},
{"�����","56.30","84.58"},
{"���","54.45","55.58"},
{"���������","48.27","135.06"},
{"�������","71.58","102.3"},
{"���������","55.10","61.25"},
{"����","52.03","113.30"}
};

void FillCity(HWND hCombo)
{
	for (int i=0; i<CITY_COUNT; i++)
		SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)City[i].Name);
}

void ChangeCity(HWND hCombo,HWND hD, HWND hS)
{
	int i=SendMessage(hCombo,CB_GETCURSEL,0,0);
	if (i==-1) return;
	SetWindowText(hD,City[i].Dolg);
	SetWindowText(hS,City[i].Shir);
}
