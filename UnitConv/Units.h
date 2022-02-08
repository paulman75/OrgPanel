#ifndef __UNITS_H
#define __UNITS_H

typedef struct
{
	char name[40];
	double value;
	char type;
} UNIT, *LPUNIT;

void UpdateUnitName();
void CalcResult();

#define ID_LEN                          50002
#define ID_SQ                           50003
#define ID_3D                           50004
#define ID_MASSA                        50005
#define ID_SPEED                        50006
#define ID_LIQ                          50007
#define ID_SYP                          50008
#define ID_PRESS                        50009
#define ID_POWER                        50010
#define ID_TEMP                         50011
#define ID_HEX                          50012
#define ID_FIZ                          50013
#define ID_ABOUT                        50014

#endif