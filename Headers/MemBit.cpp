#include <windows.h>
#include "MemBit.h"

void CMemoryBitmap::LoadMemBitmap(HINSTANCE hInstance, LPCTSTR strName)
{
	if (hdc || handle) FreeBitmap();
	HDC dc=::GetDC(0);
	hdc=CreateCompatibleDC(dc);
	if (strName)
	{
		handle=LoadBitmap(hInstance,strName);
		SelectObject(hdc,handle);
	}
	else handle=NULL;
	ReleaseDC(0,dc);
}

void CMemoryBitmap::NewEmptyBitmap(HDC dc, int w, int h)
{
	if (hdc || handle) FreeBitmap();
	hdc=CreateCompatibleDC(dc);
	handle=CreateCompatibleBitmap(dc,w,h);
	SelectObject(hdc,handle);
}

void CMemoryBitmap::NewBitmap(CMemoryBitmap* b, int x, int y, int w, int h)
{
	if (hdc || handle) FreeBitmap();
	hdc=CreateCompatibleDC(b->hdc);
	handle=CreateCompatibleBitmap(b->hdc,w,h);
	SelectObject(hdc,handle);
	BitBlt(hdc,0,0,w,h,b->hdc,x,y,SRCCOPY);
}

void CMemoryBitmap::FreeBitmap()
{
	if (hdc) DeleteDC(hdc);
	if (handle) DeleteObject(handle);
}

CMemoryBitmap::CMemoryBitmap(HINSTANCE hInstance, LPCTSTR strName)
{
	hdc=NULL;
	handle=NULL;
	LoadMemBitmap(hInstance,strName);
}

CMemoryBitmap::CMemoryBitmap()
{
	hdc=NULL;
	handle=NULL;
}

CMemoryBitmap::~CMemoryBitmap()
{
	FreeBitmap();
}

