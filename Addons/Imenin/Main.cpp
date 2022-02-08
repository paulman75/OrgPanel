#include "windows.h"
#include "res/resource.h"
#include "dialogs.h"

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow);

HINSTANCE hInstance;		  // Global instance handle for application
HWND hDialog;

typedef struct
{
     byte Month;
     byte Day;
} TDATA,*PDATA;

typedef struct
{
	char Name[20];
	byte ICount;
    TDATA Imen[30];
} TUNIT, *PUNIT;

PUNIT pu;
char temp[100];
char MainDir[200];
HWND hList,hList2;
PUNIT ActiveUnit;
TDATA Dat;
	DWORD dw;

char* Mon[]={"Январь","Февраль","Март","Апрель","Май","Июнь","Июль","Август","Сентябрь","Октябрь","Ноябрь","Декабрь"};
void OnSelChange();

void  OnLoad()
{
	char* temp=(char*)malloc(220);
	lstrcpy(temp,MainDir);
	lstrcat(temp,"imen.dat");
	HANDLE hFile=CreateFile(temp,GENERIC_READ, 0,0, OPEN_EXISTING,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		while (TRUE) 
		{
			byte len;
			ReadFile(hFile,&len,1,&dw,NULL);
			if (!len) break;
			pu=(PUNIT)malloc(sizeof(TUNIT));
			ReadFile(hFile,&pu->Name,len,&dw,NULL);
			pu->Name[len]=0;
			ReadFile(hFile,&pu->ICount,1,&dw,NULL);
			ReadFile(hFile,&pu->Imen,sizeof(TDATA)*pu->ICount,&dw,NULL);
			for (int k=0; k<pu->ICount; k++)
			for (int n=0; n<pu->ICount-1; n++)
			if ((pu->Imen[n].Month>pu->Imen[n+1].Month) ||
			(pu->Imen[n].Month==pu->Imen[n+1].Month &&
			pu->Imen[n].Day>pu->Imen[n+1].Day))
			{
			TDATA d;
				d=pu->Imen[n];
				pu->Imen[n]=pu->Imen[n+1];
				pu->Imen[n+1]=d;
			}
			int i=SendMessage(hList,LB_ADDSTRING,0,(LPARAM)pu->Name);
			SendMessage(hList,LB_SETITEMDATA,i,(LPARAM)pu);
		}
		CloseHandle(hFile);
	}
	free(temp);
}

void  OnSave()
{
	char* temp=(char*)malloc(220);
	lstrcpy(temp,MainDir);
	lstrcat(temp,"imen.dat");
	HANDLE hFile=CreateFile(temp,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		int j=SendMessage(hList,LB_GETCOUNT,0,0);
		for (int i=0; i<j; i++)
		{
			pu=(PUNIT)SendMessage(hList,LB_GETITEMDATA,i,0);
			byte len=lstrlen(pu->Name);
			WriteFile(hFile,&len,1,&dw,NULL);
			WriteFile(hFile,&pu->Name,len,&dw,NULL);
			WriteFile(hFile,&pu->ICount,1,&dw,NULL);
			WriteFile(hFile,&pu->Imen,sizeof(TDATA)*pu->ICount,&dw,NULL);
		}
		j=0;
		WriteFile(hFile,&j,1,&dw,NULL);
		CloseHandle(hFile);
	}
	free(temp);
}

void OnInit(HWND hWnd)
{
	for (int i=1; i<32; i++)
	{
		wsprintf(temp,"%d",i);
		SendMessage(GetDlgItem(hWnd,IDC_COMBO2),CB_ADDSTRING,0,(LPARAM)temp);
	}
	for (i=0; i<12; i++)
	{
		SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_ADDSTRING,0,(LPARAM)Mon[i]);
	}
}

BOOL FAR PASCAL DataDialogWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		OnInit(hDlg);
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam)==BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				EndDialog(hDlg,FALSE);
				return TRUE;
			case IDOK:
				Dat.Day=SendMessage(GetDlgItem(hDlg,IDC_COMBO2),CB_GETCURSEL,0,0)+1;
				Dat.Month=SendMessage(GetDlgItem(hDlg,IDC_COMBO1),CB_GETCURSEL,0,0)+1;
				EndDialog(hDlg,TRUE);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

void NewData()
{
	if (!DialogBox(hInstance, "DATA", hDialog, (DLGPROC)DataDialogWndProc)) return;
	ActiveUnit->Imen[ActiveUnit->ICount++]=Dat;
	OnSelChange();
}

void NewUnit()
{
	temp[0]=0;
	if (!InputQuery(hDialog,"Ввод имени",NULL,temp)) return;
	pu=(PUNIT)malloc(sizeof(TUNIT));
	lstrcpy(pu->Name,temp);
	pu->ICount=0;
	int i=SendMessage(hList,LB_ADDSTRING,0,(LPARAM)pu->Name);
	SendMessage(hList,LB_SETITEMDATA,i,(LPARAM)pu);
}

void OnSelChange()
{
	int i;
	while (TRUE)
	{
		i=SendMessage(hList2,LB_GETCOUNT,0,0);
		if (!i) break;
		while (i) SendMessage(hList2,LB_DELETESTRING,--i,0);
	}
	i=SendMessage(hList,LB_GETCURSEL,0,0);
	ActiveUnit=(PUNIT)SendMessage(hList,LB_GETITEMDATA,i,0);
	i=0;
	while (i<ActiveUnit->ICount)
	{
		wsprintf(temp,"%d %s",ActiveUnit->Imen[i].Day,Mon[ActiveUnit->Imen[i].Month-1]);
		SendMessage(hList2,LB_ADDSTRING,0,(LPARAM)temp);
		i++;
	}
}

BOOL FAR PASCAL DialogWndProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		hDialog=hDlg;
		hList=GetDlgItem(hDialog,IDC_LIST1);
		hList2=GetDlgItem(hDialog,IDC_LIST2);
		OnLoad();
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam)==BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				EndDialog(hDlg,FALSE);
				return TRUE;
			case IDOK:
				OnSave();
				EndDialog(hDlg,TRUE);
				return TRUE;
			case IDC_BUTTON1:
				NewUnit();
				break;
			case IDC_BUTTON3:
				NewData();
				break;
				case IDC_BUTTON4:
				ActiveUnit->ICount--;
				OnSelChange();
				break;
			}
		}
		if (HIWORD(wParam)==LBN_SELCHANGE && LOWORD(wParam)==IDC_LIST1)
		OnSelChange();
		break;
	}
	return FALSE;
}

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow)
{
	GetModuleFileName(hInstance, MainDir, sizeof(MainDir));
	byte i=sizeof(MainDir);
	while ((i>0) && (MainDir[i]!=92))
	{
		i--;
	}
	MainDir[++i]=0;
	DialogBox(hInstance, "MAIN", 0, (DLGPROC)DialogWndProc);
    return(0);
}
