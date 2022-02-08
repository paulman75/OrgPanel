#include "windows.h"
#define _MYDLL_
#include "../Headers/general.h"

extern LPVOID	lpImen;

typedef struct
{
     byte Month;
     byte Day;
}IMENDATE,*PIMENDATE;

LPBYTE FindNote(int iNameIndex)
{
	LPBYTE lb=(LPBYTE)lpImen;
	if (!*lb) return NULL;
	while (iNameIndex--)
	{
		if (!*lb) return NULL;
		byte l=*(lb++);
		lb+=l;
		l=*(lb++);
		lb+=l*sizeof(IMENDATE);
	}
	return lb;
}

BOOL __stdcall GetHumanName(int iNameIndex, LPTSTR name)
{
	LPBYTE lb=FindNote(iNameIndex);
	if (!lb) return FALSE;
	byte l=*(lb++);
	for (int i=0; i<l; i++) name[i]=lb[i];
	name[l]=0;
	return TRUE;
}

BOOL __stdcall GetHumanImenDate(int iNameIndex, int iDateIndex, PDate dat)
{
	LPBYTE lb=FindNote(iNameIndex);
	if (!lb) return FALSE;
	byte l=*(lb++);
	lb+=l;
	l=*(lb++);
	if (iDateIndex>=l) return FALSE;
	PIMENDATE pid=(PIMENDATE)(lb+iDateIndex*sizeof(IMENDATE));
	dat->Month=pid->Month;
	dat->Day=pid->Day;
	return TRUE;
}

int __stdcall GetNameIndex(LPCTSTR name)
{
	char* temp=(char*)malloc(100);
	BOOL b=FALSE;
	int i=0;
	while (TRUE)
	{
		if (!GetHumanName(i,temp)) break;
		if (!StringCompare(temp,name))
		{
			b=TRUE;
			break;
		}
		i++;
	}
	return b ? i:-1;
}

BOOL __stdcall GetHumanMainImenDate(int iNameIndex, PDate bDate, PDate dat)
{
	dat->Month=0;
	if (bDate->Month==0) return FALSE;
	TDate imdat2;
	int d=400;
	int j=0;
	while (TRUE)
	{
		if (!GetHumanImenDate(iNameIndex,j++,&imdat2)) break;
		imdat2.Year=bDate->Year;
		int dist=EncodeDate(bDate)-EncodeDate(&imdat2);
		if (dist<0) dist=-dist;
		if (dist>182) dist=365-dist;
		if (dist<d)
		{
			d=dist;
			*dat=imdat2;
		}
	}
	return dat->Month;
}