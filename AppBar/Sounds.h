#ifndef __SOUNDS_H
#define __SOUNDS_H

void PlayWavFile(LPCTSTR s, byte Repeats);

#include "../Headers/IconList.h"

#define SOUND_CAPACITY	5

class CSoundSelect
{
private:
    UINT	wDeviceID;
	BOOL	bPlayNow;
	BOOL	bSoundCard;
	BOOL	FindExists(LPCTSTR s);
	CIconListBox*	cList;
	HWND		hCombo;
	HWND		hPlay,hStop;
	HBITMAP	hBit;
	HDC		hBitDC;
	HBITMAP hBitNo;
	HDC		hBitNoDC;
	char*	Files[SOUND_CAPACITY];
	byte	Count;
	void	UpdateButtons();
	void	AddToCombo(LPCTSTR);
public:
	LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void AddUnit(LPCTSTR, LPTSTR);
	void StartWork();
	void UpdateAll();
	void ChangeFile();
	void DoBrowse();
	void DoPlay();
	void DoStop();
	void OnEndPlay();
	
	CSoundSelect(HWND, HWND, HWND, HWND);
	~CSoundSelect();
};

#endif
