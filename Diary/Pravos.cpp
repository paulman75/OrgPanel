#include <windows.h>
#include "../Headers/general.h"
#include "Pages.h"
#include "Pravos.h"
#include "../Headers/const.h"

typedef struct
{
	byte Day;
	byte Month;
	char Text[60];
} TPrav;

#define PRAV_COUNT 19
const TPrav Prav[PRAV_COUNT]={
	{7,1,"Рождество"},
	{14,1,"Обрезание Господне"},
	{19,1,"Крещение Господне. Богоявление"},
	{15,2,"Сретение Господне"},
	{25,2,"Празднование Иверской иконы Божией Матери"},
	{7,4,"Благовещение Пресвятой Богородицы"},
	{5,5,"Память преподобного Виталия Александрийского"},
	{7,7,"Рождество Иоанна Предтечи"},
	{12,7,"Святых первоверховных апостолов Петра и Павла"},
	{14,8,"Начало Успенского поста"},
	{19,8,"Преображение Господне"},
	{28,8,"Успение Пресвятой Богородицы"},
	{11,9,"Усекновение главы Иоанна Предтечи"},
	{21,9,"Рождество Пресвятой Богородицы"},
	{27,9,"Воздвижение Креста Господня"},
	{14,10,"Покров Пресвятой Богородицы"},
	{4,11,"Празднование в честь Казанской иконы Божией Матери"},
	{28,11,"Начало Рождественского поста"},
	{4,12,"Введение во храм Пресвятой Богородицы"}
};

void AddPrav(PPage pag)
{
	//Рассчитываем пасху
	int a,b,c,d,e;
	a=pag->Date.Year % 19;
	b=pag->Date.Year % 4;
	c=pag->Date.Year % 7;
	d=(19*a + 15) % 30;
	e=(2*(b + 2*c + 3*d + 3)) % 7;
	TDate easd;
	easd.Year=pag->Date.Year;
	easd.Month=3;
	easd.Day=22;
	DateForward(&easd,13+d + e);

	if (easd==pag->Date)
	AddStrings("Светлое Христово Воскресение. Пасха",pag,ID_PRAV,0,TRUE);
	TDate dd=easd;
	DateBefore(&dd,48);
	if (dd==pag->Date)
	AddStrings("Прощенное воскресенье",pag,ID_PRAV,0,TRUE);
	dd=easd;
	DateBefore(&dd,47);
	if (dd==pag->Date)
	AddStrings("Начало великого поста",pag,ID_PRAV,0,TRUE);
	dd=easd;
	DateBefore(&dd,7);
	if (dd==pag->Date)
	AddStrings("Вход Господень в Иерусалим (Вербное воскресенье)",pag,ID_PRAV,0,TRUE);

	dd=easd;
	DateForward(&dd,39);
	if (dd==pag->Date)
	AddStrings("Вознесение Господне",pag,ID_PRAV,0,TRUE);
	dd=easd;
	DateForward(&dd,49);
	if (dd==pag->Date)
	AddStrings("День Святой Троицы, Пятидесятница",pag,ID_PRAV,0,TRUE);
	dd=easd;
	DateForward(&dd,57);
	if (dd==pag->Date)
	AddStrings("Начало Петрова поста",pag,ID_PRAV,0,TRUE);
	for (int i=0; i<PRAV_COUNT; i++)
		if (pag->Date.Day==Prav[i].Day && pag->Date.Month==Prav[i].Month)
		AddStrings((char*)&Prav[i].Text[0],pag,ID_PRAV,0,TRUE);
}