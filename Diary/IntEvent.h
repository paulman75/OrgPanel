#include "DateTime.h"

typedef struct
{
	TDateTime	Time;
	TDate		RealDate;
	int		type;
	int		ID2;
} TINTEVENT, *PINTEVENT;

void DelIntEvent(int type, int ID2);
void UpdateIntEvent();
void OnIconDblClick();
void ShowSvod();