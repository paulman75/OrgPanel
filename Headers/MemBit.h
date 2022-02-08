#ifndef __MEMBITMAP_H
#define __MEMBITMAP_H

class CMemoryBitmap
{
private:
public:
	HDC		hdc;
	HBITMAP	handle;
	void NewBitmap(CMemoryBitmap* b, int x, int y, int w, int h);
	void NewEmptyBitmap(HDC dc, int w, int h);
	void FreeBitmap();
	CMemoryBitmap();
	CMemoryBitmap(HINSTANCE hInstance, LPCTSTR strName);
	void LoadMemBitmap(HINSTANCE hInstance, LPCTSTR strName);
	~CMemoryBitmap();
};

#endif