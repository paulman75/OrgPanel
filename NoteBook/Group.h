#ifndef __GROUP_H
#define __GROUP_H

typedef struct TGROUPUNIT
{
	DWORD		Key;
	LPTSTR		Name;
	TGROUPUNIT*	Next;
} TGROUPUNIT, *LPGROUPUNIT;

void LoadGroups(HANDLE fil);
void SaveGroups(HANDLE fil);
char* GetGroupName(DWORD Index);
void SelectGroup(DWORD* pIndex, HWND hOwner);
void LoadGroupToCombo(HWND hCombo, DWORD* Sel);

#endif