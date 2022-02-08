#include "windows.h"
#include "Bitmap.h"

extern HINSTANCE hInstance;

void CMyBitmap::CreateMyBitmap(WORD Width, WORD Height)
{
	bb.bmiHeader.biBitCount=24;
	bb.bmiHeader.biHeight=Height;
	bb.bmiHeader.biWidth=Width;
	bb.bmiHeader.biCompression=0;
	bb.bmiHeader.biPlanes=1;
	bb.bmiHeader.biSize=40;
	bb.bmiHeader.biSizeImage=40+Width*Height*3;
	bb.bmiHeader.biClrImportant=0;
	bb.bmiHeader.biClrUsed=0;
	bb.bmiHeader.biXPelsPerMeter=0;
	bb.bmiHeader.biYPelsPerMeter=0;
	HDC dc=GetDC(0);
	hdc=CreateCompatibleDC(dc);
	LPVOID		Poi;
	hBit=CreateDIBSection(hdc,&bb,DIB_RGB_COLORS,&Poi,0,0);
	SelectObject(hdc,hBit);
	RECT rc;
	rc.left=rc.top=0;
	rc.bottom=Height;
	rc.right=Width;
	FillRect(hdc,&rc,(HBRUSH)COLOR_WINDOW);
	ReleaseDC(0,dc);
}

void CMyBitmap::SaveBitmap(LPCTSTR path)
{
	HANDLE	hFile;
 
hFile = CreateFile(path,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
if (hFile==INVALID_HANDLE_VALUE) return;

BITMAPFILEHEADER Head;
Head.bfType=0x4d42;
Head.bfReserved1=Head.bfReserved2=0;
Head.bfOffBits=0x36;
Head.bfSize=bb.bmiHeader.biHeight*bb.bmiHeader.biWidth*3+54;
DWORD	dwd;
WriteFile(hFile,&Head,14,&dwd,NULL);
WriteFile(hFile,&bb.bmiHeader,40,&dwd,NULL);
LPVOID	 pBits;
pBits=GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, bb.bmiHeader.biSizeImage);
GetDIBits(hdc, hBit, 0, bb.bmiHeader.biHeight, (LPSTR)pBits, &bb, DIB_RGB_COLORS);
WriteFile(hFile,(LPCVOID)pBits,bb.bmiHeader.biSizeImage,&dwd,NULL);
GlobalFree(pBits);
CloseHandle(hFile);
}

CMyBitmap::CMyBitmap(LPCTSTR path)
{
    HANDLE          hFile, hMapFile;
    LPVOID          pMapFile;

	hdc=NULL;
	hBit=NULL;
    if ((hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL)) == (HANDLE)-1) 
        return;
    if ((hMapFile = CreateFileMapping(hFile, NULL,
                             PAGE_READONLY, 0, 0, NULL))!=NULL) 
	{
		if ((pMapFile = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0)) != NULL) 
		{
			if (*((PWORD)pMapFile) == 0x4d42)
			{
				CopyMemory(&bb.bmiHeader,(PBYTE)pMapFile + sizeof(BITMAPFILEHEADER),40);
				CreateMyBitmap((WORD)bb.bmiHeader.biWidth,(WORD)bb.bmiHeader.biHeight);
				LPVOID	Poi;
				Poi=LocalAlloc(GMEM_FIXED, bb.bmiHeader.biSizeImage);
				GetDIBits(hdc, hBit, 0, bb.bmiHeader.biHeight, (LPSTR)Poi, &bb, DIB_RGB_COLORS);
				CopyMemory(Poi,(PBYTE)pMapFile+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER),
					bb.bmiHeader.biSizeImage);
				SetDIBits(hdc,hBit,0,bb.bmiHeader.biHeight,(LPSTR)Poi,&bb,DIB_RGB_COLORS);
				LocalFree(Poi);
			}
		}
		UnmapViewOfFile(pMapFile);
	}
	CloseHandle(hMapFile);
	CloseHandle(hFile);
}

CMyBitmap::CMyBitmap(WORD Width, WORD Height)
{
	CreateMyBitmap(Width, Height);
}

CMyBitmap::CMyBitmap(int iRes)
{
	hBit=LoadBitmap(hInstance,MAKEINTRESOURCE(iRes));
	GetObject(hBit,sizeof(bb),&bb);
	HDC dc=GetDC(0);
	hdc=CreateCompatibleDC(dc);
	SelectObject(hdc,hBit);
	ReleaseDC(0,dc);
}

CMyBitmap::~CMyBitmap()
{
	if (hdc!=NULL) DeleteDC(hdc);
	if (hBit!=NULL) DeleteObject(hBit);
	HDC dc=GetDC(0);
	hdc=CreateCompatibleDC(dc);
	SelectObject(hdc,hBit);
	ReleaseDC(0,dc);
}
