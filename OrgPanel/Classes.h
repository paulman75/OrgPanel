#ifndef __CLASSES__
#define __CLASSES__

class CMyString
{
private:
	void AllocMore(WORD PlusNeedLen);
public:
// Constructors
	LPTSTR	Text;
	CMyString();
	CMyString(LPCTSTR lpsz);
	void Empty();
	const CMyString& operator=(LPCTSTR lpsz);
	const CMyString& operator=(const CMyString& string);
	const CMyString& operator+=(const CMyString& string);
	const CMyString& operator+=(LPCTSTR lpsz);
	friend BOOL operator==(const CMyString&, LPCTSTR lpsz);
	~CMyString();
};
#endif