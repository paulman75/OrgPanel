#include "windows.h"
#include "ctimer.h"

//UINT WINAPI CTimerProc(HWND hwnd, WORD, int idTimer, DWORD);

CTimer::CTimer(int milliseconds, HWND MainWindow,byte Tag)
{
  id = 0;
  milli = milliseconds ;
  OwnerWindow=MainWindow;
  FTag=Tag;
}

CTimer::~CTimer(void)
{
  Stop();
}

BOOL CTimer::Start(void)
{
  if (id!=0) return FALSE;
  id = SetTimer(OwnerWindow, FTag, (UINT)milli, NULL);
  if (id > 0)
      return TRUE;
  else return FALSE;
}

void CTimer::Stop(void)
{
  if (id) {
    KillTimer(OwnerWindow, (UINT)id);
  }
  id = 0 ;
}

void CTimer::SetInterval(int millis)
{
	milli=millis;
}