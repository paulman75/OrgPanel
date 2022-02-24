#include "windows.h"
#include <time.h>
#include <winsock.h>
#include "../Headers/general.h"
#include "res/resource.h"
#include "Main.h"
#include "../Addons/Logger/logger.h"

HWND g_hDlg;
extern HINSTANCE hInstance;
const int SERVER_SIZE = 40;
char Server[80][SERVER_SIZE];
int CurrentServer;
int FirstReqServer;
int ServerCount;
DWORD IP;
char hostentBuf[MAXGETHOSTSTRUCT];
SOCKET s;
int Progress;
LONGLONG g_TimeoutTime;
DWORD Latency=0;
LONGLONG Corr;
LONGLONG g_LocalTimeOffset = 0;
HANDLE hDNS;
BOOL bAutoShow;
int Port;
BOOL bTime;
extern HWND hOKBut;

LONGLONG GetMillisecondsSince1900()
{
	FILETIME ft;

	GetSystemTimeAsFileTime(&ft);
	return *((LONGLONG*)&ft) / 10000 - (LONGLONG)9435484800000;
}

void MillisecondsToText(LONGLONG ms, char* s)
{
	LONGLONG base, unityBase = 10;
	char s2[2];
	char cUnits;

	if (ms >= 0)
	{
		lstrcpy(s, "+");
	}
	else if (ms < 0)
	{
		ms = -ms;
		lstrcpy(s, "-");
	}

	if (ms < 1000 * 60 * 2)
	{
		cUnits = 'с';
		ms /= 10;
		unityBase = 10;
	}
	else if (ms < 1000 * 60 * 60 * 2)
	{
		cUnits = 'м';
		ms /= 60;
		unityBase = 100;
	}
	else if (ms < 1000 * 60 * 60 * 24 * 2)
	{
		cUnits = 'ч';
		ms /= 60 * 60;
		unityBase = 100;
	}
	else 
	{
		cUnits = 'д';
		ms /= 60 * 60 * 24;
		unityBase = 100;
	}
	ms += 5;
	ms /= 10;

	if (!ms)
	{
		*s = ' ';
	}
	base = unityBase * 10;
	while (base <= ms) base *= 10;

	s2[1] = 0;
	while (base > 1)
	{
		base /= 10;
		s2[0] = (char)(ms / base);
		ms -= s2[0] * base;
		s2[0] += '0';
		lstrcat(s, s2);
		if (base == unityBase)
		lstrcat(s, ".");
	}
	s2[0] = cUnits;
	lstrcat(s, s2);
}

void FormatMillisecondsSince1900(DWORDLONG time, char* s)
{
	FILETIME ft, ftLocal;
	SYSTEMTIME st;

	*((DWORDLONG *)&ft) = 10000 * (time + DWORDLONG(9435484800000));

	FileTimeToLocalFileTime(&ft, &ftLocal);

	FileTimeToSystemTime(&ftLocal, &st);
	wsprintf(s, "%u:%02u:%02u   %u.%u.%u ",
		st.wHour, st.wMinute, st.wSecond, 
		st.wDay, st.wMonth, st.wYear);
}

LONGLONG OffsetMillisecondsSince1900(LONGLONG correction)
{
	FILETIME ft;
	SYSTEMTIME st;
	LONGLONG time;

	time = GetMillisecondsSince1900() + correction;

	*((LONGLONG *)&ft) = 10000 * (time + LONGLONG(9435484800000));

	FileTimeToSystemTime(&ft, &st);
	if (!SetSystemTime(&st))
	{
		time = 0;
	}
	if (!time)
	{
		AddTextToRichEdit(ID_RICH,"Невозможно установить системное время\r\n");
	}
	else
	{
		AddTextToRichEdit(ID_RICH,"Системное время успешно скорректировано\r\n");
		char* s1=(char*)malloc(200);
		char* s2=(char*)malloc(200);
		char* s3=(char*)malloc(200);

		FormatMillisecondsSince1900(time, s2);
		WriteBinToReg("LastTimeBin",(LPBYTE)&time,sizeof(time));
		strcpy_s(s1,200,"Сейчас: ");
		strcat_s(s1,200,s2);
		MillisecondsToText(Corr, s3);
		strcat_s(s1,200," Корректировка: ");
		strcat_s(s1,200,s3);
		AddTextToRichEdit(ID_RICH,s1);
		wsprintf(s1,"%s на %s",s2,s3);
		WriteStringToReg("LastTimeUpdate",s1);
		free(s1);
		free(s2);
		free(s3);
	}
	return time;
}

BOOL DoSystemTimeCorrection()
{
	LONGLONG correctionTime	= OffsetMillisecondsSince1900(Corr);

	SendMessage(HWND_TOPMOST, WM_TIMECHANGE, 0, 0);

	return TRUE;
}

void OnEndTimeRequest(BOOL bOK)
{
#ifdef DEBUG
	WriteLog("OnEndTimeRequest OK=",bOK);
#endif

	if (bOK)
	{
		char* t=(char*)malloc(100);
		wsprintf(t,"Задержка: %d мс\r\n",Latency);
		AddTextToRichEdit(ID_RICH,t);
		free(t);
		WriteDWordToReg("TimeServer",CurrentServer);
		DoSystemTimeCorrection();
		SetWindowText(hOKBut,"OK");
		if (!bAutoShow) DestroyWindow(g_hDlg);
#ifdef DEBUG
	WriteLog("Удалили окно");
#endif
	}
	else
	{
		CurrentServer++;
		if (CurrentServer>=ServerCount) CurrentServer=0;
#ifdef DEBUG
	WriteLog("Переходим к следующему серверу ", CurrentServer);
#endif
		if (CurrentServer==FirstReqServer)
		{
			AddTextToRichEdit(ID_RICH,"Не удалось синхронизировать время\r\n");
#ifdef DEBUG
	WriteLog("Не удалось синхронизировать bAutoShow=",bAutoShow);
#endif
			SetWindowText(hOKBut,"OK");
			if (!bAutoShow) DestroyWindow(g_hDlg);
		}
		else ServerStart(Server[CurrentServer]);
	}
}

void ServerTimeRead(int err)
{
	if (!g_hDlg) return;
#ifdef DEBUG
	WriteLog("ServerTimeRead err=",err);
#endif
	DWORD temp;
	LONGLONG correction;

	if (err)
	{
		ServerStop(eStatusNotResponding);
	}
	else
	{
		err = recv(s, (char FAR *)&temp, 4, 0);
		if (err != 4)
		{
			ServerStop(eStatusNotResponding);
			return;
		}

		correction = ntohl(temp) * (LONGLONG)1000;
		correction -= GetMillisecondsSince1900();
		Latency = GetTickCount() - Latency;
		Corr = 
			correction + 
			(Latency >> 1) +
			g_LocalTimeOffset;
		if (Latency>2000) ServerStop(eStatusSlowResponding);
		else ServerStop(eStatusUsed);
	}
}

void MakeTimeStart()
{
	ServerStart(Server[CurrentServer]);
}

void AddServer(char* name)
{
	strcpy_s(Server[ServerCount++], SERVER_SIZE,name);
}

BOOL __stdcall InetTimeSync(HWND hwnd, BOOL bShow)
{
    WSADATA WSAData;
	bAutoShow=bShow;
	if (WSAStartup(0x101, &WSAData)) return TRUE;

	ServerCount=0;

    AddServer("time.symmetricom.com");
	AddServer("ntp.time.in.ua");
	AddServer("ntp.mobatime.ru");
	AddServer("nist1.symmetricom.com");
	AddServer("ntp1.symmetricom.com");
	AddServer("ntp2.symmetricom.com");
	AddServer("ntp3.symmetricom.com");
	AddServer("clock.isc.org");
	AddServer("clock.nc.fukuoka-u.ac.jp");
	AddServer("fartein.ifi.uio.no");
	AddServer("ntp1.strath.ac.uk");
	AddServer("swisstime.ethz.ch");
	AddServer("time1.stupi.se");
	AddServer("timex.cs.columbia.edu");
	AddServer("otc1.psu.edu");
	AddServer("bitsy.mit.edu");
    AddServer("canon.inria.fr");
    AddServer("chronos.univ-rennes1.fr");
    AddServer("clock.llnl.gov");
    AddServer("lerc-dns.lerc.nasa.gov");
    AddServer("norad.arc.nasa.gov");
    AddServer("ntp1.sony.com");
    AddServer("ntps1-0.cs.tu-berlin.de");
    AddServer("ntps1-1.cs.tu-berlin.de");
    AddServer("ntps1-1.uni-erlangen.de");
    AddServer("otc1.psu.edu");
    AddServer("rackety.udel.edu");
    AddServer("tick.usno.navy.mil");
    AddServer("time-A.timefreq.bldrdoc.gov");
    AddServer("time.nist.gov");
    AddServer("time.service.uit.no");
    AddServer("tock.usno.navy.mil");
    AddServer("wwvb.erg.sri.com");
    AddServer("wwvb.isi.edu");
    AddServer("augean.eleceng.adelaide.edu.au");
    AddServer("biofiz.mf.uni-lj.si");
    AddServer("black-ice.cc.vt.edu");
    AddServer("chime1.surfnet.nl");
    AddServer("clock.psu.edu");
    AddServer("clock1.unc.edu");
    AddServer("clock-1.cs.cmu.edu");
    AddServer("clock-2.cs.cmu.edu");
    AddServer("delphi.cs.ucla.edu");
    AddServer("esavax.esa.lanl.gov");
    AddServer("fartein.ifi.uio.no");
    AddServer("gazette.bcm.tmc.edu");
    AddServer("gilbreth.ecn.purdue.edu");
    AddServer("harbor.ecn.purdue.edu");
    AddServer("jane.jpl.nasa.gov");
    AddServer("kuhub.cc.ukans.edu");
    AddServer("lane.cc.ukans.edu");
    AddServer("louie.udel.edu");
    AddServer("molecule.ecn.purdue.edu");
    AddServer("noc.near.net");
    AddServer("ns.unet.umn.edu");
    AddServer("nss.unet.umn.edu");
    AddServer("ntp.ctr.columbia.edu");
    AddServer("ntp0.pipex.net");
    AddServer("ntp0.strath.ac.uk");
    AddServer("ntp1.pipex.net");
    AddServer("ntp2.pipex.net");
    AddServer("ntp2.sura.net");
    AddServer("ntp-2.mcs.anl.gov");
    AddServer("ntp0.cornell.edu");
    AddServer("rolex.peachnet.edu");
    AddServer("salmon.maths.tcd.ie");
    AddServer("timelord.cs.uregina.ca");
    AddServer("ticktock.wang.com");
    AddServer("timex.cs.columbia.edu");
    AddServer("timex.peachnet.edu");
    AddServer("tmc.edu");
    AddServer("tycho.usno.navy.mil");
    AddServer("wuarchive.wustl.edu");
	Progress=eProgressNot;
	CurrentServer=0;
	Port=37;
	ReadRegistry("TimeServer",(LPBYTE)&CurrentServer,sizeof(int));
	if (CurrentServer>=ServerCount) CurrentServer=0;
	FirstReqServer=CurrentServer;
	bTime=TRUE;
	return InitUpdate(hwnd);
}
