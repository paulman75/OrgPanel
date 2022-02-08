#include "Windows.h"
#define _MYDLL_
#include "regunit.h"
#include "../Headers/general.h"

BOOL __stdcall ReadRegistry(LPCTSTR Cap, LPBYTE poi, DWORD len)
{
	HKEY hKey=0;
	if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RegistryPath,0,KEY_ALL_ACCESS,&hKey)) return FALSE;
	DWORD	alen;
	*poi=0;
	if (ERROR_SUCCESS!=RegQueryValueEx(hKey,Cap,NULL,NULL,NULL,&alen)) return FALSE;
	if (alen>len) alen=len;
	BOOL res=(ERROR_SUCCESS==RegQueryValueEx(hKey,Cap,NULL,NULL,poi,&alen));
	RegCloseKey(hKey);
	return res;
}

void __stdcall WriteDWordToReg(LPCTSTR Cap, DWORD Val)
{
	HKEY hKey=0;
	if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RegistryPath,0,KEY_ALL_ACCESS,&hKey)) return;
	RegSetValueEx(hKey,Cap,0,REG_DWORD,(LPBYTE)&Val,4);
	RegCloseKey(hKey);
}

void __stdcall WriteStringToReg(LPCTSTR Cap, LPCTSTR Text)
{
	HKEY hKey=0;
	if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RegistryPath,0,KEY_ALL_ACCESS,&hKey)) return;
	RegSetValueEx(hKey,Cap,0,REG_SZ,(LPBYTE)Text,lstrlen(Text)+1);
	RegCloseKey(hKey);
}

void __stdcall WriteBinToReg(LPCTSTR Cap, LPBYTE Val, int size)
{
	HKEY hKey=0;
	if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RegistryPath,0,KEY_ALL_ACCESS,&hKey)) return;
	RegSetValueEx(hKey,Cap,0,REG_BINARY,(LPBYTE)Val,size);
	RegCloseKey(hKey);
}

void CheckCodePage()
{
	DWORD dw=GetVersion();
	if (dw>0x80000000) return;
	dw=(DWORD)(LOBYTE(LOWORD(dw)));
	if (dw==4)
	{
		HKEY hKey=0;
		if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage",0,KEY_ALL_ACCESS,&hKey)) return;
		DWORD len=50;
		LPTSTR st=(LPTSTR)malloc(50);
		st[0]=0;
		BOOL res=(ERROR_SUCCESS==RegQueryValueEx(hKey,"1252",NULL,NULL,(LPBYTE)st,&len));
		if (res && st[5]=='2')
		{
			st[5]='1';
			RegSetValueEx(hKey,"1252",0,REG_SZ,(LPBYTE)st,lstrlen(st)+1);
			MessageBox(0,"Если в диалогах русские символы будут отображаться не правильно - перезагрузите Window","Предупреждение",0);
		}
		RegCloseKey(hKey);
	}
}
