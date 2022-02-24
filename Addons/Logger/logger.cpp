#include <stdio.h>
#include "windows.h"
#include "logger.h"

//#define LogFileName "c:/org.log"

#ifdef DEBUG

char LogFileName[100];

void InitLog(char* fn)
{
	strcpy(LogFileName,fn);
	remove(LogFileName);
}

void WriteLog(char* mess, int Param1)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	FILE *LogFile = fopen ( LogFileName , "at" );
	if (!LogFile) return;
	char ch[200];
	wsprintf(ch,"%d:%d:%d %s",st.wHour,st.wMinute,st.wSecond,mess);
//	strcpy(ch,mess);
	if (Param1!=-1) wsprintf(ch,"%s %d",ch,Param1);
	fprintf ( LogFile , "%s\n" , ch ) ;
        fclose  ( LogFile ) ;
}

#endif