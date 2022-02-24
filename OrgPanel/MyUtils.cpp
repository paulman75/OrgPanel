#include "windows.h"
#define _MYDLL_
#include "../Headers/general.h"
#include "regunit.h"
#include <shlobj.h>
#include "../Headers/MemBit.h"
#include "RichEdit.h"

extern LPVOID	lpBeepWave;
extern HINSTANCE hInstance;

void __stdcall DrawBitmap(HDC hdc, HBITMAP hBit, LONG x, LONG y)
{
	HDC hdc2=CreateCompatibleDC(hdc);
	SelectObject(hdc2,hBit);
	BITMAP bi;
	GetObject(hBit,sizeof(bi),&bi);
	RECT rc;
	rc.left=x;
	rc.top=y;
	int w=bi.bmWidth/2;
	rc.right=x+w;
	rc.bottom=y+bi.bmHeight;
	FillRect(hdc,&rc,(HBRUSH)COLOR_WINDOW);
	BitBlt(hdc,x,y,w,bi.bmHeight,hdc2,w,0,SRCPAINT);
	BitBlt(hdc,x,y,w,bi.bmHeight,hdc2,0,0,SRCAND);
	DeleteDC(hdc2);
}

int __stdcall StringCompare(LPCTSTR s1, LPCTSTR s2)
{
// 0-идентичны,
// 1-s1 входит в s2
// 2-s2 входит в s1
// 3-разные
	int i=0;
	while (s1[i] && s2[i])
	{
		byte b=s1[i];
		byte b2=s2[i];
		if ((b>64 && b<91) || (b>191 && b<224)) b+=32;
		if ((b2>64 && b2<91) || (b2>191 && b2<224)) b2+=32;
		if (b!=b2) return 3;
		i++;
	}
	if (!s1[i] && !s2[i]) return 0;
	if (!s1[i]) return 2;
	if (!s2[i]) return 1;
	return 0;
}

int __stdcall FindInString(LPCTSTR s, LPCTSTR s2, BOOL bCase)
{
	int l=lstrlen(s2);
	int i=0;
	int j=0;
	byte b;
	byte b2;
	while (s[i]!=0 && j<l) 
	{
		b=s[i++];
		b2=s2[j];
		if (!bCase)
		{
			if ((b>64 && b<91) || (b>191 && b<224)) b+=32;
			if ((b2>64 && b2<91) || (b2>191 && b2<224)) b2+=32;
		}
		if (b==b2) j++;
		else j=0;
	}
	if (j<l) return -1;
	return i;
}

LPBYTE __stdcall FindInString2(LPBYTE s, LPCTSTR s2, DWORD Size, BOOL first, BOOL NeedTrimSpace)
{
	WORD l=lstrlen(s2);
	WORD j=0;
	LPBYTE f;
	while (Size-- && j<l)
	{
		char b=*s;
		if (b>96 && b<123) b-=32;
		if (b==s2[j] || s2[j]==*s)
		{
			if (!j) f=s;
			j++;
		}
		else j=0;
		s++;
		if (NeedTrimSpace) while (Size && *s==32)
		{
			Size--;
			s++;
		}
	}
	if (j<l) return NULL;
	return first ? f:s;
}

void __stdcall ExtractFilePath(LPCTSTR filename,LPTSTR path)
{
	WORD i=lstrlen(filename)-1;
	if (i>60000) return;
	while (filename[i--]!=0x5c);
	i++;
	path[i+1]=0;
	do
	{
		path[i]=filename[i];
	}
	while (i--!=0);
}

BOOL __stdcall FileExists(LPCTSTR fil)
{
	WIN32_FIND_DATA		FindData;
	HANDLE Handle = FindFirstFile(fil, &FindData);
	if (Handle!=INVALID_HANDLE_VALUE) FindClose(Handle);
	return Handle!=INVALID_HANDLE_VALUE;
}

BOOL __stdcall DirectoryExists(LPCTSTR Name)
{
	int Code=GetFileAttributes(Name);
	return (Code!=-1) && ((FILE_ATTRIBUTE_DIRECTORY & Code)!=0);
}

void __stdcall FillBackGround(HDC dc,LPRECT rc,HBITMAP im)
{
	HDC dci=CreateCompatibleDC(dc);
	SelectObject(dci,im);
	BITMAPINFOHEADER bi;
	GetObject(im,sizeof(bi),(LPVOID)&bi);
	int x=rc->left;
	int y=rc->top;
	int dx,dy;
	while (y<rc->bottom) 
	{
		if (x+bi.biWidth>rc->right) dx=rc->right-x;
		else dx=bi.biWidth;
		if (y+bi.biHeight>rc->bottom) dy=rc->bottom-y;
		else dy=bi.biHeight;
		BitBlt(dc,x,y,dx,dy,dci,0,0,SRCCOPY);
		x+=bi.biWidth;
		if (x>=rc->right)
		{
			x=rc->left;
			y+=bi.biHeight;
		}
	}
	DeleteDC(dci);
}

static BOOL need=TRUE;

void __stdcall ChangeSoundConfig()
{
	if (!ReadRegistry("Sounds",(LPBYTE)&need,1)) need=TRUE;
}

void __stdcall MySound()
{
	if (!need) return;
	sndPlaySound((LPSTR)lpBeepWave, SND_ASYNC | SND_MEMORY);
}

void __stdcall GotoUrl(HWND Owner, LPCTSTR lpstrUrl)
{
	#define TEMPNAME "c://test.html"
	HANDLE hFile= CreateFile(TEMPNAME,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE) CloseHandle(hFile);
	LPTSTR ch=(char*)malloc(300);
	FindExecutable(TEMPNAME,NULL,ch);
	if (lstrlen(ch)>0)	ShellExecute(Owner,NULL,ch,lpstrUrl,NULL,SW_NORMAL);
	else ShellExecute(Owner,NULL,lpstrUrl,NULL,NULL,SW_NORMAL);
	free(ch);
	DeleteFile(TEMPNAME);
}

void __stdcall DrawAlignText(HDC hdc, LONG x, LONG y, LPCTSTR s, BOOL fCen)
{
	SIZE siz;
	GetTextExtentPoint32(hdc,s,lstrlen(s),&siz);
	TextOut(hdc,(int)(x-siz.cx/(fCen ? 2:1)),y,s,lstrlen(s));
}

BOOL __stdcall GetModulePath(HINSTANCE hIns, char* Dir, int size)
{
	GetModuleFileName(hIns, Dir, size);
	int i=lstrlen(Dir);
	while ((i>0) && (Dir[i]!=92)) i--;
	Dir[++i]=0;
	return TRUE;
}

BOOL __stdcall GetDataDir(char* Dir, int size)
{
	Dir[0]=0;
#ifdef NoPortable
	int i;
	LPITEMIDLIST pi;
	if (SHGetSpecialFolderLocation(0,CSIDL_APPDATA,&pi)==NOERROR)
		if (SHGetPathFromIDList(pi,Dir))
		{
			i=lstrlen(Dir);
			if (Dir[i-1]!='\\') strcat(Dir,"\\");
			strcat(Dir,"OrgPanel\\");
		}
#endif
	if (Dir[0]==0) GetModulePath(hInstance,Dir,size);
	strcat_s( Dir,size, "Data\\");
	return TRUE;
}

WORD __stdcall GetBuild(HMODULE hMod)
{
	HRSRC sr=FindResource(hMod,(char*)VS_VERSION_INFO,RT_VERSION);
	if (!sr) return 0;
	HGLOBAL hGlob=LoadResource(hMod,sr);
	if (!hGlob) return 0;
	char*	vv=(char*)LockResource(hGlob);
	vv+=3*sizeof(WORD);
	vv+=16*sizeof(WORD);
	int i=sizeof(VS_FIXEDFILEINFO);
	int j=i/4;
	i=i-4*j;
	vv+=i;
	VS_FIXEDFILEINFO* vs=(VS_FIXEDFILEINFO*)vv;
	WORD res=HIWORD(vs->dwFileVersionLS);
	FreeResource(hGlob);
	return res;
}

typedef BOOL (WINAPI* TIMESYNCPROC)(HWND, int);
typedef BOOL (WINAPI* CHECKUPDATEPROC)(HWND);

BOOL __stdcall TimeSync(BOOL bShow, HWND hwnd)
{
	char *Dir=(char*)malloc(300);
	GetModulePath(hInstance,Dir,300);
	strcat_s(Dir,300,"OrgInet.dll");
	HINSTANCE hIns=LoadLibrary(Dir);
	free(Dir);
	if (!hIns) return TRUE;
	TIMESYNCPROC proc=(TIMESYNCPROC)GetProcAddress(hIns,"InetTimeSync");
	BOOL res=TRUE;
	if (proc) res=proc(hwnd, bShow);
	FreeLibrary(hIns);
	return res;
}

BOOL __stdcall CheckUpdate(HWND hwnd)
{
	char *Dir=(char*)malloc(300);
	GetModulePath(hInstance,Dir,300);
	strcat_s(Dir,300,"OrgInet.dll");
	HINSTANCE hIns=LoadLibrary(Dir);
	free(Dir);
	if (!hIns) return TRUE;
	CHECKUPDATEPROC proc=(CHECKUPDATEPROC)GetProcAddress(hIns,"InetCheckUpdate");
	BOOL res=TRUE;
	if (proc) res=proc(hwnd);
	FreeLibrary(hIns);
	return res;
}

void __stdcall PaintBack(HWND hWnd, HBITMAP hBitmap, WNDPROC wndProc, int dx, int dy)
{
	PAINTSTRUCT ps;
	BeginPaint(hWnd,&ps);
	RECT rc;
	RECT rcClip;
	GetClientRect(hWnd, &rc);
	GetClipBox(ps.hdc,&rcClip);
	if (rcClip.bottom==0 && rcClip.right==0) 
	{
		EndPaint(hWnd,&ps);
		return;
	}
	CMemoryBitmap *cmb=new CMemoryBitmap();
	cmb->NewEmptyBitmap(ps.hdc,rc.right,rc.bottom);

	CallWindowProc(wndProc, hWnd, WM_PAINT, (WPARAM)cmb->hdc, 0);
	if (hBitmap)
	{
		CMemoryBitmap *bitMask=new CMemoryBitmap();
		bitMask->handle=CreateBitmap( rc.right, rc.bottom,
						1, 1, NULL );
		bitMask->hdc=CreateCompatibleDC(ps.hdc);
		SelectObject(bitMask->hdc,bitMask->handle);
		SetBkColor(bitMask->hdc,GetSysColor( IsWindowEnabled(hWnd) ? COLOR_WINDOW : COLOR_3DFACE ) );

		BitBlt(bitMask->hdc, 0, 0, rc.right, rc.bottom, cmb->hdc, 
					rc.left, rc.top, SRCCOPY );

		CMemoryBitmap *tempBit=new CMemoryBitmap();
		tempBit->NewEmptyBitmap(ps.hdc,rc.right,rc.bottom);

		RECT rcPaint=rc;

		rcPaint.left+=dx;
		rcPaint.top+=dy;

		FillBackGround(tempBit->hdc,&rcPaint,hBitmap);
		
		SetBkColor(cmb->hdc, RGB(0,0,0));
		SetTextColor(cmb->hdc, RGB(255,255,255));
		BitBlt(cmb->hdc, rcClip.left, rcClip.top, rcClip.right-rcClip.left, rcClip.bottom-rcClip.top, bitMask->hdc,
				rcClip.left, rcClip.top, SRCAND);

		SetBkColor(tempBit->hdc, RGB(255,255,255));
		SetTextColor(tempBit->hdc, RGB(0,0,0));
		BitBlt(tempBit->hdc, rcClip.left, rcClip.top, rcClip.right-rcClip.left, rcClip.bottom-rcClip.top, bitMask->hdc, 
				rcClip.left, rcClip.top, SRCAND);

		BitBlt(tempBit->hdc, rcClip.left, rcClip.top, rcClip.right-rcClip.left, rcClip.bottom-rcClip.top, 
					cmb->hdc, rcClip.left, rcClip.top,SRCPAINT);

		BitBlt(ps.hdc, rcClip.left, rcClip.top, rcClip.right-rcClip.left, rcClip.bottom-rcClip.top, 
					tempBit->hdc, rcClip.left, rcClip.top, SRCCOPY );
		delete tempBit;
		delete bitMask;
	}
	else
	BitBlt(ps.hdc,rcClip.left, rcClip.top, rcClip.right-rcClip.left, 
				rcClip.bottom-rcClip.top, cmb->hdc, 
				rcClip.left, rcClip.top, SRCCOPY );
	delete cmb;
	EndPaint(hWnd,&ps);
}

void __stdcall DrawTransparentBitmap(HDC hdc, int x, int y, HBITMAP bit, COLORREF tranColor)
{
	BITMAPINFOHEADER bi;
	GetObject(bit,sizeof(bi),(LPVOID)&bi);

	HDC hdcbit=CreateCompatibleDC(hdc);
	SelectObject(hdcbit, bit);

	CMemoryBitmap *bitMask=new CMemoryBitmap();
	bitMask->handle=CreateBitmap(bi.biWidth, bi.biHeight, 1, 1, NULL);
	bitMask->hdc=CreateCompatibleDC(hdc);

	SelectObject(bitMask->hdc,bitMask->handle);
	SetBkColor(bitMask->hdc, tranColor);

	CMemoryBitmap *mem=new CMemoryBitmap();
	mem->NewEmptyBitmap(hdc,bi.biWidth,bi.biHeight);

	BitBlt(mem->hdc, 0,0, bi.biWidth, bi.biHeight, hdcbit, 0, 0, SRCCOPY );
	BitBlt(bitMask->hdc, 0, 0, bi.biWidth, bi.biHeight, mem->hdc, 0, 0, SRCCOPY);
	
	SetBkColor(mem->hdc,RGB(0,0,0));
	SetTextColor(mem->hdc,RGB(255,255,255));
	BitBlt(mem->hdc, 0, 0, bi.biWidth, bi.biHeight, bitMask->hdc, 0, 0, SRCAND);
	
	SetBkColor(hdc, RGB(255,255,255));
	SetTextColor(hdc, RGB(0,0,0));
	BitBlt(hdc, x, y, bi.biWidth, bi.biHeight, bitMask->hdc, 0, 0, SRCAND);
	BitBlt(hdc,x, y, bi.biWidth, bi.biHeight, mem->hdc, 0, 0, SRCPAINT);

	delete bitMask;
	delete mem;
	DeleteDC(hdcbit);
}
