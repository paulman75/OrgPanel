#ifndef _DATETIME_H
#define _DATETIME_H

typedef struct TDateTime
{
	TDate	Date;
	byte	Hour;
	byte	Min;
	BOOL	operator <(const TDateTime &d1);
//	BOOL	operator <=(const TDate &d1);
	BOOL	operator >(const TDateTime &d1);
//	BOOL    operator ==(const TDate &d1);
	int		operator -(const TDateTime &d1);
	TDateTime&  operator +=(int);
	TDateTime&  operator -=(int);
//	TDate	operator --(int);
}TDateTime, *PDateTim;

TDateTime NowTime();

#endif

