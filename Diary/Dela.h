typedef struct
{
	int		ID;
	TDate	begDate;
	TDate	endDate;
	TDate	factDate;
	BOOL	bHoliday;
	char	Text[200];
	byte	Min;
	byte	Hour;
	byte	NeedAlarm;
} TDelo, *PDelo;

void AddDelo(PPage pag);
void NewDelo();
void ChangeDelo();
void DeleteDelo();
void DelOldDelo();
BOOL GetDeloAfterDate(PDate pdat, PDelo pdl, PDate dldate, BOOL* bND);
int GetDeloByID(int ID, PDelo pdl);
void DeloExec();