#include "Astro.h"
#include "Classes.h"

typedef struct
{
		byte		Index;
        byte		IconType;
        CMyString	Text;
        LPVOID		poi;
		byte		ColorType; //(0- расный, 1-нормальный, 2-зеленый);
} TStrings,*PStrings;

typedef struct
{
	int			TopStIndex;
	int			ActiveIndex;
	TStrings	st[60];
	int			CountSt;		//количество строк
	TDate		Date;
	int			x0;
	int			Tab;
	double		Sun1;
	double		Sun2;
	double		Moon1;
	double		Moon2;
	TMoonPhase	MoonPhase;
	double		PhaseTime;
	BOOL		TopBut;
	BOOL		BotBut;
}TPage, *PPage;

typedef enum {mDnev, mDelo} TMode;

void PaintDnev();
void OnSize();
void CalcAstro(TPage* pag);

void InitPages();
void FreePages();
void UpdatePage(PPage pag, BOOL bSetNoActive);
BOOL SetActiveNote(WORD x,WORD y, int h);
void AddStrings(LPTSTR s,PPage pp,byte sType,LPVOID poi,byte ColorType);
void GetActiveNote(PPage *ppp, int* StIndex);
void PaintBack(HDC dc);
