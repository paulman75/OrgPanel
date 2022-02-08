#include "windows.h"
#include "../Headers/general.h"
#include "units.h"

extern LPNOTEUNIT	FirstUnit;
extern LPNOTEUNIT	nNote;
extern LPFIRMUNIT	FirstFirm;
extern LPFIRMUNIT	nfNote;

LPTSTR ReadString(HANDLE hBookFile)
{
	DWORD dw;
	byte l;
	ReadFile(hBookFile,&l,1,&dw,NULL);
	if (!l) return NULL;
	LPTSTR s=(LPTSTR)malloc(l+1);
	ReadFile(hBookFile,s,l,&dw,NULL);
	s[l]=0;
	while (l>0) s[-1+l--]+=26;
	return s;
}

void LoadOldVersion(HANDLE hFile, char vers)
{
	LPNOTEUNIT nPrevNote=NULL;
	byte *buf=(byte*)malloc(1000);
	DWORD dwKey=2000;
	for (int i=0; i<32; i++)
	while (TRUE)
	{
		DWORD dw;
		ReadFile(hFile,buf,1,&dw,NULL);
		if (!buf[0]) break;
		else
		{
			nNote=(LPNOTEUNIT)malloc(sizeof(TNOTEUNIT));
			ZeroMemory(nNote,sizeof(TNOTEUNIT));
			if (nPrevNote) nPrevNote->Next=nNote;
			else FirstUnit=nNote;
			nNote->Next=NULL;
			nNote->Firm=-1;
			nNote->Key=dwKey--;
			nNote->Group=-1;
			nPrevNote=nNote;
			nNote->BDate.Day=buf[0];
			ReadFile(hFile,buf,3,&dw,NULL);
			nNote->BDate.Month=buf[0];
			nNote->BDate.Year=buf[1]+256*buf[2];
			if (nNote->BDate.Day==1 && nNote->BDate.Month==1 && nNote->BDate.Year==1980) nNote->BDate.Month=0;
			byte OldNewIndex[11]={0,1,2,3,4,9,16,6,7,8,17};
			byte OldIndex[11]={0,2,1,5,3,4,10,8,6,7,9};
			byte j;
			for (j=0; j<11; j++) 
			{
				if (vers==2) nNote->Fields[OldNewIndex[j]]=ReadString(hFile);
				else
				{
					if (j<8) nNote->Fields[OldNewIndex[OldIndex[j]]]=ReadString(hFile);
				}
			}
			short s;
			if (vers==2)	ReadFile(hFile,&s,sizeof(short),&dw,NULL);
		}
	}
	LPFIRMUNIT nPrevFirm=NULL;
	dwKey=1000;
	for (int i=0; i<32; i++)
	while (TRUE)
	{
		DWORD dw;
		ReadFile(hFile,buf,1,&dw,NULL);
		if (!buf[0]) break;
		else
		{
			nfNote=(LPFIRMUNIT)malloc(sizeof(TFIRMUNIT));
			ZeroMemory(nfNote,sizeof(TFIRMUNIT));
			if (nPrevFirm) nPrevFirm->Next=nfNote;
			else FirstFirm=nfNote;
			nfNote->Next=NULL;
			nfNote->Key=dwKey--;
			nPrevFirm=nfNote;
			ReadFile(hFile,buf,3,&dw,NULL);
			byte OldNewIndex[11]={0,1,2,3,4,5,9,6,7,8,13};
			byte OldIndex[11]={0,2,1,5,3,4,10,8,6,7,9};
			for (byte j=0; j<11; j++) 
			{
				if (vers==2) nfNote->Fields[OldNewIndex[j]]=ReadString(hFile);
				else
				{
					if (j<8) nfNote->Fields[OldNewIndex[OldIndex[j]]]=ReadString(hFile);
				}
			}
			short s;
			if (vers==2)	ReadFile(hFile,&s,sizeof(short),&dw,NULL);
		}
	}
	free(buf);
}
