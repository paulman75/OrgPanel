#ifndef __UNITS_H
#define __UNITS_H

#include <commctrl.h>

#define IDC_RICHEDIT	6

typedef struct
{
	int		ID;
	int		Group_ID;
	char	Name[50];
	int		LoadIndex;
	LPTSTR		St;
} TGROUP, *LPGROUP;

void LoadUnits();
void SaveUnits();
void ShowInfo(HTREEITEM hit);
void SaveInfo();
LPTSTR ReadString(HANDLE hFile);

void RenameUnit();
void DeleteUnit();
void Convert(DWORD operation, char* s);
void Find(DWORD operation);
void AddGroup();
void FillTree();
void OnDropped(int ID1, int ID2);
BOOL CanDelete();
void UpdateToolBar();

#endif