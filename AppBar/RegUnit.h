#if !defined __REG_H
#define __REG_H

typedef struct 
{
        byte Edge;
        BOOL AutoHide;
        char FName[15],SName[15];
        TDate	Birth;
		BOOL	LargeIcons;
        BOOL	UsedPassword;
        char	Password[15];
		byte	RestorePassword;
        byte	DeleteDelo;
		byte	DeleteEvent;
        byte	Dnev[DNEV_COUNT];
        WORD	DesktopX,DeskTopY;
		BOOL	PulseDial;
		char	Prefix[15];
		COLORREF	BackGr; //используется для Главной панели
		COLORREF	BackGr2; //используется для Главной панели
		BOOL	AutoOpen;
		int		OpenDelay;
		byte	Speed;

		BOOL	Sounds;

		char 	HourFileName[200];
		BOOL	bCuckoo;

		char 	HalfHourFileName[200];

		byte	SyncTime;
		LONGLONG LastSync;
		char 	DoFileName[200];
		char 	EventFileName[200];
		char 	RovFileName[200];
		BOOL	ShowFullName;
		BOOL	ShowSkyInfo;
		char	GeoD[20];
		char	GeoS[20];
		BOOL	FullImen;
		BOOL	TitShow;
		WORD	LastFolderIndex;//только для чтения
		BOOL    AlwaysOnTop;
} TBarConfig, *PBarConfig;
#endif

void BarConfigFromRegistry(PBarConfig);
BOOL IsStartUp();
void GetDesktop(LPTSTR);
void SetStartUp(BOOL cc);
void SaveOptions(PBarConfig bc);
BOOL IsDiaryStartUp();
void SetDiaryStartUp(BOOL cc);
BOOL ReadWordReg(LPTSTR sName, WORD *bValue);
void WriteWordReg(LPTSTR sName, WORD wValue);
void BadRegLnk(HWND hOwn);
void WriteWord2Ini(LPCTSTR Cap, DWORD Val);