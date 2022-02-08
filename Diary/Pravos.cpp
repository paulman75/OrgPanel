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
	{7,1,"���������"},
	{14,1,"��������� ��������"},
	{19,1,"�������� ��������. �����������"},
	{15,2,"�������� ��������"},
	{25,2,"������������ �������� ����� ������ ������"},
	{7,4,"������������ ��������� ����������"},
	{5,5,"������ ������������ ������� ����������������"},
	{7,7,"��������� ������ ��������"},
	{12,7,"������ �������������� ��������� ����� � �����"},
	{14,8,"������ ���������� �����"},
	{19,8,"������������ ��������"},
	{28,8,"������� ��������� ����������"},
	{11,9,"����������� ����� ������ ��������"},
	{21,9,"��������� ��������� ����������"},
	{27,9,"����������� ������ ��������"},
	{14,10,"������ ��������� ����������"},
	{4,11,"������������ � ����� ��������� ����� ������ ������"},
	{28,11,"������ ��������������� �����"},
	{4,12,"�������� �� ���� ��������� ����������"}
};

void AddPrav(PPage pag)
{
	//������������ �����
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
	AddStrings("������� �������� �����������. �����",pag,ID_PRAV,0,TRUE);
	TDate dd=easd;
	DateBefore(&dd,48);
	if (dd==pag->Date)
	AddStrings("��������� �����������",pag,ID_PRAV,0,TRUE);
	dd=easd;
	DateBefore(&dd,47);
	if (dd==pag->Date)
	AddStrings("������ �������� �����",pag,ID_PRAV,0,TRUE);
	dd=easd;
	DateBefore(&dd,7);
	if (dd==pag->Date)
	AddStrings("���� ��������� � ��������� (������� �����������)",pag,ID_PRAV,0,TRUE);

	dd=easd;
	DateForward(&dd,39);
	if (dd==pag->Date)
	AddStrings("���������� ��������",pag,ID_PRAV,0,TRUE);
	dd=easd;
	DateForward(&dd,49);
	if (dd==pag->Date)
	AddStrings("���� ������ ������, �������������",pag,ID_PRAV,0,TRUE);
	dd=easd;
	DateForward(&dd,57);
	if (dd==pag->Date)
	AddStrings("������ ������� �����",pag,ID_PRAV,0,TRUE);
	for (int i=0; i<PRAV_COUNT; i++)
		if (pag->Date.Day==Prav[i].Day && pag->Date.Month==Prav[i].Month)
		AddStrings((char*)&Prav[i].Text[0],pag,ID_PRAV,0,TRUE);
}