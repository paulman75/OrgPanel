#define PASS_FILE "passkeep.psk"

#define IDC_ABOUT	103
#define IDC_MAGIC	104
#define IDC_GOTO	105

#include <commctrl.h>

typedef struct TUnitOld
{
	char	Name[40];
	char	URL[100];
	char	Login[30];
	char	Password[20];
	char	Comment[200];
	SYSTEMTIME	Time;
	TUnitOld*	Next;
}TUNITOLD, *LPUNITOLD;

typedef struct
{
	int		ID;
	int		Group_ID;
	char	Name[40];
	char	URL[100];
	char	Login[30];
	char	Password[20];
	char	Comment[200];
	SYSTEMTIME	Time;
	int		LoadIndex;
}TUNIT, *LPUNIT;

const int GROUPNAME_SIZE = 50;
typedef struct
{
	int		ID;
	int		Group_ID;
	char	Name[GROUPNAME_SIZE];
	int		LoadIndex;
} TGROUP, *LPGROUP;

BOOL LoadUnits();
void SaveUnits();
void ShowInfo(HTREEITEM hit);
void AddUnit();
void AddGroup();
void AddUnit(char* name, char* url);
void Del();
void SaveInfo(HTREEITEM hit);
void FillTree();
void Rename();
void OnDropped(int image1, int ID1, int image2, int ID2);
