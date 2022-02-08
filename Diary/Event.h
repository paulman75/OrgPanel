#include "DateTime.h"

typedef struct
{
	int		ID;
	char	Text[200];
	TDateTime	Time;
	int		NotifyMin;
	BOOL	bHoliday;
	int		Period;
} TEVENT, *PEVENT;

void AddEvent(PPage pp);
void NewEvent();
void ChangeEvent();
void DeleteEvent();
int GetEventByID(int ID, PEVENT pev);
void DelOldEvent();
BOOL GetEventAfterDate(PDate pdat, PEVENT pev, PDate evdate);
