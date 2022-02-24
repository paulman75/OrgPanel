#include "windows.h"
#include "units.h"
#include "OldVers.h"
#include "../Headers/DynList.h"

extern HWND hwndMain;
extern DynArray<TGROUP> GroupList;

BOOL CheckLen(HANDLE hFile, byte Len)
{
	SetFilePointer(hFile,0,0,FILE_BEGIN);
	DWORD	dw;
	for (int i=0; i<32; i++)
	{
		byte buf[2];
		if (!ReadFile(hFile,buf,Len,&dw,NULL)) return FALSE;
		WORD l;
		if (Len==1) l=buf[0];
		else l=buf[0]*256+buf[1];
		if (l>4000)	return FALSE;
		if (l!=0) SetFilePointer(hFile,l,0,FILE_CURRENT);
	}
	dw=SetFilePointer(hFile,0,0,FILE_CURRENT);
	DWORD dw2=SetFilePointer(hFile,0,0,FILE_END);
	if (dw2!=dw) return FALSE;
	return TRUE;
}

void LoadOldVersion(HANDLE hFile, int vVersion)
{

	if (vVersion!='3')
	{
		MessageBox(hwndMain,"Файл не является файлом записной книжки","Ошибка",MB_ICONSTOP);
		return;
	}
	TGROUP tg;
	tg.Group_ID=0;
	tg.ID=1;
	tg.LoadIndex=1;
	tg.St=NULL;
	strcpy_s(tg.Name, GROUPNAME_SIZE, "Все разделы");
	GroupList.Add(&tg);
	int ID=2;
	while (TRUE)
	{
		char* s=ReadString(hFile);
		if (!s) break;
		tg.Group_ID=1;
		tg.LoadIndex=ID;
		tg.ID=ID++;
		lstrcpy(tg.Name,s);
		tg.St=ReadString(hFile);
		GroupList.Add(&tg);
	}
}

void LoadVeryOldVersion(HANDLE hFile)
{
/*	byte Len=2;
	if (!CheckLen(hFile,2))
	{
		if (!CheckLen(hFile,1)) return;
		Len=1;
	}
	SetFilePointer(hFile,0,0,FILE_BEGIN);
	byte i;
	DWORD	dw;
	LPUNIT prev=NULL;
	for (i=0; i<32; i++)
	{
		byte buf[2];
		if (!ReadFile(hFile,buf,Len,&dw,NULL)) break;
		WORD l;
		if (Len==1) l=buf[0];
		else l=buf[0]*256+buf[1];
		if (l)
		{
			mu=(LPUNIT)malloc(sizeof(UNIT));
			mu->Caption=(char*)malloc(2);
			mu->Caption[0]=i+'А';
			mu->Caption[1]=0;
			mu->St=(char*)malloc(l+1);
			ReadFile(hFile,mu->St,l,&dw,NULL);
			mu->St[l]=0;
	        WORD j;
			for (j=0; j<l; j++) mu->St[j]-=30;
			mu->next=NULL;
			mu->prev=prev;
			if (prev) prev->next=mu;
			else First=mu;
			prev=mu;
		}
	}*/
}
