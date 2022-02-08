#include "windows.h"
#include "Classes.h"
#include "../BarStruct.h"
#include "Shortcut.h"
#include "Objbase.h"
#include "MyUtils.h"
#include "../../Headers/general.h"

extern char	buf[1000],buf2[230];
extern HINSTANCE	hInstance;		  // Global instance handle for application

void ExtractIconPath(LPCTSTR s,BarUnit* uni)
{
byte b;
ZeroMemory(buf,1000);
byte i=0;
byte l=lstrlen(s);
while (s[i]!=',' && i<=l) 
{
	b=s[i];
	if (b>64 && b<91) b+=32;
	buf[i]=b;
    i++;
}
i++;
*uni->IconPath=(LPCTSTR)buf;
ZeroMemory(buf,1000);
byte k=0;
while (i<=l) buf[k++]=s[i++];
uni->IconIndex=atoi((LPCTSTR)&buf);
}

BOOL GetIconExt(LPCTSTR filepath,BarUnit* uni)
{
HKEY	hKey=0;
ExtractFileExt(filepath,(LPTSTR)&buf2);
if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CLASSES_ROOT,(LPCTSTR)&buf2,0,KEY_ALL_ACCESS,&hKey))
	return(FALSE);
char path[150];
ZeroMemory(path,150);
DWORD	len=150;
DWORD	type;
if (ERROR_SUCCESS!=RegQueryValueEx(hKey,"",NULL,&type,(LPBYTE)&path,&len)) 
{
	RegCloseKey(hKey);
	return FALSE;
}
RegCloseKey(hKey);
lstrcat(path,"\x5c");
lstrcat(path,"DefaultIcon");
if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CLASSES_ROOT,path,0,KEY_ALL_ACCESS,&hKey))
	return(FALSE);
ZeroMemory(path,150);
len=150;
if (ERROR_SUCCESS!=RegQueryValueEx(hKey,"",NULL,&type,(LPBYTE)&path,&len)) 
{
	RegCloseKey(hKey);
	return FALSE;
}
RegCloseKey(hKey);
int i=lstrlen(path)-1;
int count=0;
while (i>=0)
{
	if (path[i]==0x25) count++;
	i--;
}
if (path[0]==0x25 && count==1) 
{
	*uni->IconPath=filepath;
	uni->IconIndex=0;
} else
	ExtractIconPath(path,uni);
return TRUE;
}

BOOL GetIcon(LPTSTR gui,BarUnit* uni)
{

HKEY	hKey=0;
DWORD len,type;
char	path[150]="CLSID\x5c";
lstrcat(path,gui);
lstrcat(path,"\x5c");
lstrcat(path,"DefaultIcon");
if (ERROR_SUCCESS==RegOpenKeyEx(HKEY_CLASSES_ROOT,path,0,KEY_QUERY_VALUE,&hKey))
{
	ZeroMemory(path,150);

	len=150;
	RegQueryValueEx(hKey,"",NULL,&type,(LPBYTE)&path,&len);
	RegCloseKey(hKey);
	if (lstrlen(path)!=0) 
	{
		ExtractIconPath(path,uni);
		return (TRUE);
	}
}
	ZeroMemory(path,150);
	lstrcat(path,"CLSID\x5c");
	lstrcat(path,gui);
	lstrcat(path,"\x5cshell\x5cOpenHomePage\x5c\x43ommand");
	if (ERROR_SUCCESS!=RegOpenKeyEx(HKEY_CLASSES_ROOT,path,0,KEY_QUERY_VALUE,&hKey))
		return(FALSE);
	ZeroMemory(path,100);
	len=150;
	if (ERROR_SUCCESS!=RegQueryValueEx(hKey,"",NULL,&type,(LPBYTE)&path,&len)) 
	{
		RegCloseKey(hKey);
		return FALSE;
	}
	RegCloseKey(hKey);
	uni->IconIndex=0;
	LPCTSTR pt=(LPCTSTR)((DWORD)&path+1);
	len=lstrlen(pt);
	path[len]=0;
	*uni->IconPath=pt;
return TRUE;
}

BOOL GetIconFromLnk(BarUnit* uni)
{
	SHFILEINFO ShInfo1;
	long l=SHGetFileInfo(uni->FilePath->Text, FILE_ATTRIBUTE_NORMAL, &ShInfo1, sizeof(ShInfo1),
		SHGFI_ICONLOCATION);

CLSID cl;
HANDLE hFile;
if ((hFile=CreateFile(uni->FilePath->Text,GENERIC_READ,0,NULL,OPEN_EXISTING,
	 FILE_ATTRIBUTE_NORMAL,0))==INVALID_HANDLE_VALUE) return(FALSE);
SetFilePointer(hFile,80,NULL,FILE_BEGIN);
DWORD dwd;
if (!ReadFile(hFile,buf,2,&dwd,NULL)) return(FALSE);
if (buf[0]!=31) 
{
	    CloseHandle(hFile);
        return (FALSE);
}
ReadFile(hFile,&cl,sizeof(CLSID),&dwd,NULL);
ReadFile(hFile,buf,4,&dwd,NULL);
if (buf[0]==20) ReadFile(hFile,&cl,sizeof(CLSID),&dwd,NULL);
CloseHandle(hFile);
WCHAR * 	pb=(WCHAR*)&buf[0];
StringFromCLSID(cl,&pb);
char bb[50];
WideCharToMultiByte(CP_ACP,0,pb,-1,bb,700,NULL,NULL);
if (lstrlen(bb)==0) return FALSE;
return GetIcon(bb,uni);
}

void ReadUrl(BarUnit* uni)
{
HANDLE	hFile;
if ((hFile=CreateFile(uni->FilePath->Text,GENERIC_READ,0,NULL,OPEN_EXISTING,
	 FILE_ATTRIBUTE_NORMAL,0))==INVALID_HANDLE_VALUE) return;
ZeroMemory(buf,1000);
DWORD	dwd;
ReadFile(hFile,buf,1000,&dwd,NULL);
CloseHandle(hFile);
int i=FindInString((LPCTSTR)&buf,"iconfile=");
if (i==-1) return;
ZeroMemory(buf2,200);
byte k=0;
while (buf[i]!=13 && buf[i]!=0) buf2[k++]=buf[i++];
*uni->IconPath=(LPCTSTR)&buf2;
i=FindInString((LPCTSTR)&buf,"iconindex=");
if (i==-1) return;
ZeroMemory(buf2,200);
k=0;
while (buf[i]!=13 && buf[i]!=0) buf2[k++]=buf[i++];
uni->IconIndex=atoi((LPCTSTR)&buf2);
}

BOOL GetLnkData(LPLnk Lnk)
{
	Lnk->strIconLocation="";
	Lnk->nIconIndex=0;
	IShellLink* psl;
	HRESULT  hRes=CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*) &psl);
    if (!SUCCEEDED(hRes)) return (FALSE);
	IPersistFile* ppf;
	hRes=psl->QueryInterface(IID_IPersistFile, (LPVOID*) &ppf);
    if (!SUCCEEDED(hRes))
	{
		psl->Release();
		return (FALSE);
	}
	wchar_t  wsz[MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, Lnk->strLnkPath.Text, -1, wsz, MAX_PATH);

  hRes=ppf->Load(wsz,STGM_READ);
  if (!SUCCEEDED(hRes))
  {
	  ppf->Release();
	  psl->Release();
	  return (FALSE);
  }
  char buf[250];
//  psl->GetDescription(buf,250);
//  Lnk->strDescription=buf;
  psl->GetWorkingDirectory(buf,250);
  Lnk->strStartDir=buf;
  psl->GetIconLocation(buf,250,&(Lnk->nIconIndex));
  if (!strlen(buf))
  {
	  WIN32_FIND_DATA fd;
	  psl->GetPath(buf, 250, &fd, SLGP_SHORTPATH);

  }
  Lnk->strIconLocation=buf;
  WIN32_FIND_DATA ww;
  psl->GetPath(buf,250,&ww,SLGP_UNCPRIORITY);
  Lnk->strPath=buf;
  ppf->Release();
  psl->Release();
  return (TRUE);
}

BOOL SaveLnkData(LPLnk ll)
{
    IShellLink* psl;
    // Create shell link instance
    HRESULT hres = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLink, (LPVOID*) &psl);
    if (SUCCEEDED(hres))
    {
        // Get a pointer to persist file interface
        IPersistFile* ppf;
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*) &ppf);
        if (SUCCEEDED(hres))
        {
            // Convert to ANSI
			wchar_t  wsz[MAX_PATH];
            ::MultiByteToWideChar(CP_ACP, 0, ll->strLnkPath.Text, -1, wsz, 
                MAX_PATH);

            // Set attributes of link
            psl->SetPath(ll->strPath.Text);
            psl->SetWorkingDirectory(ll->strStartDir.Text);
            psl->SetIconLocation(ll->strIconLocation.Text, ll->nIconIndex);
            psl->SetDescription(ll->strDescription.Text);
            psl->SetArguments("");
            psl->SetHotkey(0);
            psl->SetShowCmd(SW_SHOW);

            // Save the updated link
            hres = ppf->Save(wsz, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
return TRUE;
}

