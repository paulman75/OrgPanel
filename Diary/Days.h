typedef struct
{
	TDate Date;
	byte Type; //1-выходной, 0-рабочий
} TISKDAY;

DWORD GetDayColor(TDate *date);
void ChangeIskDay(TDate *date);
