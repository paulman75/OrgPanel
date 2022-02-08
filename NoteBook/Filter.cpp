#include "windows.h"
#include "../Headers/general.h"
#include "Filter.h"

extern HWND	hFilterBox;
extern HWND	hFilterEdit[4];
extern	BOOL	bHumanMode;

char Filters[2][3][30];
DWORD FilterGroup=-1;

BOOL CheckUnitFilter(LPNOTEUNIT nu)
{
	byte nIndex[3]={UN_NAME,UN_SNAME,UN_EMAIL};
	for (int i=0; i<3; i++)
	{
		if (lstrlen(Filters[1][i])>0)
		{
			if (!nu->Fields[nIndex[i]]) return FALSE;
			if (FindInString(nu->Fields[nIndex[i]],Filters[1][i])==-1) return FALSE;
		}
	}
	if (FilterGroup!=-1 && FilterGroup!=nu->Group) return FALSE;
	return TRUE;
}

BOOL CheckFirmFilter(LPFIRMUNIT nf)
{
	byte nIndex[3]={UNF_NAME,UNF_OPIS,UNF_EMAIL};
	for (int i=0; i<3; i++)
	{
		if (lstrlen(Filters[0][i])>0)
		{
			if (!nf->Fields[nIndex[i]]) return FALSE;
			if (FindInString(nf->Fields[nIndex[i]],Filters[0][i])==-1) return FALSE;
		}
	}
	return TRUE;
}

void OnFilterComboSelect()
{
	int i=SendMessage(hFilterEdit[3], CB_GETCURSEL,0,0);
	FilterGroup=SendMessage(hFilterEdit[3], CB_GETITEMDATA, i, 0);
	UpdateAllData();
}
