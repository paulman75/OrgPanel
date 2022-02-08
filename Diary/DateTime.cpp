#include <windows.h>
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "DateTime.h"

TDateTime& TDateTime::operator +=(int v)
{
	v+=Min;
	while (v>=60)
	{
		v-=60;
		Hour++;
		if (Hour==24)
		{
			Hour=0;
			Date++;
		}
	}
	Min=(byte)v;
	return *this;
}

TDateTime& TDateTime::operator -=(int v)
{
	int m=Min-v;
	while (m<0)
	{
		m+=60;
		if (Hour==0)
		{
			Hour=23;
			Date--;
		}
		else Hour--;
	}
	Min=(byte)m;
	return *this;
}

int	TDateTime::operator -(const TDateTime &d1)
{
	DWORD dw1=EncodeDate(&Date)*24*60;
	DWORD dw2=EncodeDate((TDate*)&d1.Date)*24*60;
	dw1+=Hour*60+Min;
	dw2+=d1.Hour*60+d1.Min;
	return dw1-dw2;
}

BOOL TDateTime::operator >(const TDateTime &d1)
{
	if (Date>d1.Date) return TRUE;
	if (Date<d1.Date) return FALSE;
	if (Hour>d1.Hour ||
		(Hour==d1.Hour && Min>d1.Min)) return TRUE;
	return FALSE;
}

BOOL TDateTime::operator <(const TDateTime &d1)
{
	if (Date<d1.Date) return TRUE;
	if (Date>d1.Date) return FALSE;
	if (Hour<d1.Hour ||
		(Hour==d1.Hour && Min<d1.Min)) return TRUE;
	return FALSE;
}

TDateTime NowTime()
{
	SYSTEMTIME SysTim;
	GetLocalTime(&SysTim);
	TDateTime	res;
	res.Date.Year=SysTim.wYear;
	res.Date.Month=(byte)SysTim.wMonth;
	res.Date.Day=(byte)SysTim.wDay;
	res.Hour=(byte)SysTim.wHour;
	res.Min=(byte)SysTim.wMinute;
	return res;
}