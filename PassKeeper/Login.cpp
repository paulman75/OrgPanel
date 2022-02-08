#include <windows.h>
#include "Units.h"
#include "res\resource.h"
#include "Utils.h"
#include "Login.h"
#include "../Headers/general.h"

BOOL bExistsData;
BOOL bExistsPass;
extern char	buf[200];
extern char MainDir[200];
char Password[20];
extern HINSTANCE hInstance;

BOOL FAR PASCAL PassDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetWindowText(GetDlgItem(hDlg,IDC_PASS),buf,200);
			if (strlen(buf)<4)
			{
				MessageBox(hDlg,"������ ������ ���� �� ����� 4 ��������.","������",MB_ICONERROR | MB_OK);
				break;
			}
			EndDialog(hDlg,TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg,FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL InputPassword(BOOL bNewInput)
{
	if (!DialogBox(hInstance,"PASSDLG",NULL,PassDlgProc)) return FALSE;
	if (bNewInput)
	{
		strcpy(Password,buf);
		return TRUE;
	}
	BOOL b=strcmp(Password,buf)==0;
	if (!b) MessageBox(0,"������ ��������! �������� � ��������� ���!","������",MB_ICONERROR | MB_OK);
	return b;
}

void FillPass()
{
	strcat(Password,"��");
	strcat(buf,"��");
	strcat(Password,"��");
}

BOOL CheckPassword()
{
	lstrcpy(buf,MainDir);
	lstrcat(buf,PASS_FILE);
	DWORD dw;
	HANDLE hFile = CreateFile(buf,GENERIC_READ, 0,0, OPEN_EXISTING,0,NULL);
	bExistsData=(hFile!=INVALID_HANDLE_VALUE);
	if (bExistsData) CloseHandle(hFile);

	lstrcpy(buf,MainDir);
	lstrcat(buf,LOGIN_FILE);
	hFile = CreateFile(buf,GENERIC_READ, 0,0, OPEN_EXISTING,0,NULL);
	bExistsPass=FALSE;
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		ReadFile(hFile,buf,4,&dw,NULL);
		if (buf[0]=='p' && buf[1]=='s' && buf[2]=='c' && buf[3]=='1')
		{
			byte b;
			ReadFile(hFile,&b,1,&dw,NULL);
			ReadFile(hFile,Password,20,&dw,NULL);
			if (dw==20)
			{
				byte crc=Decrypt((byte*)Password,20,b);
				byte crc2;
				ReadFile(hFile,&crc2,1,&dw,NULL);
				if (crc==crc2) bExistsPass=TRUE;
			}
		}
		CloseHandle(hFile);
	}
	if (bExistsData && !bExistsPass)
	{
		//���� ������ �� ��� ������ - ���� ������ ��������� ������
		int i=MessageBox(0,"�� ������ ���� ������������. ����� ����� � ��������� �� ������ ������ ��������� ������. ����� �� ������ ������ ����� ������, ��� ���� ������ ����� ��������. ������ ��������� ������ � ���������� ������?","������",MB_ICONQUESTION | MB_YESNOCANCEL);
		if (i==IDCANCEL) return FALSE;
		if (i==IDNO)
		{
			lstrcpy(buf,MainDir);
			lstrcat(buf,PASS_FILE);
			DeleteFile(buf);
		}
		else
		{
			strcpy(Password,"�");
			strcat(Password,"��");
			FillPass();
			strcat(Password,"�");
			if (!InputPassword(FALSE)) return FALSE;
		}
		bExistsData=FALSE;
	}
	if (!bExistsData && !bExistsPass)
	{
		//��� �� ������ �� ������ - ���� ������ ������
		MessageBox(0,"���������� ������� ������ ��� ������ � ����������.","������ ������",MB_ICONQUESTION | MB_OK);
		if (!InputPassword(TRUE)) return FALSE;
		char Pass[20];
		strcpy(Pass,Password);
		MessageBox(0,"���������� ������� ��� ��� ������ ��� ������ � ����������.","������ ������",MB_ICONQUESTION | MB_OK);
		if (!InputPassword(TRUE)) return FALSE;
		if (strcmp(Password,Pass))
		{
			MessageBox(0,"������ �� ���������. ���������� ������","������",MB_ICONERROR | MB_OK);
			return FALSE;
		}
		lstrcpy(buf,MainDir);
		lstrcat(buf,LOGIN_FILE);
		HANDLE hFile = CreateFile(buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
		if (hFile!=INVALID_HANDLE_VALUE)
		{
			DWORD dw;
			buf[0]='p';
			buf[1]='s';
			buf[2]='c';
			buf[3]='1';
			WriteFile(hFile,buf,4,&dw,NULL);
			SYSTEMTIME st;
			GetLocalTime(&st);
			byte b=(byte)st.wMilliseconds;
			WriteFile(hFile,&b,1,&dw,NULL);
			byte crc=Crypt((byte*)Password,20,b);
			WriteFile(hFile,Password,20,&dw,NULL);
			WriteFile(hFile,&crc,1,&dw,NULL);
			CloseHandle(hFile);
		}
	}
	if (bExistsData && bExistsPass)
	{
		//���� ��� - ���� ������ ������
		if (!InputPassword(FALSE)) return FALSE;
	}
	return TRUE;
}