typedef struct
{
	TDate Date;
	byte Type; //1-��������, 0-�������
} TISKDAY;

DWORD GetDayColor(TDate *date);
void ChangeIskDay(TDate *date);
