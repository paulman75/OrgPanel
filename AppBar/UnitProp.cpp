#include "Windows.h"
#include "BarStruct.h"
#include "UnitProp.h"
#include "shlobj.h"
#include "Compon\MyUtils.h"
#include "../Headers/general.h"

extern HWND	BarhWnd;
extern BarUnit*	hMenuUnit;

BOOL	HandleContextMenu(HWND Parent, IContextMenu* CtxMenu, ULONG Attr)
{
	BOOL Res=FALSE;
    CMINVOKECOMMANDINFO	ICI;
	ZeroMemory(&ICI, sizeof(CMINVOKECOMMANDINFO));
    ICI.cbSize=sizeof(CMINVOKECOMMANDINFO);
    ICI.hwnd=Parent;
    ICI.nShow=SW_SHOWNORMAL;
    // does it have a property sheet?
    if ((Attr && SFGAO_HASPROPSHEET)!=0)
	{
		ICI.lpVerb="properties"; // Built-in verb for all items, I think
		Res=SUCCEEDED(CtxMenu->InvokeCommand(&ICI));
	}
	return Res;
}

BOOL HandleFromPIDLs(HWND Parent, IShellFolder* SubFolder,
					 LPITEMIDLIST ItemID, ULONG Attr)
{
    BOOL Res= FALSE;
    IContextMenu*	ContextMenu;
	if (SUCCEEDED(SubFolder->GetUIObjectOf(Parent, 1, (LPCITEMIDLIST*)&ItemID,
       IID_IContextMenu, NULL, (LPVOID*)&ContextMenu))) 
    {
		IContextMenu2*	ContextMenu2;
      if (SUCCEEDED(ContextMenu->QueryInterface(IID_IContextMenu2,
         (LPVOID*)&ContextMenu2))) 
	  {
        ContextMenu->Release(); // Delphi 3 does this for you.
		ContextMenu=ContextMenu2;
		IContextMenu3*	ContextMenu3;
        if (SUCCEEDED(ContextMenu->QueryInterface(IID_IContextMenu3,
           (LPVOID*)&ContextMenu3)))
		{
          ContextMenu->Release(); // Delphi 3 does this for you.
          ContextMenu = ContextMenu3;
        } //else MessageBox(BarhWnd,"12","12",MB_OK);
      }
//	  else MessageBox(BarhWnd,"13","13",MB_OK);
      Res = HandleContextMenu(Parent, ContextMenu, Attr);
      ContextMenu->Release(); // Delphi 3 does this for you.
	}
//	else 		MessageBox(BarhWnd,"11","11",MB_OK);
return Res;
}

BOOL Go(LPCTSTR FileName,HWND Parent)
{
 	  BOOL Res=FALSE;
	  IMalloc*	ShellMalloc;
      SHGetMalloc(&ShellMalloc);
      // I'm extra liberal with my try-finally blocks when dealing with system
      // resources like these.  Last thing I want to do is make the shell itself
      // unstable.
		char JustName[200];
        ExtractFileName(FileName,(LPTSTR)&JustName);
		wchar_t  oleSubDir[200];
		byte wsz[200];
		ExtractFilePath(FileName,(LPTSTR)&wsz);
		MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)&wsz, -1, oleSubDir, 200);

		wchar_t  oleFileName[200];
		MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)&JustName, -1, oleFileName, 200);
        IShellFolder*	ShellFolder;
		if (SUCCEEDED(SHGetDesktopFolder(&ShellFolder))) 
		{
			ULONG	Eaten,Attr;
			ITEMIDLIST*	FolderID;
			if (SUCCEEDED(ShellFolder->ParseDisplayName(Parent, NULL,
                 (LPOLESTR)oleSubDir, &Eaten, &FolderID, &Attr))) 
			{
				IShellFolder*	SubFolder;
                if (SUCCEEDED(ShellFolder->BindToObject(FolderID,NULL,
                   IID_IShellFolder, (LPVOID*)&SubFolder)))
				{
					LPITEMIDLIST	ItemID;
                  if (SUCCEEDED(SubFolder->ParseDisplayName(Parent, NULL,
                     (LPOLESTR)oleFileName, &Eaten, &ItemID, &Attr)))
				  {
					  Attr=Attr||SFGAO_HASPROPSHEET;
					  Res=HandleFromPIDLs(Parent, SubFolder, ItemID, Attr);
                      ShellMalloc->Free(ItemID);
				  }
/*				  else {
					  int r=GetLastError();
					  char buf[200];
					  wsprintf(buf,"%d",r);
					  MessageBox(BarhWnd,&buf[0],"4",MB_OK);
//					  MessageDlg(0,"Parse %d",r,"4");
				  }*/
				  ShellMalloc->Free(FolderID);
				}
//				else MessageBox(BarhWnd,"3","3",MB_OK);
                ShellFolder->Release(); // Delphi 3 does this for you.
			}
//			else MessageBox(BarhWnd,"2","2",MB_OK);
		}
//		else MessageBox(BarhWnd,"1","1",MB_OK);
        ShellMalloc->Release();
return Res;
}

void ProcessMessages()
{
     MSG        msgMain;
GetMessage((LPMSG) &msgMain, NULL, 0, 0);
TranslateMessage((LPMSG) &msgMain);
DispatchMessage((LPMSG) &msgMain);
}

BOOL bFinded;

BOOL __stdcall EnumWindowProc(HWND hWnd,LPARAM lParam)
{
	char className[15];
	GetClassName(hWnd,(LPTSTR)&className,15);
	if (lstrcmp((LPCTSTR)&className,"StubWindow32")==0) 
	{
		DWORD prId;
		GetWindowThreadProcessId(hWnd,&prId);
		if (prId==(DWORD)lParam)
		{
			bFinded=TRUE;
			return FALSE;
		}
	}
	return TRUE;
}

BOOL FindProp()
{
	DWORD prIdMy;
	bFinded=FALSE;
	GetWindowThreadProcessId(BarhWnd,&prIdMy);
	EnumWindows(&EnumWindowProc,(LPARAM)prIdMy);
	return bFinded;
}

void __stdcall UnitPropTimer(HWND hWnd,UINT msg, UINT uID, DWORD dwTime)
{
	if (FindProp()) return;
	KillTimer(NULL,uID);
	BitmapToUnitFromIcon(hMenuUnit);
	PlaceIcon();
	SaveMainBitmap();
}

void UnitProp(BarUnit* uni)
{
	Go(uni->FilePath->Text, BarhWnd);
	SetTimer(NULL,0,1000,&UnitPropTimer);
}
