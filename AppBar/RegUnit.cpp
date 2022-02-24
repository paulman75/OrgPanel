#include "windows.h"
#include "..\Headers\general.h"
#include "Compon\MyUtils.h"
#include "..\Headers\const.h"
#include "RegUnit.h"

#define RunPath	 "Software\\Microsoft\\Windows\\CurrentVersion\\Run"

extern char MainPathFile[200];
extern char	buf2[230];
extern char MainDir[200];
char	IniFile[430];
HKEY hKey;

void StringFromReg(LPTSTR st,LPTSTR Cap, WORD Size)
{
DWORD	len;
st[0]=0;
if (ERROR_SUCCESS!=RegQueryValueEx(hKey,Cap,NULL,NULL,NULL,&len)) return;
if (len>Size) len=Size;
RegQueryValueEx(hKey,Cap,NULL,NULL,(LPBYTE)st,&len);
}

void StringFromIni(LPTSTR st,LPTSTR Cap, WORD Size)
{
	GetPrivateProfileString("Main",Cap,"",st,Size,IniFile);
}

BOOL ReadFromReg(void* v,LPTSTR Cap,byte Size=4)
{
	DWORD	len=Size;
	if (ERROR_SUCCESS!=RegQueryValueEx(hKey,Cap,NULL,NULL,NULL,&len)) 
	{
		DWORD err=GetLastError();
		return FALSE;
	}
	if (len>Size) len=Size;
	RegQueryValueEx(hKey,Cap,NULL,NULL,(LPBYTE)v,&len);
	return TRUE;
}

void ReadByte(void* v,LPTSTR Cap)
{
	DWORD	dw=0;
	if (!ReadFromReg(&dw,Cap,4)) return;
	byte* b=(byte*)v;
	*b=(byte)dw;
}

void ReadByteFromIni(void* v,LPTSTR Cap)
{
	GetPrivateProfileString("Main",Cap,"",&buf2[0],250,IniFile);
	byte* b=(byte*)v;
	*b=atoi(buf2);
}

void ReadWordFromIni(void* v, LPTSTR Cap)
{
	GetPrivateProfileString("Main", Cap, "", &buf2[0], 250, IniFile);
	WORD* b = (WORD*)v;
	*b = atoi(buf2);
}

void BarConfigFromRegistry(PBarConfig bc)
{
bc->AutoHide=FALSE;
bc->FName[0]=0;
bc->SName[0]=0;
bc->Birth.Day=1;
bc->Birth.Month=0;
bc->Birth.Year=1998;
bc->Edge=ABE_LEFT;
bc->UsedPassword=FALSE;
bc->Password[0]=0;
bc->RestorePassword=3;
bc->DeleteDelo=1;
bc->DesktopX=90;
bc->DeskTopY=90;
bc->LargeIcons=TRUE;
bc->PulseDial=TRUE;
bc->BackGr=0;
bc->BackGr2=0;
bc->Prefix[0]=0;
bc->AutoOpen=TRUE;
bc->OpenDelay=0;
bc->Speed=10;
bc->SyncTime=0;
bc->LastSync=0;

bc->ShowFullName=TRUE;
bc->Sounds=TRUE;
bc->bCuckoo=TRUE;

bc->ShowSkyInfo=TRUE;
bc->FullImen=FALSE;
bc->DeleteEvent=1;

lstrcpy(bc->HourFileName,MainDir);
lstrcat(bc->HourFileName,"Wavs\\");
lstrcpy(bc->DoFileName,bc->HourFileName);
lstrcpy(bc->RovFileName,bc->HourFileName);
lstrcpy(bc->EventFileName,bc->HourFileName);
lstrcat(bc->HourFileName,"hour.wav");
lstrcpy(bc->HalfHourFileName,bc->HourFileName);
lstrcat(bc->DoFileName,"delo.wav");
lstrcat(bc->EventFileName,"delo.wav");
lstrcat(bc->RovFileName,"delo.wav");

for (int i=0; i<DNEV_COUNT; i++) bc->Dnev[i]=i+100;

lstrcpy(IniFile,MainDir);
lstrcat(IniFile,"Data\\params.ini");

BOOL bIniOk=FileExists(IniFile);

if (bIniOk)
{
	ReadByteFromIni(&bc->Edge,"PanelState");
	ReadWordFromIni(&bc->DesktopX,"DeskTopX");
	ReadWordFromIni(&bc->DeskTopY,"DeskTopY");
	ReadByteFromIni(&bc->AutoHide,"AutoHide");
	ReadByteFromIni(&bc->BackGr,"BackGnd");
	ReadByteFromIni(&bc->BackGr2,"BackGnd2");
	ReadByteFromIni(&bc->Birth.Day,"BirthDay");
	ReadByteFromIni(&bc->Birth.Month,"BirthMonth");
	ReadWordFromIni(&bc->Birth.Year,"BirthYear");

	ReadByteFromIni(&bc->UsedPassword,"UsedPassword");
	ReadByteFromIni(&bc->RestorePassword,"RestorePassword");
	ReadByteFromIni(&bc->LargeIcons,"LargeIcons");
	ReadByteFromIni(&bc->AutoOpen,"AutoOpen");
	ReadByteFromIni(&bc->AlwaysOnTop, "AlwaysOnTop");
	ReadByteFromIni(&bc->TitShow,"TitShow");
	ReadByteFromIni(&bc->Speed,"Speed");
	ReadByteFromIni(&bc->Sounds,"Sounds");
	ReadByteFromIni(&bc->bCuckoo,"Cuckoo");
	ReadByteFromIni(&bc->SyncTime,"TimeSync");
//	ReadByteFromIni(&bc->LastSync,"LastTimeBin",sizeof(bc->LastSync));
	StringFromIni(bc->DoFileName,"DoWav",200);
	StringFromIni(bc->EventFileName,"EventWav",200);
	StringFromIni(bc->RovFileName,"RovWav",200);
	StringFromIni(bc->HalfHourFileName,"HalfWav",200);
	StringFromIni(bc->HourFileName,"HourWav",200);
	ReadByteFromIni(&bc->PulseDial,"PulseDial");
	ReadByteFromIni(&bc->DeleteDelo,"DeleteDelo");
	ReadByteFromIni(&bc->DeleteEvent,"DeleteEvent");
	ReadFromReg(&bc->Dnev,"Dnev",DNEV_COUNT);
	ReadByteFromIni(&bc->OpenDelay,"Delay");
	StringFromIni(bc->FName,"FName",15);
	StringFromIni(bc->SName,"SName",15);
	StringFromIni(bc->Prefix,"Prefix",15);
	StringFromIni(bc->Password,"Password",15);
	ReadByteFromIni(&bc->ShowFullName,"ShowFullName");
	ReadByteFromIni(&bc->ShowSkyInfo,"ShowSkyInfo");
	ReadByteFromIni(&bc->LastFolderIndex,"LastFolderIndex");
	StringFromIni(bc->GeoD,"GeoD",20);
	if (!lstrlen(bc->GeoD)) strcpy_s(bc->GeoD,20, "37.37");
	StringFromIni(bc->GeoS,"GeoS",20);
	if (!lstrlen(bc->GeoS)) strcpy_s(bc->GeoS,20, "55.45");

	ReadByteFromIni(&bc->FullImen,"FullImen");
	int i=0;
	while (bc->Password[i]>0)
	{
		bc->Password[i]=bc->Password[i]-20;
		i++;
	}
}

hKey=0;
if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RegistryPath,0,KEY_ALL_ACCESS,&hKey))
return;

ReadFromReg(&bc->LastSync,"LastTimeBin",sizeof(bc->LastSync));

if (!bIniOk)
{
	ReadFromReg(&bc->LastFolderIndex,"LastFolderIndex");
	ReadByte(&bc->Edge,"PanelState");
	ReadFromReg(&bc->DesktopX,"DeskTopX");
	ReadFromReg(&bc->DeskTopY,"DeskTopY");
	ReadByte(&bc->AutoHide,"AutoHide");
	ReadByte(&bc->AlwaysOnTop, "AlwaysOnTop");
	ReadFromReg(&bc->BackGr,"BackGnd");
	ReadFromReg(&bc->BackGr2,"BackGnd2");
	ReadFromReg(&bc->Birth,"BDate",sizeof(TDate));
	ReadByte(&bc->UsedPassword,"UsedPassword");
	ReadByte(&bc->RestorePassword,"RestorePassword");
	ReadByte(&bc->LargeIcons,"LargeIcons");
	ReadByte(&bc->AutoOpen,"AutoOpen");
	ReadByte(&bc->TitShow,"TitShow");
	ReadByte(&bc->Speed,"Speed");
	ReadByte(&bc->Sounds,"Sounds");
	ReadByte(&bc->bCuckoo,"Cuckoo");
	ReadByte(&bc->SyncTime,"TimeSync");
	StringFromReg(bc->DoFileName,"DoWav",200);
	StringFromReg(bc->EventFileName,"EventWav",200);
	StringFromReg(bc->RovFileName,"RovWav",200);
	StringFromReg(bc->HalfHourFileName,"HalfWav",200);
	StringFromReg(bc->HourFileName,"HourWav",200);
	ReadByte(&bc->PulseDial,"PulseDial");
	ReadByte(&bc->DeleteDelo,"DeleteDelo");
	ReadByte(&bc->DeleteEvent,"DeleteEvent");
	ReadFromReg(&bc->Dnev,"Dnev",DNEV_COUNT);
	ReadFromReg(&bc->OpenDelay,"Delay",4);
	StringFromReg(bc->FName,"FName",15);
	StringFromReg(bc->SName,"SName",15);
	StringFromReg(bc->Prefix,"Prefix",15);
	StringFromReg(bc->Password,"Password",15);
	ReadByte(&bc->ShowFullName,"ShowFullName");
	ReadByte(&bc->ShowSkyInfo,"ShowSkyInfo");
	StringFromReg(bc->GeoD,"GeoD",20);
	if (!lstrlen(bc->GeoD)) strcpy_s(bc->GeoD,20,"37.37");
	StringFromReg(bc->GeoS,"GeoS",20);
	if (!lstrlen(bc->GeoS)) strcpy_s(bc->GeoS,20,"55.45");

	ReadByte(&bc->FullImen,"FullImen");
	int i=0;
	while (bc->Password[i]>0)
	{
		bc->Password[i]=bc->Password[i]+20;
		i++;
	}
}
RegCloseKey(hKey);
}

void GetDesktop(LPTSTR path)
{
hKey=0;
if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,"Software\x5CMicrosoft\x5CWindows\x5C\x43urrentVersion\x5C\x45xplorer\x5cShell Folders",0,KEY_ALL_ACCESS,&hKey))
return;
StringFromReg(path,"Desktop",200);
RegCloseKey(hKey);
}

BOOL ReadWordReg(LPTSTR sName, WORD* bValue)
{
hKey=0;
if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RunPath,0,KEY_ALL_ACCESS,&hKey))
return (FALSE);
DWORD dw;
ReadFromReg(&dw,sName,10);
*bValue=(WORD)dw;
RegCloseKey(hKey);
return TRUE;
}

BOOL IsStartUp()
{
hKey=0;
if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RunPath,0,KEY_ALL_ACCESS,&hKey))
return (FALSE);
StringFromReg(buf2,"OrgPanel",200);
RegCloseKey(hKey);
return (lstrlen(buf2)>0);
}

BOOL IsDiaryStartUp()
{
hKey=0;
if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CURRENT_USER,RunPath,0,KEY_ALL_ACCESS,&hKey))
return (FALSE);
StringFromReg(buf2,"OrgPanel_Diary",200);
RegCloseKey(hKey);
return (lstrlen(buf2)>0);
}

void WriteString(LPCTSTR Cap,  LPCTSTR Text)
{
	RegSetValueEx(hKey,Cap,0,REG_SZ,(LPBYTE)Text,lstrlen(Text)+1);
}

void WriteString2Ini(LPCTSTR Cap,  LPCTSTR Text)
{
	WritePrivateProfileString("Main",Cap,Text,IniFile);
}

void SetStartUp(BOOL cc)
{
if (!cc) return;
hKey=0;
DWORD dw;
if (ERROR_SUCCESS!=RegCreateKeyEx(HKEY_CURRENT_USER,RunPath,0,REG_NONE,
	REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,&hKey,&dw)) return;
if (cc) WriteString("OrgPanel",MainPathFile);
else RegDeleteValue(hKey,"OrgPanel");
RegCloseKey(hKey);
}

void SetDiaryStartUp(BOOL cc)
{
	if (!cc && !IsDiaryStartUp()) return;
	hKey=0;
	DWORD dw;
	if (ERROR_SUCCESS!=RegCreateKeyEx(HKEY_CURRENT_USER,RunPath,0,REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,&hKey,&dw)) return;
	if (cc)
	{
		char *temp=(char*)malloc(300);
		lstrcpy(temp,MainDir);
		lstrcat(temp,"Diary.exe /r");
		WriteString("OrgPanel_Diary",temp);
		free(temp);
	}
	else RegDeleteValue(hKey,"OrgPanel_Diary");
	RegCloseKey(hKey);
}


void WriteWord(LPCTSTR Cap, DWORD Val)
{
	RegSetValueEx(hKey,Cap,0,REG_DWORD,(LPBYTE)&Val,4);
}

void WriteWord2Ini(LPCTSTR Cap, DWORD Val)
{
	wsprintf(buf2,"%d",Val);
	WritePrivateProfileString("Main",Cap,buf2,IniFile);
}

void WriteWordReg(LPTSTR sName, WORD wValue)
{
	hKey=0;
	DWORD	dwd;
	if (ERROR_SUCCESS!=RegCreateKeyEx(HKEY_CURRENT_USER,RegistryPath,0,"",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwd))
	return;
	WriteWord(sName,wValue);
	RegCloseKey(hKey);
}

void SaveOptions(PBarConfig bc)
{
/*hKey=0;
DWORD	dwd;
if (ERROR_SUCCESS!=RegCreateKeyEx(HKEY_CURRENT_USER,RegistryPath,0,"",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwd))
	return;
*/
lstrcpy(IniFile,MainDir);
lstrcat(IniFile,"Data\\params.ini");
WriteWord2Ini("PanelState",bc->Edge);
WriteWord2Ini("AutoHide",bc->AutoHide);
WriteWord2Ini("AlwaysOnTop", bc->AlwaysOnTop);
WriteString2Ini("FName",bc->FName);
WriteString2Ini("SName",bc->SName);
WriteString2Ini("Prefix",bc->Prefix);
WriteWord2Ini("BackGnd",bc->BackGr);
WriteWord2Ini("BackGnd2",bc->BackGr2);
WriteWord2Ini("DesktopX",bc->DesktopX);
WriteWord2Ini("DesktopY",bc->DeskTopY);
WriteWord2Ini("AutoOpen",bc->AutoOpen);
WriteWord2Ini("TitShow",bc->TitShow);
WriteWord2Ini("Delay",bc->OpenDelay);
WriteWord2Ini("Speed",bc->Speed);
WriteWord2Ini("Sounds",bc->Sounds);
WriteWord2Ini("UsedPassword",bc->UsedPassword);
WriteWord2Ini("RestorePassword",bc->RestorePassword);
WriteWord2Ini("LargeIcons",bc->LargeIcons);
WriteWord2Ini("Cuckoo",bc->bCuckoo);
WriteWord2Ini("TimeSync",bc->SyncTime);
WriteString2Ini("DoWav",bc->DoFileName);
WriteString2Ini("EventWav",bc->EventFileName);
WriteString2Ini("RovWav",bc->RovFileName);
WriteString2Ini("HalfWav",bc->HalfHourFileName);
WriteString2Ini("HourWav",bc->HourFileName);
WriteWord2Ini("PulseDial",bc->PulseDial);
lstrcpy(buf2,bc->Password);
int i=0;
while (buf2[i]!=0)   buf2[i++]+=20;
WriteString2Ini("Password",buf2);
i=0;
while (i<DNEV_COUNT)
{
	buf2[i]=bc->Dnev[i];
	i++;
}
buf2[i]=0;
WriteString2Ini("Dnev",buf2);
//RegSetValueEx(hKey,"Dnev",0,REG_BINARY,(LPBYTE)&bc->Dnev[0],DNEV_COUNT);
//RegSetValueEx(hKey,"BDate",0,REG_BINARY,(LPBYTE)&bc->Birth,sizeof(TDate));
WriteWord2Ini("BirthDay",bc->Birth.Day );
WriteWord2Ini("BirthMonth",bc->Birth.Month );
WriteWord2Ini("BirthYear",bc->Birth.Year );

WriteWord2Ini("DeleteDelo",bc->DeleteDelo);
WriteWord2Ini("DeleteEvent",bc->DeleteEvent);
WriteWord2Ini("ShowFullName",bc->ShowFullName);
WriteWord2Ini("ShowSkyInfo",bc->ShowSkyInfo);
WriteString2Ini("GeoD",bc->GeoD);
WriteString2Ini("GeoS",bc->GeoS);
WriteWord2Ini("FullImen",bc->FullImen);
//RegCloseKey(hKey);
}

void BadRegLnk(HWND hOwn)
{
if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CLASSES_ROOT,"LNKFILE",0,KEY_ALL_ACCESS,&hKey))
	return;
	DWORD w=1;
	char ss[5];
	DWORD len=4;
	if (ERROR_SUCCESS!=RegQueryValueEx(hKey,"IsShortcut",NULL,NULL,(LPBYTE)&ss[0],&len))
		w=8;

	if (ss[0]=='0')
	{
		int i=MessageBox(hOwn,"В системе отключено отображение стрелок у ярлыков. Ярлыки в Оргпанель работать не будут. Изменить настройки(потребуется перезагрузка)?","Ошибка",MB_ICONWARNING || MB_YESNO);
		if (i==IDOK)
		{
			w=1;
			RegSetValueEx(hKey,"IsShortcut",0,REG_DWORD,(LPBYTE)&w,4);
			RegCloseKey(hKey);
			RegOpenKeyEx(HKEY_CLASSES_ROOT,"PIFFILE",0,KEY_ALL_ACCESS,&hKey);
			RegSetValueEx(hKey,"IsShortcut",0,REG_DWORD,(LPBYTE)&w,4);
			MessageBox(hOwn,"Настройки изменены. Перезагрузите систему!","Настройки",MB_ICONWARNING);
		}
	}
RegCloseKey(hKey);
}