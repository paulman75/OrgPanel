#ifndef __MYBITMAP_H
#define __MYBITMAP_H

class CMyBitmap
{
private:
	HBITMAP		hBit;
	BITMAPINFO	bb;
	void CreateMyBitmap(WORD, WORD);
public:
	HDC			hdc;
	inline HBITMAP		GetBitmap(){return hBit;};
	void SaveBitmap(LPCTSTR path);
	CMyBitmap(LPCTSTR path);
	CMyBitmap(WORD, WORD);
	CMyBitmap(int);
	~CMyBitmap();
};

#endif