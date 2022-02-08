#include "windows.h"
#include "sounds.h"
#include "Compon\MyUtils.h"
#include <mmsystem.h>
#include "res/resource.h"

extern HINSTANCE hInstance;		  // Global instance handle for application
extern char MainDir[200];

typedef struct
{
	LPCTSTR FileName;
	byte	Repeats;
} SOUNDINFO;

DWORD WINAPI SoundThread(LPVOID lpvThreadParam)
{
	SOUNDINFO* si=(SOUNDINFO*)lpvThreadParam;
	while (si->Repeats--) sndPlaySound(si->FileName,SND_SYNC); 
	free(si);
	return 0;
}

void PlayWavFile(LPCTSTR s, byte Repeats)
{
	DWORD dwThreadID;
	SOUNDINFO*	si=(SOUNDINFO*)malloc(sizeof(SOUNDINFO));
	si->FileName=s;
	si->Repeats=Repeats;
	HANDLE hThread=CreateThread(NULL,0,SoundThread,si,0,&dwThreadID);
	CloseHandle(hThread);
}

void CSoundSelect::DoPlay()
{
	int j=SendMessage(cList->hWnd,LB_GETCURSEL,0,0);
	if (j==LB_ERR) return;
	char* fp=Files[j];

    DWORD dwReturn;
    MCI_OPEN_PARMS mciOpenParms;
    MCI_PLAY_PARMS mciPlayParms;

    mciOpenParms.lpstrDeviceType = "waveaudio";
    mciOpenParms.lpstrElementName = fp;
    if (dwReturn = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, 
       (DWORD)(LPVOID) &mciOpenParms)) return;

    wDeviceID = mciOpenParms.wDeviceID;

    mciPlayParms.dwCallback = (DWORD) GetParent(cList->hWnd);
    if (dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY, 
        (DWORD)(LPVOID) &mciPlayParms))
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return ;
    }
	bPlayNow=TRUE;
	UpdateButtons();
}

void CSoundSelect::DoStop()
{
	mciSendCommand(wDeviceID, MCI_STOP, 0, NULL);
}

void CSoundSelect::OnEndPlay()
{
    mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
	bPlayNow=FALSE;
	UpdateButtons();
}

void CSoundSelect::AddToCombo(LPCTSTR s)
{
	char* fullp=(char*)malloc(lstrlen(s)+1);
	lstrcpy(fullp,s);
	char* temp=(char*)malloc(100);
	ExtractFileName(s,temp);
	int j=0;
	while (temp[j] && temp[j]!=46) j++;
	temp[j]=0;
	j=SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)temp);
	SendMessage(hCombo,CB_SETITEMDATA,j,(LPARAM)fullp);
}

void CSoundSelect::DoBrowse()
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=GetParent(hCombo);
	ofn.hInstance=hInstance;
	ofn.lpstrFilter="Звуковые файлы\0*.wav\0";
	ofn.lpstrCustomFilter=NULL;
	ofn.nFilterIndex=0;
	char* buf=(char*)malloc(300);
	buf[0]=0;
	ofn.lpstrFile=buf;
	ofn.nMaxFile=299;
	ofn.lpstrFileTitle=NULL;
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrTitle="Выбор звукового файла";
	ofn.Flags=OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt="wav";
	if (GetOpenFileName(&ofn))
	{
		int i=FindExists(buf);
		int j=SendMessage(cList->hWnd,LB_GETCURSEL,0,0);
		char* fp=Files[j];
		lstrcpy(fp,buf);
		if (i==CB_ERR) AddToCombo(buf);
		UpdateAll();
	}
	free(buf);
}

void CSoundSelect::ChangeFile()
{
	int i=SendMessage(hCombo,CB_GETCURSEL,0,0);
	int j=SendMessage(cList->hWnd,LB_GETCURSEL,0,0);
	if (j==LB_ERR) return;
	char* fc=(char*)SendMessage(hCombo,CB_GETITEMDATA,i,0);
	char* fp=Files[j];
	if (fc) lstrcpy(fp,fc);
	else fp[0]=0;
	cList->UpdateItem(j,FALSE,fp[0] ? hBitDC : hBitNoDC);
	InvalidateRect(cList->hWnd,NULL,FALSE);
	UpdateAll();
}

void CSoundSelect::UpdateButtons()
{
	int i=SendMessage(hCombo,CB_GETCURSEL,0,0);
	EnableWindow(hPlay,!bPlayNow && i && bSoundCard);
	EnableWindow(hStop,bPlayNow && bSoundCard);
}

void CSoundSelect::UpdateAll()
{
	int i=SendMessage(cList->hWnd,LB_GETCURSEL,0,0);
	if (i==LB_ERR) return;
	char* fp=Files[i];
	if (!fp) return;
	i=FindExists(fp);
	if (i==CB_ERR) i=0;
	SendMessage(hCombo,CB_SETCURSEL,i,0);
	UpdateButtons();
}

int CSoundSelect::FindExists(LPCTSTR s)
{
	int i=SendMessage(hCombo,CB_GETCOUNT,0,0);
	if (i==CB_ERR) return i;
	while (i--)
	{
		char* fp=(char*)SendMessage(hCombo,CB_GETITEMDATA,i,0);
		if (fp)
		{
			if (!lstrcmpi(fp,s)) return i;
		}
		else if (!lstrlen(s)) return i;
	}
	return CB_ERR;
}

void CSoundSelect::StartWork()
{
	int i=SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)"(Нет)");
	SendMessage(hCombo,CB_SETITEMDATA,i,0);
	char* WavDir=(char*)malloc(250);
	lstrcpy(WavDir,MainDir);
	lstrcat(WavDir,"Wavs\\*.wav");
	WIN32_FIND_DATA fd;
	HANDLE h=FindFirstFile(WavDir,&fd);
	if (h!=INVALID_HANDLE_VALUE)
	{
		while (TRUE)
		{
			lstrcpy(WavDir,MainDir);
			lstrcat(WavDir,"Wavs\\");
			lstrcat(WavDir,fd.cFileName);
			AddToCombo(WavDir);
			if (!FindNextFile(h,&fd)) break;
		}
		FindClose(h);
	}
	i=SendMessage(cList->hWnd,LB_GETCOUNT,0,0);
	if (i!=LB_ERR)
		while (i--)
		{
			char* fp=Files[i];
			if (fp)
			{
				int j=FindExists(fp);
				if (j==LB_ERR) AddToCombo(fp);
			}
		}
	free(WavDir);
	SendMessage(cList->hWnd,LB_SETCURSEL,0,0);
	UpdateAll();
}

void CSoundSelect::AddUnit(LPCTSTR Cap, LPTSTR Fil)
{
	if (Count==SOUND_CAPACITY)
	{
		MessageBox(0,"Error","Capacity in SoundSelect",0);
		return;
	}
	cList->AddItem(Cap,0,Fil[0] ? hBitDC : hBitNoDC);
	Files[Count++]=Fil;
}

CSoundSelect::CSoundSelect(HWND hList, HWND hCom, HWND hp, HWND hs)
{
	Count=0;

	hCombo=hCom;
	hPlay=hp;
	hStop=hs;
	hBit=LoadBitmap(hInstance,"SOUND");
	HDC dc=GetDC(0);
	hBitDC=CreateCompatibleDC(dc);
	SelectObject(hBitDC,hBit);
	hBitNo=LoadBitmap(hInstance,"NOSOUND");
	hBitNoDC=CreateCompatibleDC(dc);
	SelectObject(hBitNoDC,hBitNo);
	ReleaseDC(0,dc);

	cList=new CIconListBox(hList,FALSE,16);

	bSoundCard=(waveOutGetNumDevs()>0);
	bPlayNow=FALSE;
}

CSoundSelect::~CSoundSelect()
{
	int i=SendMessage(hCombo,CB_GETCOUNT,0,0);
	if (i!=CB_ERR)
		while (i--)
		{
			char* fp=(char*)SendMessage(hCombo,CB_GETITEMDATA,i,0);
			if (fp) free(fp);
		}
	delete cList;
	DeleteObject(hBitDC);
	DeleteObject(hBit);
	DeleteObject(hBitNoDC);
	DeleteObject(hBitNo);
}

LRESULT CSoundSelect::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (wParam==IDC_LIST1) return (cList->OnChildNotify(msg,wParam,lParam));
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
