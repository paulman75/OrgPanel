#ifndef __CONST_H_
#define __CONST_H_

#define WM_CALENDARDATECHANGE  WM_USER+7
#define WM_CHECKPASSWORD	WM_USER+395
#define WM_PASSWORDUSED		WM_USER+396
#define OK_PASS				78546

#define WM_SWITCHVALUECHANGED WM_USER+9
#define WM_SWITCHGETFOLDERNAME WM_USER+10

#define WM_APPBARCHANGESTATE WM_USER+11

#define WM_COLORDIALOGCHANGE WM_USER+152

#define WM_APPBARCHANGEFOLDER WM_USER+12


#define	WM_LISTBOXCHECKCHANGE	WM_USER+20
//HIWORD(wParam) - ID контрола
//LOWORD(wParam) - индекс элемента
//lParam - BOOL - новое значение
//родительское окно может запретить переключение, 
//вернув значение отличное от 0.
#define WM_LISTBOXTABDOWN		WM_USER+21
#define WM_LISTBOXENDDRAG		WM_USER+23
//HIWORD(wParam) - ID контрола
//LOWORD(wParam) - индекс элемента который переносим
//HIWORD(lParam) - ID контрола на который положили
//LOWORD(lParam) - индекс элемента куда положили

#define WM_SPINVALUECHANGED WM_USER+5

//For tool Bar2
#define WM_TOOLBARBUTTONCLICK WM_USER+153
#define WM_TOOLBARBUTTONRCLICK WM_USER+154
#define WM_TOOLBARBUTTONABOVE WM_USER+155
#define WM_DROPTOOLBUT WM_USER+156

#define WM_UPDOWNBUTTONCLICK WM_USER+22


#define WM_COLUMNORDERCHANGE	WM_USER+120
//lParam-номер колонки(неизменный),
//wParam-новый порядковый номер
#define WM_COLUMNWIDTHCHANGE	WM_USER+121
//lParam-номер измененного столбца, wParam-новая ширина
#define WM_COLUMNOPTION			WM_USER+122
#define WM_COLUMNHIDE			WM_USER+123
//lParam-номер столбца

#define WM_MENULOADED WM_USER+169	
//wParam=hMenu, lParam=ButID

#define WM_FILLBIOCOMBO	WM_USER+170

#define WM_MOUSELEAVE	0x02A3
#define WM_SENSORLABELCLICK WM_USER+6
//wParam=hWnd, lParam=ID

#define WM_CONFIGCHANGE WM_USER+135

#define ID_DELA	0
#define ID_ROV	1
#define ID_IMEN	2
#define ID_PRAZ	3
#define ID_PRIM	4
#define ID_EVENT 5
#define ID_PRAV 6
#define DNEV_COUNT 7

#define DiaryWindow "OrgPanelDiaryWindow"
#define NoteBookWindow "OrgPanelNoteBookWindow"
#define NotePadWindow "OrgPanelNotePadWindow"

#define NOTEBOOK_FILE "book.dat"
#define NOTEPAD_FILE "notepad.dat"
#define DNEV_FILE "dnev.dat"

#endif