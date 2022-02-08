#ifndef __UNITS_H
#define __UNITS_H

#define UN_NAME		0
#define UN_MNAME	1
#define UN_SNAME	2
#define UN_HOMETEL	3
#define UN_WORKTEL	4
#define UN_FAX		5
#define UN_HOMEPAGE	6
#define UN_EMAIL	7
#define UN_ICQ		8
#define UN_ADDRESS	9
#define UN_CITY		10
#define UN_OBLAST	11
#define UN_COUNTRY	12
#define UN_ZIPCODE	13
#define UN_SOTOV	14
#define UN_PAGER	15
#define UN_DOLV		16
#define UN_MISC		17
#define UN_WIFE		18
#define UN_CHILDREN	19
#define UN_BDATE	20
#define UN_FIRM		21
#define UN_SEX		22
#define UN_GROUP	23

#define UN_COUNT	24

#define UNF_NAME	0
#define UNF_HOMEPAGE	1
#define UNF_EMAIL	2
#define UNF_TEL1	3
#define UNF_TEL2	4
#define UNF_ADDRESS	5
#define UNF_CITY	6
#define UNF_OBLAST	7
#define UNF_ZIPCODE	8
#define UNF_COUNTRY	9
#define UNF_FAX		10
#define UNF_OPIS	11
#define UNF_REKV	12
#define UNF_MISC	13

#define UNF_COUNT	14

typedef struct TNOTEUNIT
{
	DWORD		Key;
	LPTSTR		Fields[UN_COUNT-3];
	TDate		BDate;
	DWORD		Firm;
	byte		Sex;
	DWORD		Group;
	TNOTEUNIT*	Next;
} TNOTEUNIT, *LPNOTEUNIT;

typedef struct TFIRMUNIT
{
	DWORD		Key;
	LPTSTR		Fields[UNF_COUNT];
	TFIRMUNIT*	Next;
} TFIRMUNIT, *LPFIRMUNIT;

#include <commctrl.h>

void UpdateAllData();
void LoadUnits();
void SaveUnits();
void EnableActionMenu(HMENU hSub);
void AddHuman();
void AddFirm();
void SetActiveUnit(NM_LISTVIEW* nm);
void ChangeNote();
void DeleteNote();
int MainCompareFunc(LPARAM lParam1, LPARAM lParam2, BOOL bsd, int isc);
void MakeUnitName(LPNOTEUNIT nu,LPTSTR s);
void FillBioCombo(HWND hwnd);
int CompareStrings(LPTSTR s,LPTSTR s2);
void SetItemSelect(LPARAM nu);
void DeleteUnits();
void UpdateToolBar();
LPTSTR ReadField(HANDLE hFile);
void WriteField(HANDLE hFile, LPCTSTR s);

#endif