#include "windows.h"
#include "general.h"

BOOL TDate::operator <(const TDate &d1)
{
	if (!Month || !d1.Month) return FALSE;
	if (Year<d1.Year) return TRUE;
	if (Year==d1.Year && Month<d1.Month) return TRUE;
	if (Year==d1.Year && Month==d1.Month && Day<d1.Day) return TRUE;
	return FALSE;
}

BOOL TDate::operator <=(const TDate &d1)
{
	return !(*this>d1);
}

BOOL TDate::operator >(const TDate &d1)
{
	if (Year>d1.Year) return TRUE;
	if (Year==d1.Year && Month>d1.Month) return TRUE;
	if (Year==d1.Year && Month==d1.Month && Day>d1.Day) return TRUE;
	return FALSE;
}

BOOL TDate::operator ==(const TDate &d1)
{
	return (Year==d1.Year && Month==d1.Month && Day==d1.Day);
}

TDate& TDate::operator ++()
{
	DateForward(this,1);
	return *this;
}

TDate TDate::operator ++(int wint)
{
	TDate temp=*this;
	DateForward(this,1);
	return temp;
}

TDate& TDate::operator --()
{
	DateBefore(this,1);
	return *this;
}

TDate TDate::operator --(int wint)
{
	TDate temp=*this;
	DateBefore(this,1);
	return temp;
}

