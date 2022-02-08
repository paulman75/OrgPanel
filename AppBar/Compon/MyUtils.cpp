#include "windows.h"
#include "..\..\Headers\general.h"
#include "MyUtils.h"
#include "..\..\Headers\const.h"
#include "..\RegUnit.h"

extern TBarConfig	BarCon;

char sMonth[12][10]={"Январь","Февраль","Март","Апрель","Май","Июнь","Июль","Август","Сентябрь","Октябрь","Ноябрь","Декабрь"};
char sDayWeek[7][3]={"ПН","ВТ","СР","ЧТ","ПТ","СБ","ВС"};
char srMonth[12][10]={"Января","Февраля","Марта","Апреля","Мая","Июня","Июля","Августа","Сентября","Октября","Ноября","Декабря"};
char sWeekDay[7][12]={"Понедельник","Вторник","Среда","Четверг","Пятница","Суббота","Воскресенье"};
extern char MainDir[100];

void ExtractCharAfterThis(LPCTSTR path, LPTSTR name, byte simbol, BOOL NeedCase)
{
	name[0]=0;
	WORD i=lstrlen(path)-1;
	if (i>60000) return;
	while (i && path[i--]!=simbol);
	if (i==0) return;
	i++;
	byte k=0;
	byte b;
	while (path[i]!=0) 
	{
		b=path[i++];
		if (NeedCase && b>64 && b<91) b+=32;
		name[k++]=b;
	}
	name[k]=0;
}

void ExtractFileExt(LPCTSTR path,LPTSTR buf)
{
	ExtractCharAfterThis(path,buf,0x2e, TRUE);
}

void ExtractFileName(LPCTSTR path,LPTSTR name)
{
	ExtractCharAfterThis(path,name,0x5c, FALSE);
	WORD i=1;
	do
		name[i-1]=name[i];
	while (name[i++]!=0);
}

void MyMoveFile(LPCTSTR sFrom, LPCTSTR sTo, LPCTSTR name)
{
	char* s1=(char*)malloc(300);
	char* s2=(char*)malloc(300);

	strcpy(s1,sFrom);
	strcat(s1,name);
	strcpy(s2,sTo);
	strcat(s2,name);
	if (s1[lstrlen(s1)-1]!='*')
		if (!FileExists(s1)) return;
	if (s2[lstrlen(s2)-3]=='*')	s2[lstrlen(s2)-2]=0;
	s1[lstrlen(s1)+1]=0;

	SHFILEOPSTRUCT fo;
	fo.hwnd=0;
	fo.wFunc=FO_COPY;
	fo.pFrom=s1;
	fo.pTo=s2;
	fo.fFlags=FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
	fo.fAnyOperationsAborted=FALSE;
	SHFileOperation(&fo);

	fo.hwnd=0;
	fo.wFunc=FO_DELETE;
	fo.pFrom=s1;
	fo.fFlags=FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
	fo.fAnyOperationsAborted=FALSE;
	SHFileOperation(&fo);
	free(s1);
	free(s2);
}

typedef struct
{
	char	Name[40];
	char	Shir[7];
	char	Dolg[7];
} TCITY;

#define CITY_COUNT 52
TCITY City[CITY_COUNT]=
{
{"Анапа","44.53","37.18"},
{"Архангельск","64.34","40.32"},
{"Астана","51.10","71.30"},
{"Астрахань","46.21","48.03"},
{"Ашхабад","37.57","58.23"},
{"Баку","40.22","49.53"},
{"Барнаул","53.22","83.45"},
{"Белгород ","50.36","36.34"},
{"Бишкек","42.53","74.46"},
{"Владивосток","43.09","131.53"},
{"Владимир","56.10","40.25"},
{"Волгоград","48.44","44.25"},
{"Воркута","67.27","63.58"},
{"Ереван","40.10","44.31"},
{"Иркутск","52.16","104.2"},
{"Казань","55.45","49.10"},
{"Калининград","54.43","20.30"},
{"Калуга","54.31","36.16"},
{"Киев","50.28","30.29"},
{"Кишинев","47","28.5"},
{"Краснодар ","45.02","39.00"},
{"Красноярск","56.05","92.46"},
{"Курган","55.26","65.18"},
{"Курск","51.42","36.12"},
{"Магадан","59.34","150.48"},
{"Махачкала","42.58","47.30"},
{"Минск","53.51","27.3"},
{"Москва", "55.45","37.37"},
{"Мурманск","68.58","33.05"},
{"Нальчик","43.29","43.37"},
{"Новгород","58.3","31.2"},
{"Новосибирск","55.04","82.55"},
{"Норильск","69.20","88.06"},
{"Одесса","46.30","30.46"},
{"Омск","55.00","73.22"},
{"Пермь","58.0","56.15"},
{"Петрозаводск","61.47","34.20"},
{"Рига","56.53","24.05"},
{"Рязань","54.38","39.44"},
{"Самара","53.1","50.15"},
{"Санкт-Петербург","59.55","30.25"},
{"Саратов","51.34","46.02"},
{"Свердловск","56.52","60.35"},
{"Смоленск","54.47","32.03"},
{"Таганрог","47.12","38.56"},
{"Тбилиси","41.43","44.48"},
{"Томск","56.30","84.58"},
{"Уфа","54.45","55.58"},
{"Хабаровск","48.27","135.06"},
{"Хатанга","71.58","102.3"},
{"Челябинск","55.10","61.25"},
{"Чита","52.03","113.30"}
};

void FillCity(HWND hCombo)
{
	for (int i=0; i<CITY_COUNT; i++)
		SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)City[i].Name);
}

void ChangeCity(HWND hCombo,HWND hD, HWND hS)
{
	int i=SendMessage(hCombo,CB_GETCURSEL,0,0);
	if (i==-1) return;
	SetWindowText(hD,City[i].Dolg);
	SetWindowText(hS,City[i].Shir);
}
