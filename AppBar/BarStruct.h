#if !defined __BARSTRUCT__
#define __BARSTRUCT__

#include "Compon\Classes.h"
#include "../Headers/Bitmap.h"

#define ID_BARBUTTON	144

void CreateBarStruct();
void CheckExistsUnitName(LPTSTR FileName);

class BarUnit;
//BarUnit* FindBarUnit(WPARAM Index,LPARAM ToolBarIndex);
void BitmapToUnitFromIcon(BarUnit*);
void SaveMainBitmap();
void PlaceIcon();
void AddIcon(POINT* point);
void AddSplitter(POINT* point);
void HideButton(BarUnit*);
void DropUnit(LPCTSTR name,WORD x, WORD y);
void SaveBarStruct();
void DeleteUnit(BarUnit*);
void NewFolder();
void RenameUnit(BarUnit* uni);
void SaveFolderDat();
void FreeAllStruct();
void UnitDropEnd(BarUnit* nUnit, POINT* lpt);

typedef struct TBarFolder
{
        BOOL		Modified;
        CMyString*	Path;
        byte		Kol;
        struct TBarFolder * PrevFolder;
		struct TBarFolder * NextFolder; 
		BarUnit*	FirstUnit;
        CMyString*	Caption;
        byte		Index;
		COLORREF	BkColor;
		COLORREF	BkColor2;
		COLORREF	TitColor;
		byte		Alpha;
} TBarFolder, *PBarFolder;

void DeleteFolder(PBarFolder fol);
void RenameFolder(PBarFolder fol);
void MoveShortCutToFolder(BarUnit* nUnit, PBarFolder nFolder);

class BarUnit
{
private:
public:
	   CMyBitmap*	ButImage;
	   CMyBitmap*	BigImage;
       CMyString*	FilePath;
       CMyString*	Caption;
	   CMyString*	IconPath;
	   CMyString*	strPath;
	   int			IconIndex;
       BOOL			Hidden;
	   BOOL			Splitter;
	   PBarFolder	ParentFolder;
       BarUnit* 	PrevUnit;
	   BarUnit*		NextUnit;
	   BarUnit();
		void DeleteBitmap();
	   ~BarUnit();
};

#endif