#include "windows.h"
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "praz.h"
#include "res/resource.h"
#include "../Headers/DynList.h"

extern HINSTANCE hInstance;		  // Global instance handle for application

HANDLE hPrim,hPraz;
LPBYTE lbPrim,lbPraz;
DynArray<TMyPraz> MyPrazList;
extern char	DataDir[200];
extern HWND hwndMain;
HWND hPrazDlg;
HWND hPrazList;
BOOL bPrazModified;

TMyPraz EditPraz;

#define DNEV_PRAZ_FILE "dnevpraz.dat"

LPTSTR FindPr(LPBYTE lb,PDate dat, int Index, int* cou)
{
	int Ind=0;
	byte i=0;
	while (*lb)
	{
		if (*lb==dat->Day && *(lb+1)==dat->Month) 
		{
			if (Ind==Index)	return (LPTSTR)(lb+2);
			Ind++;
		}
		lb+=2;
		while (*lb) lb++;
		lb++;
	}
	*cou=Ind;
	return NULL;
}

LPTSTR FindPrim(PDate dat, int Index)
{
	int cou;
	return FindPr(lbPrim,dat,Index,&cou);
}

LPTSTR FindPraz(PDate dat, int Index)
{
	int cou;
	LPTSTR lps=FindPr(lbPraz,dat, Index, &cou);
	if (lps) return lps;
	int i=0;
	while (i<MyPrazList.count())
	{
		TMyPraz pr=MyPrazList[i];
		if (dat->Month==pr.Date.Month && dat->Day==pr.Date.Day)
		{
			if (cou==Index) return MyPrazList[i].Message;
			cou++;
		}
		i++;
	}
	return NULL;
}

void LoadMyPraz()
{
	char* temp=(char*)malloc(220);
	lstrcpy(temp,DataDir);
	lstrcat(temp,DNEV_PRAZ_FILE);
	HANDLE hFile=CreateFile(temp,GENERIC_READ, 0,0, OPEN_EXISTING,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD dw;
		ReadFile(hFile,temp,5,&dw,NULL);
		if (temp[0]!='p' || temp[1]!='r' || temp[2]!='a' || temp[3]!='z')
		{
			MessageBox(hwndMain,"Файл не является файлом праздников","Ошибка",MB_ICONSTOP);
			CloseHandle(hFile);
			free(temp);
			return;
		}
		if (temp[4]!='3')
		{
			MessageBox(hwndMain,"Неверная версия файла праздников","Ошибка",MB_ICONSTOP);
			CloseHandle(hFile);
			free(temp);
			return;
		}
		MyPrazList.LoadFromFile(hFile);
		CloseHandle(hFile);
	}
	free(temp);
}

void SaveMyPraz()
{
	char* temp=(char*)malloc(220);
	lstrcpy(temp,DataDir);
	lstrcat(temp,DNEV_PRAZ_FILE);
	HANDLE hFile=CreateFile(temp,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		temp[0]='p';
		temp[1]='r';
		temp[2]='a';
		temp[3]='z';
		temp[4]='3';
		DWORD dw;
		WriteFile(hFile,temp,5,&dw,NULL);
		MyPrazList.SaveToFile(hFile);
		CloseHandle(hFile);
	}
	free(temp);
}

void InitPraz()
{
	hPrim=LoadResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDR_PRIM), "MDATA"));
	lbPrim=(LPBYTE)LockResource(hPrim);
	hPraz=LoadResource(hInstance, FindResource(hInstance, MAKEINTRESOURCE(IDR_PRAZ), "MDATA"));
	lbPraz=(LPBYTE)LockResource(hPraz);
	LoadMyPraz();
}

void FreePraz()
{
    FreeResource(hPrim);
    FreeResource(hPraz);
	MyPrazList.Clear();
}

void FormatMyPraz(PMyPraz ppr, char* buf)
{
	DateToStringWithoutYear(&ppr->Date, buf);
	strcat(buf," - ");
	strcat(buf,ppr->Message);
}

void PrazListToScreen()
{
	SendMessage(hPrazList,LB_RESETCONTENT,0,0);
	char *temp=(char*)malloc(120);
	for (int i=0; i<MyPrazList.count(); i++)
	{
		FormatMyPraz(&MyPrazList[i], temp);
		SendMessage(hPrazList,LB_ADDSTRING,0,(LPARAM)temp);
	}
	free(temp);
}

void UpdateDelButton()
{
	int i=SendMessage(hPrazList,LB_GETCURSEL,0,0);
	EnableWindow(GetDlgItem(hPrazDlg,IDC_BUTPRDEL),i!=LB_ERR);
}

int InsertMyPraz(PMyPraz ppr)
{
	int i=0;
	while (i<MyPrazList.count())
	{
		TDate dat=MyPrazList[i].Date;
		if (dat.Month>ppr->Date.Month || (dat.Month==ppr->Date.Month && dat.Day>ppr->Date.Day)) break;
		i++;
	}
	if (i==MyPrazList.count())
	{
		MyPrazList.Add(ppr);
		return MyPrazList.count()-1;
	}
	MyPrazList.Add(ppr,i);
	return i;
}

BOOL FAR PASCAL MyPrazItemDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		SetWindowText(GetDlgItem(hDlg,IDC_PRMESS),EditPraz.Message);
		RECT rc;
		GetClientRect(hDlg,&rc);
		CreateDateLabel(hDlg,(int)(rc.right*0.5),(int)(rc.bottom*0.25),&EditPraz.Date,1,FALSE);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam)==IDCANCEL)
		{
			EndDialog(hDlg,FALSE);
			return TRUE;
		}
		if (LOWORD(wParam)==IDOK) 
		{
			GetDate(&EditPraz.Date,1);
			GetWindowText(GetDlgItem(hDlg,IDC_PRMESS),EditPraz.Message,99);
			EndDialog(hDlg, TRUE);
			return (TRUE);
	    }
		if (HIWORD(wParam)==EN_CHANGE)
		{
			int i;
			i=GetWindowTextLength((HWND) lParam);
			EnableWindow(GetDlgItem(hDlg,IDOK),i>2);
		}
		break;
    }
    return (FALSE);
}

BOOL EditMyPraz()
{
	return DialogBox(hInstance, "MYPRAZITEMDLG", hPrazDlg, (DLGPROC)MyPrazItemDlgProc);
}

void OnAddPraz()
{
	EditPraz.Date=Now();
	EditPraz.Message[0]=0;
	if (!EditMyPraz()) return;
	int i=InsertMyPraz(&EditPraz);
	PrazListToScreen();
	SendMessage(hPrazList,LB_SETCURSEL ,i,0);
	UpdateDelButton();
	bPrazModified=TRUE;
}

void OnEditPraz()
{
	int i=SendMessage(hPrazList,LB_GETCURSEL,0,0);
	EditPraz=MyPrazList[i];
	if (!EditMyPraz()) return;
	MyPrazList.Del(i);
	i=InsertMyPraz(&EditPraz);
	PrazListToScreen();
	SendMessage(hPrazList,LB_SETCURSEL ,i,0);
	UpdateDelButton();
	bPrazModified=TRUE;
}

void OnDelPraz()
{
	if (MessageBox(hPrazDlg,"Действительно удалить праздник?","Удаление..",MB_YESNO | MB_ICONINFORMATION)==IDNO) return;
	int i=SendMessage(hPrazList,LB_GETCURSEL,0,0);
	MyPrazList.Del(i);
	PrazListToScreen();
	UpdateDelButton();
	bPrazModified=TRUE;
}

BOOL FAR PASCAL MyPrazDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		hPrazDlg=hDlg;
		hPrazList=GetDlgItem(hDlg,IDC_LIST1);
		PrazListToScreen();
		SendMessage(hPrazList,LB_SETCURSEL ,0,0);
		UpdateDelButton();
		return TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam)==IDCANCEL || LOWORD(wParam)==IDOK) 
		{
			EndDialog(hDlg, TRUE);
			return (TRUE);
	    }
		if (LOWORD(wParam)==IDC_BUTPRADD) OnAddPraz();
		if (LOWORD(wParam)==IDC_BUTPRDEL) OnDelPraz();
		if (HIWORD(wParam)==LBN_SELCHANGE) UpdateDelButton();
		if (HIWORD(wParam)==LBN_DBLCLK) OnEditPraz();
		break;
    }
    return (FALSE);
}

void PrazEdit()
{
	bPrazModified=FALSE;
	DialogBox(hInstance, "MYPRAZDLG", hwndMain, (DLGPROC)MyPrazDlgProc);
	if (bPrazModified) SaveMyPraz();
}
