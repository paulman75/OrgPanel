#include "windows.h"
#include "ListView.h"
#include "../Headers/general.h"
#include "units.h"
#include "Print.h"
#include "res/resource.h"

extern HWND hwndMain;
extern HINSTANCE hInstance;		  // Global instance handle for application
extern LPNOTEUNIT	ActiveUnit;
extern LPFIRMUNIT	ActiveFirm;
extern BOOL	bHumanMode;
extern char* ColName[];
extern char* FirmColName[];
extern  LPFIRMUNIT	FirstFirm;
extern  LPFIRMUNIT	nfNote;
extern  CSortListView*	cList;


const int xp=200;
int iRight;
PRINTDLG pd;

byte PrOrder[UN_COUNT-2]={UN_NAME,UN_BDATE,UN_SEX,UN_HOMETEL,UN_WORKTEL,UN_FAX,UN_SOTOV,UN_PAGER,UN_HOMEPAGE,UN_EMAIL,UN_ICQ,UN_COUNTRY,UN_ZIPCODE,UN_CITY,UN_OBLAST,UN_ADDRESS,UN_FIRM,UN_DOLV,UN_WIFE,UN_CHILDREN,UN_MISC};
byte PrFirmOrder[UNF_COUNT]={UNF_NAME,UNF_OPIS,UNF_REKV,UNF_COUNTRY,UNF_ZIPCODE,UNF_CITY,UNF_OBLAST,UNF_ADDRESS,UNF_HOMEPAGE,UNF_EMAIL,UNF_TEL1,UNF_TEL2,UNF_FAX,UNF_MISC};

BOOL MakeFirmNote(LPTSTR s, int iIndex, LPFIRMUNIT nf)
{
	s[0]=0;
	if (!nf->Fields[iIndex]) return FALSE;
	lstrcpy(s,FirmColName[iIndex]);
	lstrcat(s,": ");
	lstrcat(s,nf->Fields[iIndex]);
	return TRUE;
}

BOOL MakeUnitNote(LPTSTR s, int iIndex, LPNOTEUNIT nu)
{
	s[0]=0;
	if (iIndex==0)
	{
		if (nu->Fields[UN_SNAME])
		{
			lstrcpy(s,nu->Fields[UN_SNAME]);
			lstrcat(s," ");
		}
		if (nu->Fields[UN_NAME])
		{
			lstrcat(s,nu->Fields[UN_NAME]);
			lstrcat(s," ");
		}
		if (nu->Fields[UN_MNAME])
			lstrcat(s,nu->Fields[UN_MNAME]);
		return TRUE;
	}
	if (iIndex==UN_BDATE)
	{
		if (nu->BDate.Month)
		{
			char* temp2=(char*)malloc(100);
			DateToString(&nu->BDate,temp2);
			wsprintf(s,"Дата рождения: %sг.",temp2);
			free(temp2);
			return TRUE;
		}
		return FALSE;
	}
	if (iIndex==UN_WIFE)
	{
		if (!nu->Fields[iIndex]) return FALSE;
		if (nu->Sex)
		{
			if (nu->Sex==1)
				lstrcpy(s,"Жена: ");
			else lstrcpy(s,"Муж: ");
			lstrcat(s,nu->Fields[iIndex]);
			return TRUE;
		}
	}
	if (iIndex>=UN_HOMETEL && iIndex<=UN_CHILDREN)
	{
		if (!nu->Fields[iIndex]) return FALSE;
		lstrcpy(s,ColName[iIndex]);
		lstrcat(s,": ");
		lstrcat(s,nu->Fields[iIndex]);
		return TRUE;
	}
	if (iIndex==UN_SEX && nu->Sex)
	{
		lstrcpy(s,"Пол: ");
		if (nu->Sex==1) lstrcat(s,"муж");
		else lstrcat(s,"жен");
		return TRUE;
	}
	if (iIndex==UN_FIRM)
	{
		if (nu->Firm==0xffffffff) return FALSE;
		nfNote=FirstFirm;
		while (nfNote)
		{
			if (nfNote->Key==nu->Firm)
			{
				if (!nfNote->Fields[UNF_NAME]) return FALSE;
				lstrcpy(s,"Организация: ");
				lstrcat(s,nfNote->Fields[UNF_NAME]);
				return TRUE;
			}
			nfNote=nfNote->Next;
		}
	}
	return FALSE;
}

void InitPrinter()
{
	pd.lStructSize=sizeof(pd);
	pd.hInstance=hInstance;
	pd.hDevMode=NULL;
	pd.hDevNames=NULL;
	pd.Flags=PD_PRINTSETUP;
	pd.hwndOwner=hwndMain;
}

void ShowPrintDlg()
{
	PrintDlg(&pd);
}

char*	GetItem(char* s)
{
	while (*s!=44 && *s) s++;
	*s=0;
	return ++s;
}

void MyTextOut(HDC dc,LPTSTR s,int* y)
{
	RECT rc;
	rc.left=xp;
	rc.right=iRight;
	rc.top=*y;
	rc.bottom=*y+10000;
	int hei=DrawText(dc,s,lstrlen(s),&rc,DT_WORDBREAK);
	*y+=hei+35;
}

void Print()
{
	HDC dc;
	if (pd.hDevNames==NULL)
	{
		char* Info=(char*)malloc(100);
		GetProfileString("windows", "device", ",,", Info, 100);
		char* Driver=Info;
		char* PrType=GetItem(Info);
		char* Port=GetItem(PrType);
		GetItem(Port);
		dc=CreateDC(PrType, Driver, Port, NULL);
		free(Info);
	}
	else
	{
		LPDEVNAMES dm=(LPDEVNAMES)(*(LPDWORD)pd.hDevNames);
		dc=CreateDC((char*)((DWORD)dm+dm->wDriverOffset), (char*)((DWORD)dm+dm->wDeviceOffset), (char*)((DWORD)dm+dm->wOutputOffset), NULL);
	}
	int i=GetDeviceCaps(dc,HORZRES);
	int j=GetDeviceCaps(dc,HORZSIZE);
	iRight=j/200*i-200;
	if (Escape(dc, STARTDOC, 15, "Записная книжка", NULL)<0)
	{
		MessageBox(hwndMain, "Printing can not be started", NULL,MB_OK | MB_ICONSTOP);
		return;
	}
	if (Escape(dc, NEWFRAME, 0, NULL, NULL)<0)
	{
		MessageBox(hwndMain, "Paper can not be advanced", NULL, MB_OK | MB_ICONSTOP);
		DeleteDC(dc);
		return;
	}
	int y=(int)(GetDeviceCaps(dc,LOGPIXELSY)*0.9);

	char* temp=(char*)malloc(1000);
	if (bHumanMode)
	{
		for (int k=0; k<UN_COUNT-2; k++)
			if (MakeUnitNote(temp,PrOrder[k],ActiveUnit))
			MyTextOut(dc,temp,&y);
	}
	else
	{
		for (int k=0; k<UNF_COUNT; k++)
			if (MakeFirmNote(temp,PrFirmOrder[k],ActiveFirm))
			MyTextOut(dc,temp,&y);
	}
	free(temp);

	if (Escape(dc, NEWFRAME, 0, NULL, NULL)<0)
	{
		MessageBox(hwndMain, "Paper can not be advanced", NULL, MB_OK | MB_ICONSTOP);
		return;
	}
	if (Escape(dc, ENDDOC, 0, NULL, NULL)<0)
	{
		MessageBox(hwndMain, "Error Printing", NULL, MB_OK | MB_ICONSTOP);
	}
	DeleteDC(dc);
}

//Экспорт в файл
void ExportToText()
{
	char* txtfile=(char*)malloc(400);
	txtfile[0]=0;
	OPENFILENAME ofn;
	ZeroMemory(&ofn,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hwndMain;
	ofn.hInstance=hInstance;
	ofn.lpstrFilter="Текстовые файлы\0*.txt\0";
	ofn.lpstrCustomFilter=NULL;
	ofn.nFilterIndex=0;
	ofn.lpstrFile=txtfile;
	ofn.nMaxFile=399;
	ofn.lpstrFileTitle=NULL;
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrTitle="Выбор текстового файла";
	ofn.Flags=OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt="txt";
	if (!GetOpenFileName(&ofn))
	{
		free(txtfile);
		return;
	}
	if (FileExists(txtfile))
	{
		char* temp=(char*)malloc(400);
		lstrcpy(temp,"Файл ");
		lstrcat(temp,txtfile);
		lstrcat(temp," существует. Перезаписать?");
		int i=MessageBox(hwndMain,temp,"Экспорт",MB_YESNO | MB_ICONQUESTION);
		free(temp);
		if (i==IDNO)
		{
			free(txtfile);
			return;
		}
	}
	HANDLE hFile=CreateFile(txtfile,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		char* temp=(char*)malloc(1000);
		int Count=SendMessage(cList->GetWnd(),LVM_GETITEMCOUNT,0,0);
		DWORD dw;
		int CurCount=1;
		byte ent[2]={0x0d,0x0a};
		for (int i=0; i<Count; i++)
		{
			LV_ITEM lv;
			ZeroMemory(&lv,sizeof(lv));
			lv.mask=LVIF_PARAM;
			lv.iItem=i;
			lv.iSubItem=0;
			SendMessage(cList->GetWnd(),LVM_GETITEM,0,(LPARAM)&lv);
			if (bHumanMode)
			{
				for (int k=0; k<UN_COUNT-2; k++)
					if (MakeUnitNote(temp,PrOrder[k],(LPNOTEUNIT)lv.lParam))
					{
						WriteFile(hFile,temp,lstrlen(temp),&dw,NULL);
						WriteFile(hFile,ent,2,&dw,NULL);
					}
			}
			else
			{
				for (int k=0; k<UNF_COUNT; k++)
					if (MakeFirmNote(temp,PrFirmOrder[k],(LPFIRMUNIT)lv.lParam))
					{
						WriteFile(hFile,temp,lstrlen(temp),&dw,NULL);
						WriteFile(hFile,ent,2,&dw,NULL);
					}
			}
			WriteFile(hFile,ent,2,&dw,NULL);
		}
		free(temp);
		CloseHandle(hFile);
	}
	free(txtfile);
}
