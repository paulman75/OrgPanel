#ifndef __PRAZ_H
#define __PRAZ_H

void InitPraz();
void FreePraz();
LPTSTR FindPrim(PDate dat, int Index);
LPTSTR FindPraz(PDate dat, int Index);

void PrazEdit();

typedef struct
{
	byte Hour;
	byte Min;
	byte Second;
} TTime;

typedef struct
{
	TDate	Date;
	char	Message[100];
} TMyPraz, *PMyPraz;

#endif