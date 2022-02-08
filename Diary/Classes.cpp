#include "windows.h"
#include "Classes.h"

CMyString::CMyString()
{
Text=NULL;
}

CMyString::CMyString(LPCTSTR lpsz)
{
	Text=(LPTSTR)malloc(lstrlen(lpsz)+1);
	lstrcpy(Text,lpsz);
}

void CMyString::Empty()
{
	if (Text)		free(Text);
	Text=NULL;
}

CMyString::~CMyString()
{
	Empty();
}

void CMyString::AllocMore(WORD PlusNeedLen)
{
	LPTSTR Temp;
	Temp=(LPTSTR)malloc(lstrlen(Text)+PlusNeedLen+1);
	lstrcpy(Temp,Text);
	free(Text);
	Text=Temp;
}

const CMyString& CMyString::operator=(LPCTSTR lpsz)
{
	Empty();
	Text=(LPTSTR)malloc(lstrlen(lpsz)+1);
	lstrcpy(Text,lpsz);
	return *this;
}

const CMyString& CMyString::operator=(const CMyString& string)
{
	Empty();
	Text=(LPTSTR)malloc(lstrlen(string.Text)+1);
	lstrcpy(Text,string.Text);
	return *this;
}

const CMyString& CMyString::operator+=(const CMyString& string)
{
	AllocMore(lstrlen(string.Text));
	lstrcat(Text,string.Text);
	return *this;
}

const CMyString& CMyString::operator+=(LPCTSTR lpsz)
{
	AllocMore(lstrlen(lpsz));
	lstrcat(Text,lpsz);
	return *this;
}

BOOL operator==(const CMyString& string, LPCTSTR lpsz)
{
	return (string.Text==NULL || lstrcmp(string.Text,lpsz)==0);
}

