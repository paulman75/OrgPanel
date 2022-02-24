#ifdef _MYDLL_
#define MYAPI __declspec(dllexport)
#else
#define MYAPI __declspec(dllimport)
#endif

typedef struct TDate
{
	byte	Day;
	byte	Month; //0 - дата не задана
	WORD	Year;
	BOOL	operator <(const TDate &d1);
	BOOL	operator <=(const TDate &d1);
	BOOL	operator >(const TDate &d1);
	BOOL    operator ==(const TDate &d1);
	TDate&  operator ++();
	TDate	operator ++(int);
	TDate&  operator --();
	TDate	operator --(int);
} TDate, *PDate;

#define RegistryPath	"SoftWare\x5CPaulSoft\x5COrgBar"

extern "C" MYAPI byte __stdcall MessageDlg(HWND,LPCTSTR,LPCTSTR,LPCTSTR);
extern "C" MYAPI BOOL __stdcall InputQuery(HWND,LPCTSTR,LPCTSTR,LPCTSTR, LPTSTR, WORD);
extern "C" MYAPI void __stdcall PutOnCenter(HWND hh);

extern "C" MYAPI void __stdcall UsePassword();
extern "C" MYAPI BOOL __stdcall CheckPassword(BOOL, HWND);
extern "C" MYAPI void __stdcall FillBackGround(HDC,LPRECT,HBITMAP);
extern "C" MYAPI void __stdcall MySound();
extern "C" MYAPI void __stdcall ChangeSoundConfig();
extern "C" MYAPI void __stdcall ExtractFilePath(LPCTSTR,LPTSTR);
extern "C" MYAPI BOOL __stdcall FileExists(LPCTSTR);
extern "C" MYAPI BOOL __stdcall DirectoryExists(LPCTSTR Name);
extern "C" MYAPI void __stdcall ShowAbout(HWND, LPCTSTR, HINSTANCE);
extern "C" MYAPI void __stdcall StartCalendar(PDate Dat,HWND Owner, BOOL bCanNo);

extern "C" MYAPI void __stdcall CreateDateLabel(HWND hwnd, int x, int y, PDate lpdate, byte ID, BOOL bCanNo);
extern "C" MYAPI BOOL __stdcall GetDate(PDate lpDate, byte ID);
extern "C" MYAPI BOOL __stdcall SetDate(PDate lpDate, byte ID);
extern "C" MYAPI void __stdcall DeleteAllDateLabels();
extern "C" MYAPI void __stdcall DeleteAllDateLabelsOnWindow(HWND hwnd);
extern "C" MYAPI void __stdcall ShowDateLabel(byte ID, BOOL bShow);

extern "C" MYAPI void __stdcall CreateRichEdit(HWND hwnd, int x, int y, WORD w, WORD h, byte ID);
extern "C" MYAPI HWND __stdcall GetRichEditWnd(byte ID);
extern "C" MYAPI LONG __stdcall RichEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, byte ID);
extern "C" MYAPI void __stdcall SetRichEditText(byte ID, LPCTSTR lpstrText);
extern "C" MYAPI void __stdcall AddTextToRichEdit(byte ID, LPCTSTR lpstrText);
extern "C" MYAPI void __stdcall DeleteAllRichEdit();

extern "C" MYAPI void __stdcall DrawBitmap(HDC hdc, HBITMAP hBit, LONG x, LONG y);
extern "C" MYAPI void __stdcall PaintBack(HWND hWnd, HBITMAP hBitmap, WNDPROC wndProc, int dx, int dy);
extern "C" MYAPI void __stdcall GotoUrl(HWND Owner, LPCTSTR lpstrUrl);
extern "C" MYAPI void __stdcall DrawTransparentBitmap(HDC hdc, int x, int y, HBITMAP bit, COLORREF tranColor);

extern "C" MYAPI TDate __stdcall Now();
extern "C" MYAPI WORD __stdcall mod(DWORD w,WORD d);
extern "C" MYAPI DWORD __stdcall EncodeDate(PDate Dat);
extern "C" MYAPI void __stdcall DateBefore(PDate d,int len);
extern "C" MYAPI void __stdcall DateForward(PDate d,int len);
extern "C" MYAPI byte __stdcall DayofWeek(PDate dd);
extern "C" MYAPI void __stdcall DateToString(PDate Dat,LPTSTR res);
extern "C" MYAPI void __stdcall DateToStringWithoutYear(PDate Dat,LPTSTR res);
extern "C" MYAPI void __stdcall WeekDayToString(LPTSTR St,byte w);
extern "C" MYAPI void __stdcall MonthToString(LPTSTR St,byte Mon);
extern "C" MYAPI void __stdcall YearToString(LPTSTR St, WORD Y);

extern "C" MYAPI int  __stdcall FindInString(LPCTSTR s, LPCTSTR s2, BOOL bCase=FALSE);
extern "C" MYAPI LPBYTE  __stdcall FindInString2(LPBYTE s, LPCTSTR s2, DWORD Size, BOOL first, BOOL NeedTrimSpace);
extern "C" MYAPI int __stdcall StringCompare(LPCTSTR s1, LPCTSTR s2);
extern "C" MYAPI void __stdcall DrawAlignText(HDC hdc, LONG x, LONG y, LPCTSTR s, BOOL fCen);

extern "C" MYAPI void __stdcall ShowBioritm(HWND hWnd);
extern "C" MYAPI void __stdcall MakeShortName(LPCTSTR Name, LPCTSTR MName, LPCTSTR SName, LPTSTR s);

extern "C" MYAPI void __stdcall DeleteAllSensorLabels();
extern "C" MYAPI BOOL __stdcall SetSensorCaption(LPCTSTR Cap, byte ID);
extern "C" MYAPI void __stdcall ShowSensorLabel(byte ID, BOOL bShow);
extern "C" MYAPI void __stdcall CreateSensorLabel(HWND hwnd, int x, int y, int Col1, int Col2, LPCTSTR Cap, byte Cur, int ID);

extern "C" MYAPI BOOL __stdcall GetHumanName(int iNameIndex, LPTSTR name);
extern "C" MYAPI BOOL __stdcall GetHumanImenDate(int iNameIndex, int iDateIndex, PDate dat);
extern "C" MYAPI int __stdcall GetNameIndex(LPCTSTR name);
extern "C" MYAPI BOOL __stdcall GetHumanMainImenDate(int iNameIndex, PDate bDate, PDate dat);

extern "C" MYAPI BOOL __stdcall GetDataDir(char* Dir, int size);
extern "C" MYAPI BOOL __stdcall TimeSync(BOOL bShow, HWND hwnd);
extern "C" MYAPI BOOL __stdcall CheckUpdate(HWND hwnd);
extern "C" MYAPI BOOL __stdcall GetModulePath(HINSTANCE hIns, char* Dir, int size);
extern "C" MYAPI WORD __stdcall GetBuild(HMODULE hMod);

extern "C" MYAPI BOOL __stdcall ReadRegistry(LPCTSTR Cap, LPBYTE poi, DWORD len);
extern "C" MYAPI void __stdcall WriteStringToReg(LPCTSTR Cap, LPCTSTR Text);
extern "C" MYAPI void __stdcall WriteDWordToReg(LPCTSTR Cap, DWORD Val);
extern "C" MYAPI void __stdcall WriteBinToReg(LPCTSTR Cap, LPBYTE Val, int size);
