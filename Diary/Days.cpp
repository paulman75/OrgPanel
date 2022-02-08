#include <windows.h>
#include "../Headers/general.h"
#include "Days.h"
#include "../Headers/DynList.h"

DynArray<TISKDAY> days;

#define HOLIDAY_COLOR 0x0dc

DWORD GetDayColor(TDate* date)
{
	const byte praz[14]={1,1,8,3,1,5,2,5,9,5,12,6,7,11};

	int i=0;
	while (i<days.count())
	{
		TISKDAY	mday=days[i];
		if (mday.Date==*date) return mday.Type ? HOLIDAY_COLOR : 0;
		i++;
	}
	int w=DayofWeek(date);
	DWORD res;
	if (w<6) res=0;
	else res=0x0dc;
	for (i=0; i<=6; i++)
		if (date->Day==praz[i*2] && date->Month==praz[i*2+1]) res=0x0dc;
	return (res);
}

void ChangeIskDay(TDate *date)
{
	BOOL bHol=GetDayColor(date)!=0;
	int i=0;
	TISKDAY	mday;
	while (i<days.count())
	{
		mday=days[i];
		if (mday.Date==*date)
		{
			days.Del(i);
			return;
		}
		i++;
	}
	mday.Date=*date;
	mday.Type=!bHol;
	days.Add(&mday);
}
