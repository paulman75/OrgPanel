#include <windows.h>
#include "res/resource.h"
#include "Units.h"
#include <stdio.h>

extern HWND hMainDlg;
extern int	CurrentIndex;
extern BOOL bFiz;

const char uCount[12]={42,11,17,29,9,36,23,10,6,4,4,24};
const char uInit[12]={21,5,9,2    ,4,21,11,7,0,3,2,0};

const UNIT Units[]={
//�����
	{"��������",1e10,1},
	{"�����",1.406074, 1},
	{"��������������� �������",6.684587e-12, 1},
	{"���������",117.6470588, 1},
	{"������",0.0009373828, 1},
	{"������",22.49719, 1},
	{"����",39.37007874, 1},
//	{"��������� ���",0.004555808656, 1},
	{"���������",0.004556722, 1},
	{"������",3.94E3, 1},
	{"��������",0.001, 1},
	{"�����",2.086463, 1},
	{"����� ������",0.4032258, 1},
	{"������",9.84252, 1},
	{"���� �������� US",0.0002071237, 1},
	{"���� �������",0.0001798561151, 1},
	{"�����",472.4409, 1},
	{"����(�������������)",4.97, 1},
	{"����(������������)",3.333333333, 1},
	{"����� ���������",0.005399568, 1},
	{"������� ������",0.5681818, 1},
	{"������� ������",0.0004686914, 1},
	{"����",1, 1},
	{"������",1E6, 1},
	{"���� �������� US",0.0006213712, 1},
	{"���� �������",0.0005399568, 1},
	{"���",39400, 1},
	{"���������",1000, 1},
	{"������� ������",0.5468067, 1},
	{"������",3.240779e-17,1},
	{"����� (��������.)",2834.646, 1},
	{"����",5.624297, 1},
	{"������",0.4686914, 1},
	{"���������",100, 1},
	{"�������� ������",5.559402e-11, 1},
	{"�������� �������",3.335641e-9, 1},
	{"�����",2849.002849, 1},
	{"�������� ���",1.057023e-16, 1},
	{"�������",0.00497097, 1},
	{"���",3.280839895, 1},
	{"������ (��������.)",236.2205, 1},
	{"����",0.0497097, 1},
	{"���",1.093613298, 1},
//�������
	{"���",2.469135802e-004, 1},
	{"��",0.0109, 1},
	{"������", 0.000109,1},
	{"���������� ����", 1550.387597,1},
	{"���������� ��������", 1.e-006,1},
	{"���������� ����", 1,1},
	{"���������� ���������", 1000000,1},
	{"���������� ���", 1.196172249,1},
	{"���", 9.900990099e-004,1},
	{"������� (���)", 1.072501073e-008,1},
	{"����� (������)", 3.086419753e-006,1},
//�����
	{"������� �������� (���)", 6.29,1},
	{"������� ����� (���)", 8.65,1},
	{"������ (������)", 27.5,1},
	{"������ (���)", 28.4,1},
	{"�����", 81.3,1},
	{"������ (������)", 220,1},
	{"���� �������", 0.2759381898,1},
	{"���� �����", 0.2802690583,1},
	{"���������� ����", 61012.81269,1},
	{"���������� ����", 1,1},
	{"����� (������)", 1.76e3,1},
	{"����� ���������� (���)", 2.11e3,1},
	{"����� ����� US", 1.82e3,1},
	{"����", 0.3289473684,1},
	{"����� �����������", 0.3533568905,1},
	{"����� ���������� (������)", 3.52e4,1},
	{"����� ���������� (���)", 3.38e4,1},
//�����
	{"�����", 3571428.571,1},
	{"���", 9.842519685e-006,1},
	{"�����", 1,1},
	{"����", 15.43209877,1},
	{"����", 7407.407407,1},
	{"������", 0.5643340858,1},
	{"��������", 0.2344253,1},
	{"�����", 5,1},
	{"����� (������)", 4.878049,1},
	{"������� �������", 7.874015748e-005,1},
	{"������� ��������", 8.818342152e-005,1},
	{"�������� ����������", 6.329113924e-004,1},
	{"������� (������)", 1.96842334e-005,1},
	{"������� (���)", 2.204634141e-005,1},
	{"���������", 1.e-003,1},
	{"���� (������)", 3.149606299e-004,1},
	{"���", 0.07814178,1},
	{"����", 308.6419753,1},
	{"����������", 1000,1},
	{"���������", 0.6430868167,1},
	{"������", 148148.1481,1},
	{"���",0.00006104819755,1},
	{"�����", 0.000157473, 1},
	{"��� (������)", 7.874015748e-005,1},
	{"�����", 1.e-006,1},
	{"����� (���)", 0.03527396,1},
	{"�����", 3.215434084e-002,1},
	{"���� (���)", 0.002204623,1},
	{"����", 2.679528403e-003,1},

//��������
	{"�������� � ������", 0.06,1},
	{"�������� � �������", 0.001,1},
	{"�������� � ���", 3.6,1},
	{"���� � ������", 60,1},
	{"���� � �������", 1,1},
	{"���� � ���", 3.6E3,1},
	{"���� � �������", 0.0006213712,1},
	{"���� � ���", 2.236936,1},
	{"����", 1.943845,1},
//��������
	{"������", 0.03829657,1},
	{"������� ��� ��������� (������)", 6.112469438e-003,1},
	{"������� ��� ��������� (���)", 8.389261745e-003,1},
	{"������� ��� ����� ����� (������)", 6.289782877e-003,1},
	{"������� ��� ����� ����� (���)", 7.195797654e-003,1},
	{"�������", 7.112375533e-003,1},
	{"���", 2.036784325e-003,1},
	{"�����", 0.08130081,1},
	{"������ (������)", 0.2199736032,1},
	{"������ (���)", 0.2642007926,1},
	{"���������", 10,1},
	{"��������", 100,1},
	{"�����, ���� (������)", 7.042253521,1},
	{"�����, ���� (���)", 8.474576271,1},
	{"������ ������ (������)", 281.6901408,1},
	{"������ ������ (���)", 337.8378378,1},
	{"������ ����� (������)", 35.21126761,1},
	{"������ ����� (���)", 33.81805884,1},
	{"������ (������)", 0.8771929825,1},
	{"������ (���)", 1.057082452,1},
	{"���������", 1.222493888e-002,1},
	{"����", 1,1},
	{"���������", 1000,1},
	{"�����", 16230730,1},
	{"����", 2.094986697e-003,1},
	{"����� (������)", 1.754385965,1},
	{"����� (���)", 2.127659574,1},
	{"����", 0.4405286344,1},
	{"�����", 17.6056338,1},
	{"�������� �����", 70.42253521,1},
	{"������", 2.754820937e-002,1},
	{"������", 4.189885616e-003,1},
	{"������ �����", 227.2727273,1},
	{"�����", 8.130081,1},
	{"������", 16.26016,1},
	{"����(������)", 0.8130081,1},
//���� ������� ���
	{"������� (������)", 6.112469438e-003,1},
	{"������� (���)", 8.52514919e-003,1},
	{"������ (������)", 2.751031637e-002,1},
	{"������ (���)", 2.840909091e-002,1},
	{"�����", 0.08130081,1},
	{"������ (������)", 0.2199736032,1},
	{"������ (���)", 0.2642007926,1},
	{"������", 0.3048966,1},
	{"������ (������)", 0.8771929825,1},
	{"������ (���)", 0.9082652134,1},
	{"�������", 3.436426117e-003,1},
	{"����", 1,1},
	{"��� (������)", 0.113507378,1},
	{"��� (���)", 0.1298701299,1},
	{"����� (������)", 1.76056338,1},
	{"����� (���)", 1.814882033,1},
	{"��� (������)", 9.165902841e-003,1},
	{"������", 4.226753,1},
	{"�������� �����", 67.62804,1},
	{"������ (������)", 1.374948439e-002,1},
	{"������ �����", 202.8841,1},
	{"������� (������)", 7.886435331e-004,1},
	{"��������", 0.03811208,1},
//��������
	{"��������� (���)", 0.001359506,1},
	{"��������� (���)", 0.001315786,1},
	{"���", 0.00133322,1},
	{"���� �������� ������", 0.5352534,1},
	{"���������� ���� �� ��. ���������", 1.333333e-003,1},
	{"������������", 0.1333333,1},
	{"��������", 1.33322,1},
	{"��������� �������� ������", 1,1},
	{"�������", 133.322,1},
	{"���� ���� �� ��. ����", 0.01933672,1},
//��������
	{"����", 1,1},
	{"�������� � �������", 0.2388459,1},
	{"�������� ��������� ����", 0.0001019589,1},
	{"��������� ����", 0.001341022,1},
	{"����������� ��������� ����", 0.00135962,1},
	{"������������� ��������� ����", 0.00134048,1},
//�����������
	{"������ ����������", 0,3},
	{"������ ��������", 273,2},
	{"������ �������", 0,4},
	{"������ �������", 1,1},
//����.���������
	{"��������", 2,5},
	{"�����������", 8,5},
	{"����������", 10,5},
	{"�����������������", 16,5},
//���������
	{"������� ������� �����:��", 1.660565586E-27,1},
	{"�������������� ����������:�*�^2/(��^2)", 6.672041E-11,1},
	{"����� ���������:��", 1.602189246E-19,1},
	{"������������ ������ ���������:�", 2.81793807E-15,1},
	{"�������� ����:��/��", 9.274078E-24,1},
	{"��������� ����������:��/�", 1.2566370614E-6,1},
	{"����� ����� ��������:�.�.�.", 1.007825036,1},
	{"����� ����� ��������:�.�.�.", 2.014101795,1},
	{"����� ����� ���������:��", 9.10953447E-31,1},
	{"����� ����� �������:��", 1.672648586E-27,1},
	{"����� ����� ��������:��", 1.6749543E-27,1},
	{"�������� ������� ����������:��/(����*�)", 8.31441,1},
	{"���������� ��������:1/����", 6.02204531E23,1},
	{"���������� ���������:��/�", 1.38066244E-23,1},
	{"���������� ������:��*�", 6.626176E-34,1},
	{"���������� ��������:1/�", 10973731.77,1},
	{"���������� �������:��/����", 96484.56,1},
	{"�������� ����� � �������:�/�", 299792458,1},
	{"�������� ����� ���������:��/��", 175880470000,1},
	{"��������� ���������� �������:�/�^2", 9.80665,1},
	{"������������� ����������:�/�", 8.85418781871E-12,1},
	{"������� ����� ���������:���", 0.511003414,1},
	{"������� ����� �������:���", 938.279627,1},
	{"������� ����� ��������:���", 939.5731,1}
};

void UpdateUnitName()
{
	char i=CurrentIndex-ID_LEN;
	int ind=0;
	char* buf[50];
	for (char j=0; j<i; j++) ind+=uCount[j];
	SendMessage(GetDlgItem(hMainDlg,IDC_COMBO1),CB_RESETCONTENT,0,0);
	SendMessage(GetDlgItem(hMainDlg,IDC_COMBO2),CB_RESETCONTENT,0,0);
	for (int j=0; j<uCount[i]; j++)
	{
		strcpy((char*)buf,Units[ind+j].name);
		char* ts=strstr((char*)buf,":");
		if (ts) *ts=0;
		SendMessage(GetDlgItem(hMainDlg,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)buf);
		SendMessage(GetDlgItem(hMainDlg,IDC_COMBO2),CB_ADDSTRING,0,(LPARAM)buf);
	}
	SendMessage(GetDlgItem(hMainDlg,IDC_COMBO1),CB_SETCURSEL,uInit[i],0);
	SendMessage(GetDlgItem(hMainDlg,IDC_COMBO2),CB_SETCURSEL,uInit[i],0);
	CalcResult();
}

void IntToBin(char* buf, int value)
{
	int d=1;
	while (value>=d) d=d<<1;
	if (d!=1) d=d>>1;
	buf[0]=0;
	while (d)
	{
		if ((value&d)==d) strcat(buf,"1");
		else strcat(buf,"0");
		d=d>>1;
	}
}

int BinToInt(char* buf)
{
	int res=0;
	int d=1;
	int i=strlen(buf)-1;
	while (i>=0)
	{
		if (buf[i]=='1') res+=d;
		else if (buf[i]!='0') return 0;
		i--;
		d=d<<1;
	}
	return res;
}

int OctToInt(char* buf)
{
	int res=0;
	int d=1;
	int i=strlen(buf)-1;
	while (i>=0)
	{
		if (buf[i]<'0' || buf[i]>'7') return 0;
		res+=d*(buf[i]-'0');
		d*=8;
		i--;
	}
	return res;
}

int HexToInt(char* buf)
{
	int res=0;
	int d=1;
	CharUpper(buf);
	int i=strlen(buf)-1;
	while (i>=0)
	{
		if (buf[i]>='0' && buf[i]<='9') res+=d*(buf[i]-'0');
		else if (buf[i]>='A' && buf[i]<='F') res+=d*(buf[i]-'A'+10);
		else return 0;
		d*=16;
		i--;
	}
	return res;
}

void CalcResult()
{
	char i=CurrentIndex-ID_LEN;
	int ind=0;
	for (char j=0; j<i; j++) ind+=uCount[j];
	int i1,i2;
	i1=SendMessage(GetDlgItem(hMainDlg,IDC_COMBO1),CB_GETCURSEL,0,0);
	i2=SendMessage(GetDlgItem(hMainDlg,IDC_COMBO2),CB_GETCURSEL,0,0);
	LPUNIT u1,u2;
	u1=(LPUNIT)&Units[ind+i1];
	u2=(LPUNIT)&Units[ind+i2];
	char buf[100];
	if (bFiz)
	{
		sprintf(buf,"%g",u1->value);
		strcat(buf," ");
		char* ts=strstr(u1->name,":");
		if (ts) strcat(buf,ts+1);
		SendMessage(GetDlgItem(hMainDlg,IDC_EDIT1),WM_SETTEXT,0,(LPARAM)buf);
		return;
	}
	SendMessage(GetDlgItem(hMainDlg,IDC_EDIT1),WM_GETTEXT,100,(LPARAM)buf);
	if (strlen(buf))
	{
		double l;
		if (u1->type==5)
		{
			switch ((int)u1->value) // ��������������� ������ �
			{						// �����
			case 2:
				l=BinToInt(buf);
				break;
			case 8:
				l=OctToInt(buf);
				break;
			case 10:
				l=strtod(buf,NULL);
				break;
			case 16:
				l=HexToInt(buf);
				break;
			}
			switch ((int)u2->value) //����������� ����� � ������
			{
			case 2:
				IntToBin(buf,(int)l);
				break;
			case 8:
				sprintf(buf,"%o",(int)l);
				break;
			case 10:
				sprintf(buf,"%g",l);
				break;
			case 16:
				sprintf(buf,"%x",(int)l);
				CharUpper(buf);
				break;
			}
		}
		else
		{
			l=strtod(buf,NULL);
			switch (u1->type)
			{
			case 1:
				l=l/u1->value;
				break;
			case 2:
				l=l-u1->value;
				break;
			case 3:
				l=(l-32)*5/9;
				break;
			case 4:
				l=l*5/4;
				break;
			}
			switch (u2->type)
			{
			case 1:
				l=l*u2->value;
				break;
			case 2:
				l=l+u2->value;
				break;
			case 3:
				l=l*9/5+32;
				break;
			case 4:
				l=l*4/5;
				break;
			}
			sprintf(buf,"%g",l);
		}
	}
	SendMessage(GetDlgItem(hMainDlg,IDC_EDIT2),WM_SETTEXT,0,(LPARAM)buf);
}