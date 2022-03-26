#include "Windows.h"
#include "Compon\Classes.h"
#include "BarStruct.h"
#include "..\Headers\general.h"
#include "Compon\MyUtils.h"
#include "Compon\Shortcut.h"
#include "AppBar.h"
#include "Compon\Shortcut.h"
#include "BarUnit.h"
#include "Compon\FolderSw.h"
#include "Res\resource.h"
#include "Compon\ToolBar2.h"
#include "..\Headers\const.h"
#include "RegUnit.h"

extern char	MainDir[200];
extern HWND BarhWnd;
extern HINSTANCE hInstance;		  // Global instance handle for application
extern HWND hwndToolTip;
extern CAppBar* hBar;
extern byte	bIconSize,IconBorder,FullIcon;
extern char	buf[1000],buf2[230];
extern CFolderSwitch*	Switch;
extern HMENU	hButMenu;
extern char MainBarDir[200];
extern BOOL PropEdit;
extern HWND MenuWnd;
extern TBarConfig	BarCon;

DWORD	dwd;
TLnk	nLnk;
PBarFolder ActiveFolder,MainFolder,FirstFolder,nFolder;
BarUnit*	nUnit;
BarUnit*	hMenuUnit;
WNDPROC		OldButProc;
CToolBar2*	MainBar;
CToolBar2*	FolderBar;
char lab[]="ind3";
char folderlab[]="fol5";

byte ComputerLink[104]={0x4c, 0, 0, 0, 1, 20 , 2 ,0, 0,0,0,0,0xC0,0,0,0,
						0, 0, 0, 0x46, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
						1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x16,0,0x14,0,0x1f,15,0xe0,0x4f,
0xd0,0x20,0xea,0x3a,0x69,16,0xa2,0xd8,8,0,0x2b,0x30,0x30,0x9d,0,0,0,0,0,0};

byte PanelLink[124]={0x4c, 0, 0, 0, 1, 20 , 2 ,0, 0,0,0,0,0xC0,0,0,0,
						0, 0, 0, 0x46, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
						1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x2a,0,0x14,0,0x1f,15,0xe0,0x4f,
0xd0,0x20,0xea,0x3a,0x69,16,0xa2,0xd8,8,0,0x2b,0x30,0x30,0x9d,20,0,0x2e,0x77,32,32,0xec,0x21,0xea,0x3a,
0x69,16,0xa2,0xdd,8,0,0x2b,0x30,0x30,0x9d,0,0,0,0,0,0};

PBarFolder FindBarFolder(WORD x,WORD y)
{
RECT src;
nFolder=MainFolder;
GetClientRect(Switch->hWnd,&src);
MapWindowPoints(Switch->hWnd,NULL,(LPPOINT)&src,2);
if (hBar->MoveEdge==ABE_BOTTOM || hBar->MoveEdge==ABE_TOP || hBar->MoveEdge == ABE_DESKTOP)
{
        if (x>src.left) nFolder=ActiveFolder;
} else
if (y>src.bottom && ActiveFolder!=NULL) nFolder=ActiveFolder;
return nFolder;
}

void DetectDescription(LPLnk ll)
{
	WORD i=lstrlen(ll->strLnkPath.Text)-1;
	if (i>6000) return;
	while (ll->strLnkPath.Text[i]!=0x2e && ll->strLnkPath.Text[i]!=0x5c) i--;
	if (ll->strLnkPath.Text[i]==0x5c) i=lstrlen(ll->strLnkPath.Text)-1;
	WORD j=i-1;
	while (ll->strLnkPath.Text[i--]!=0x5c);
	i++;
	i++;
	byte k=0;
	while (i<=j) buf[k++]=ll->strLnkPath.Text[i++];
	buf[k]=0;
	ll->strDescription=(LPCSTR)buf;
}

void CheckExistsUnitName(LPTSTR FileName)
{

	char ext[10];
	ExtractFileExt(FileName,ext);
	while (FileExists(FileName))
	{
		int i=lstrlen(FileName)-1;
		while (i && FileName[i]!=46) i--;
		FileName[i++]=49;
		FileName[i]=0;
		lstrcat(FileName,ext);
	}
}

void AddSplitter(POINT* point)
{
PBarFolder onFolder=FindBarFolder((WORD)point->x,(WORD)point->y);
if (onFolder==NULL) return;

nUnit=onFolder->FirstUnit;
BarUnit*	mUnit=NULL;
while (nUnit!=NULL) 
{
        mUnit=nUnit;
        nUnit=nUnit->NextUnit;
}
nUnit=new BarUnit();
nUnit->PrevUnit=mUnit;
if (mUnit!=NULL) mUnit->NextUnit=nUnit;
onFolder->Kol++;
if (mUnit==NULL) onFolder->FirstUnit=nUnit;
nUnit->ParentFolder=onFolder;
nUnit->Splitter=TRUE;

*nUnit->strPath="";
*nUnit->FilePath="";
*nUnit->Caption="Разделитель";
*nUnit->IconPath="";
nUnit->IconIndex=0;
nUnit->NextUnit=NULL;
nUnit->Hidden=FALSE;
nUnit->ButImage=NULL;
nUnit->BigImage=NULL;

onFolder->Modified=TRUE;
PlaceIcon();
InvalidateRect(BarhWnd,NULL,TRUE);
SaveMainBitmap();
SaveBarStruct();
}

void AddIcon(POINT* point)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=BarhWnd;
	ofn.hInstance=hInstance;
	ofn.lpstrFilter="Все файлы\0*.*\0";
	ofn.lpstrCustomFilter=NULL;
	ofn.nFilterIndex=0;
	char* buf=(char*)malloc(300);
	buf[0]=0;
	ofn.lpstrFile=buf;
	ofn.nMaxFile=299;
	ofn.lpstrFileTitle=NULL;
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrTitle="Добавление значка";
	ofn.Flags=OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt=NULL;
	if (!GetOpenFileName(&ofn)) return;
	DropUnit(buf,(WORD)point->x,(WORD)point->y);
	PlaceIcon();
	InvalidateRect(BarhWnd,NULL,TRUE);
	SaveMainBitmap();
	SaveBarStruct();
}

void DropUnit(LPCTSTR name,WORD x, WORD y)
{
nLnk.strLnkPath=name;
PBarFolder onFolder=FindBarFolder(x,y);
if (onFolder==NULL) return;
DetectDescription(&nLnk);
char Ext[10];
ExtractFileExt(name,Ext);
lstrcpy(buf2,onFolder->Path->Text);
lstrcat(buf2,nLnk.strDescription.Text);
char ex1[5]=".lnk";
if (lstrcmpi(Ext,".url")==0) lstrcpy(ex1,".url");
if (lstrcmpi(Ext,".pif")==0) lstrcpy(ex1,".pif");
lstrcpy(buf,buf2);
lstrcat(buf,ex1);
CheckExistsUnitName(buf);
nLnk.strLnkPath=buf;
if (lstrcmpi(Ext,".lnk")==0 || lstrcmpi(Ext,".url")==0 || lstrcmpi(Ext,".pif")==0)
{
        CopyFile(name,buf,FALSE);
        GetLnkData(&nLnk);
}
else 
{
        nLnk.strPath=name;
		ExtractFilePath(name,buf);
		nLnk.strStartDir=buf;
		nLnk.strIconLocation="";
		nLnk.nIconIndex=0;
        SaveLnkData(&nLnk);
		if (!FileExists(nLnk.strLnkPath.Text))
		{
			int i=lstrlen(nLnk.strLnkPath.Text)-1;
			nLnk.strLnkPath.Text[i--]='f';
			nLnk.strLnkPath.Text[i--]='i';
			nLnk.strLnkPath.Text[i--]='p';
		}
}
nUnit=onFolder->FirstUnit;
BarUnit*	mUnit=NULL;
while (nUnit!=NULL) 
{
        mUnit=nUnit;
        nUnit=nUnit->NextUnit;
}
nUnit=new BarUnit();
nUnit->PrevUnit=mUnit;
if (mUnit!=NULL) mUnit->NextUnit=nUnit;
onFolder->Kol++;
if (mUnit==NULL) onFolder->FirstUnit=nUnit;
nUnit->ParentFolder=onFolder;
*nUnit->strPath=nLnk.strPath;
*nUnit->FilePath=nLnk.strLnkPath;
*nUnit->Caption=nLnk.strDescription;
*nUnit->IconPath=nLnk.strIconLocation;
nUnit->IconIndex=nLnk.nIconIndex;
nUnit->NextUnit=NULL;
nUnit->Splitter=FALSE;
nUnit->Hidden=FALSE;
onFolder->Modified=TRUE;
BitmapToUnitFromIcon(nUnit);
}

void HideButton(BarUnit* First)
{
PlaceIcon();
}

WORD FolderBitmapToMain(PBarFolder fol, HDC bDC, WORD Index)
{
	if (fol==NULL) return(Index);
	nUnit=fol->FirstUnit;
	while (nUnit!=NULL)
	{
		if (nUnit->ButImage!=NULL) 
			BitBlt(bDC,Index*64,0,32,32,nUnit->ButImage->hdc,0,0,SRCCOPY);
		if (nUnit->BigImage!=NULL) 
			BitBlt(bDC,Index*64,32,64,64,nUnit->BigImage->hdc,0,0,SRCCOPY);		
		Index++;
		nUnit=nUnit->NextUnit;
	}
	return Index;
}

void SaveMainBitmap()
{
	if (MainFolder==NULL) return;
	WORD kol=MainFolder->Kol;
	nFolder=FirstFolder;
	while (nFolder!=NULL) 
	{
		kol+=nFolder->Kol;
		nFolder=nFolder->NextFolder;
	}
	if (kol==0) return;
	CMyBitmap* Bit=new CMyBitmap(kol*64,64+32);
	kol=FolderBitmapToMain(MainFolder,Bit->hdc,0);
	nFolder=FirstFolder;
	while (nFolder!=NULL) 
	{
		kol=FolderBitmapToMain(nFolder,Bit->hdc,kol);
		nFolder=nFolder->NextFolder;
	}
	lstrcpy(buf,MainBarDir);
	lstrcat(buf,"Icons.bmp");

	Bit->SaveBitmap(buf);
	delete Bit;
}

void BitmapToUnitFromIcon(BarUnit* uni)
{
   uni->DeleteBitmap();
	nLnk.strLnkPath=*uni->FilePath;
	GetLnkData(&nLnk);
	*uni->IconPath=nLnk.strIconLocation;
	uni->IconIndex=nLnk.nIconIndex;
	/*ExtractFileExt(uni->strPath->Text, buf2);

	if (lstrcmpi(buf2, ".msc") == 0)
	{
		//Читаем иконку из msc файла
		HANDLE	hFile;
		if ((hFile = CreateFile(uni->strPath->Text, GENERIC_READ, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE)
		{
			DWORD dwSize = GetFileSize(hFile, NULL);
			if (dwSize != INVALID_FILE_SIZE)
			{
				char* filbuf = (char*)malloc(dwSize);

				DWORD dwd;
				BOOL rf = ReadFile(hFile, filbuf, dwSize, &dwd, NULL);
				char* lbig=strstr(filbuf, "CONSOLE_FILE_ICON_LARGE");
				lbig = strstr(lbig, "\">");
				lbig++;
				lbig++;
				if (lbig[0] == '\r') lbig++;
				if (lbig[0] == '\n') lbig++;
				char* lend = strstr(lbig, "</Binary>");
				if (lend) lend[0] = char(0);
				int ilen64 = lstrlen(lbig);
				char* imbuf = (char*)malloc(ilen64*2);
				size_t ilen;

				imbuf = base64_encode(lbig, ilen64, &ilen);

				free(filbuf);
			}
			// Закрываем файл
			CloseHandle(hFile);
		}
	}
		*/
  if (*uni->IconPath=="") 
  {
	ExtractFileExt(uni->FilePath->Text,buf2);
	if (lstrcmpi(buf2,".url")==0)
	{
		ReadUrl(uni);
		if (*uni->IconPath=="") GetIconExt(uni->FilePath->Text,uni);
	}
  }
  if (*uni->IconPath=="") GetIconExt(uni->strPath->Text,uni);
  if (*uni->IconPath=="") GetIconFromLnk(uni);

	HICON hIcon=NULL;
	HICON hSmall=NULL;
	if (*uni->IconPath=="") ;
		else ExtractIconEx(uni->IconPath->Text,uni->IconIndex,&hIcon,0,1);
	unsigned int pii;
	int res = PrivateExtractIcons(uni->IconPath->Text, uni->IconIndex, 64, 64, &hSmall, &pii, 1, 16);
	if (hIcon==NULL) hIcon=LoadIcon(hInstance,"NULLICON");
	if (hSmall==NULL) hSmall=LoadIcon(hInstance,"NULL64");
  
        /// ...
	uni->ButImage=new CMyBitmap(32,32);
	DrawIcon(uni->ButImage->hdc,0,0,hIcon);
	uni->BigImage=new CMyBitmap(64,64);
	DrawIconEx(uni->BigImage->hdc,0,0,hSmall,64,64,0,NULL,DI_NORMAL);
	DeleteObject(hSmall);
	DeleteObject(hIcon);
}

BarUnit* LoadUnit(BarUnit* PrevUnit,PBarFolder fol,HANDLE Fil,LPWORD pIndex,HDC cc, BOOL bVer)
{
	BarUnit* uni;
	uni=new BarUnit();
	BOOL rf=ReadFile(Fil,&buf,bVer ? 2 : 3,&dwd,NULL);
	uni->Hidden=(BOOL)buf[0];
	uni->ParentFolder=fol;
	byte ll=buf[bVer ? 1 : 2];
	uni->Splitter=FALSE;

	if (ll==255||ll==254)
	{
		uni->Splitter=TRUE;
		*uni->strPath="";
		*uni->FilePath="";
		*uni->Caption="";
		*uni->IconPath="";
		uni->ButImage=NULL;
		uni->BigImage=NULL;
		if (ll==254)//Версия с именами сплиттеров
		{
			ZeroMemory(buf2,230);
			rf=ReadFile(Fil,buf2,1,&dwd,NULL);
			ll=buf2[0];
			ZeroMemory(buf2,230);
			rf=ReadFile(Fil,buf2,ll,&dwd,NULL);
			*uni->Caption=buf2;
		}
	}
	else
	{
		ZeroMemory(buf2,230);
		BOOL rf=ReadFile(Fil,buf2,ll,&dwd,NULL);
		byte Spec=buf[1];
		if (buf2[1]!=':')
		{
			lstrcpy(buf,fol->Path->Text);
			lstrcat(buf,buf2);
		}
		else lstrcpy(buf,buf2);
		*uni->FilePath=buf;
		nLnk.strLnkPath=*uni->FilePath;
		GetLnkData(&nLnk);
		DetectDescription(&nLnk);
		if (!bVer && Spec)
		{
			lstrcpy(buf,MainDir);
			switch (Spec)
			{
			case 1:	lstrcat(buf,"notebook.exe");
				break;
			case 4:
			case 2: lstrcat(buf,"diary.exe");
				break;
			case 3: lstrcat(buf,"notepad.exe");
				break;
			}
			nLnk.strPath=buf;
			nLnk.strIconLocation=buf;
			nLnk.nIconIndex=0;
			SaveLnkData(&nLnk);
		}
		*uni->IconPath=nLnk.strIconLocation;
		uni->IconIndex=nLnk.nIconIndex;
		*uni->strPath=nLnk.strPath;
		*uni->Caption=nLnk.strDescription;
	}
	uni->PrevUnit=PrevUnit;
	if (PrevUnit!=NULL) PrevUnit->NextUnit=uni;
	if (fol!=NULL) fol->Kol++;
	if (PrevUnit==NULL && fol!=NULL) fol->FirstUnit=uni;
	uni->NextUnit=NULL;
	if (cc!=NULL) 
	{
		uni->ButImage=new CMyBitmap(32,32);
		BitBlt(uni->ButImage->hdc,0,0,32,32,cc,(*pIndex)*64,0,SRCCOPY);
		uni->BigImage=new CMyBitmap(64,64);
		BitBlt(uni->BigImage->hdc,0,0,64,64,cc,(*pIndex)++*64,32,SRCCOPY);
	}
	return uni;
}

PBarFolder CreateFolder(PBarFolder pprev)
{
nFolder=(PBarFolder)malloc(sizeof(TBarFolder));
nFolder->Modified=FALSE;
nFolder->Kol=0;
nFolder->FirstUnit=NULL;
nFolder->PrevFolder=pprev;
nFolder->NextFolder=NULL;
nFolder->Caption=new CMyString();
nFolder->Path=new CMyString();
nFolder->TitColor=0;
if (pprev!=NULL) pprev->NextFolder=nFolder;
if (pprev!=NULL) nFolder->Index=pprev->Index+1;
else nFolder->Index=1;
return nFolder;
}

PBarFolder LoadFolder(LPTSTR cap, LPTSTR path,PBarFolder prevF,LPWORD pIndex,HDC cc, COLORREF aBkColor, COLORREF aBkColor2, COLORREF aTitColor, byte iAlpha)
{
PBarFolder fol;
if (!DirectoryExists(path)) return(NULL);
HANDLE	hFile;
char FilePath[200];
lstrcpy(FilePath,path);
lstrcat(FilePath,"index.dat");
if ((hFile=CreateFile(FilePath,GENERIC_READ,0,NULL,OPEN_EXISTING,
	 FILE_ATTRIBUTE_NORMAL,0))==INVALID_HANDLE_VALUE) return(NULL);
fol=CreateFolder(prevF);
*fol->Caption=cap;
*fol->Path=path;
fol->BkColor=aBkColor;
fol->BkColor2=aBkColor2;
fol->TitColor=aTitColor;
fol->Alpha=iAlpha;
byte UnitKol;
ZeroMemory(buf,5);
BOOL rf=ReadFile(hFile,buf,4,&dwd,NULL);
BOOL bCorrectVersion=lstrcmp(buf,lab)==0;
if (!bCorrectVersion) SetFilePointer(hFile,0,0,FILE_BEGIN);
if (!ReadFile(hFile,&UnitKol,1,&dwd,NULL)) 
{
	free(fol);
	fol=NULL;
	CloseHandle(hFile);
	return(NULL);
}
BarUnit* PrevUnit=NULL;
while (UnitKol>0)
{
        nUnit=LoadUnit(PrevUnit,fol,hFile,pIndex,cc,bCorrectVersion);
        UnitKol--;
        PrevUnit=nUnit;
}
CloseHandle(hFile);
return (fol);
}

void IconPosition(CToolBar2* bar, BarUnit* nn,LPPOINT Poi, byte iEdge)
{
	bar->BeginUpdate();
	bar->SetButSize(FullIcon);
	bar->SetEdge(iEdge);
	bar->FreeAll();
	bar->SetNewPosition((WORD)Poi->x,(WORD)Poi->y);
	while (nn!=NULL) 
	{
        if (!nn->Hidden) 
			bar->AddButton(nn->Caption->Text,BarCon.LargeIcons ? nn->BigImage: nn->ButImage,(LPVOID)nn, nn->Splitter);
        nn=nn->NextUnit;
	}
	bar->EndUpdate();
}

WORD GetFolderWidth(PBarFolder fol)
{
	if (fol==NULL) return 0;
	nUnit=fol->FirstUnit;
	WORD res=0;
	while (nUnit!=NULL)
	{
		if (!nUnit->Hidden)
		{
			if (nUnit->Splitter) res += 10 + 3;
			else res += (FullIcon + 3);
		}
		nUnit=nUnit->NextUnit;
	}
	return res;
}

WORD GetMaxUnitKol()
{
	WORD ires = 0;
	PBarFolder pfol = FirstFolder;
	while (pfol)
	{
		WORD icou = GetFolderWidth(pfol);
		if (icou > ires) { ires = icou; }
		pfol = pfol->NextFolder;
	}
	return ires;
}


void PlaceIcon()
{
POINT pt;
BOOL Hor=FALSE;
if (MainFolder==NULL) return;
pt.y=20;
pt.x=3+hBar->XTitle;
if (hBar->MoveEdge==ABE_TOP || hBar->MoveEdge==ABE_BOTTOM)
{	
	    pt.x=20;  pt.y=3+hBar->YTitle;
        Hor=TRUE;
}
WORD w=100;
WORD MainWidth= GetFolderWidth(MainFolder);
WORD MaxFolWidth = GetMaxUnitKol();
if (hBar->MoveEdge == ABE_DESKTOP)
{
	w = (MainWidth + MaxFolWidth) + (BarCon.LargeIcons ? 105 : 65) ;// +(FullIcon + 2);
//        if (KolFol*(FullIcon+2)>w) w=KolFol*(FullIcon+2);
        //w=w+5;
        hBar->SetWidthOnDesktop(w);
//		pt.x = 2;// (w / 2) - ((KolMain * (FullIcon + 2) + FullIcon + 12) / 2);
//        pt.y=19;
		pt.y = 3 + hBar->YTitle + 26;
		pt.x = 5;
        Hor=TRUE;
}
nUnit=MainFolder->FirstUnit;
IconPosition(MainBar,nUnit,&pt,hBar->MoveEdge);
if (ActiveFolder==NULL)
{
	ShowWindow(Switch->hWnd,SW_HIDE);
	return;
}
ShowWindow(Switch->hWnd,SW_SHOW);
byte n=0;
nFolder=FirstFolder;
while (nFolder!=NULL) 
{
	n++;
	nFolder=nFolder->NextFolder;
}
RECT rc;
GetClientRect(MainBar->GetHWND(),&rc);
if (Hor) pt.x+=rc.right-rc.left;
else pt.y+=rc.bottom-rc.top;
//if (hBar->MoveEdge==ABE_DESKTOP) pt.y-=2;
Switch->UpdatePosition((WORD)(pt.x+Hor*FullIcon/5),(WORD)(pt.y+(1-Hor)*FullIcon/5-1),FullIcon,FullIcon+4,n,TRUE);
if (Hor) pt.x+=(WORD)(FullIcon*1.4);  
else pt.y+=(WORD)(FullIcon*1.4);
/*if (hBar->MoveEdge == ABE_DESKTOP)
{
        pt.x=w/2-KolFol*(FullIcon+2)/2;
        pt.y=60;
}*/
nUnit=ActiveFolder->FirstUnit;
IconPosition(FolderBar,nUnit,&pt,hBar->MoveEdge);
InvalidateRect(BarhWnd, NULL, TRUE);
//RepaintBar();
}

void WriteString(HANDLE Fil, LPCTSTR st)
{
	DWORD dwd;
	byte len=lstrlen(st);
	WriteFile(Fil,&len,1,&dwd,NULL);
	WriteFile(Fil,st,len,&dwd,NULL);
}

void SaveUnitToIndex(HANDLE Fil,BarUnit* uni)
{          
	buf[0]=uni->Hidden;
    DWORD dwd;

	if (uni->Splitter)
	{
		buf[1]=(byte)254; //Если разделитель, то пишем 254
		WriteFile(Fil,&buf,2,&dwd,NULL);
    
		buf[0]=lstrlen(uni->Caption->Text);

		WriteFile(Fil,&buf,1,&dwd,NULL);

		WriteFile(Fil,uni->Caption->Text,buf[0],&dwd,NULL);
	}
	else
	{
		char *f=(char*)malloc(250);
		ExtractFileName(uni->FilePath->Text,f);
    
		buf[1]=lstrlen(f);

		WriteFile(Fil,&buf,2,&dwd,NULL);

		WriteFile(Fil,f,buf[1],&dwd,NULL);
		free(f);
	}
}

void FreeAllStruct()
{
	delete MainBar;
	delete FolderBar;
}

#define TOPIC1	"Органайзер"
#define TOPIC2	"Утилиты"

void CreateUnit(HANDLE hFile, LPCTSTR topic, LPCTSTR name, LPCTSTR path, LPCTSTR filename)
{
	nUnit=new BarUnit();
	nUnit->Hidden=FALSE;
	nUnit->Splitter=FALSE;
	lstrcpy(buf,MainBarDir);
	lstrcat(buf,topic);
	lstrcat(buf,"\\");
	lstrcat(buf,name);
	*nUnit->FilePath=buf;
	nLnk.nIconIndex=0;
	nLnk.strDescription="";
	nLnk.strStartDir="";
	nLnk.strLnkPath=buf;
	lstrcpy(buf,path);
	lstrcat(buf,filename);
	nLnk.strIconLocation=buf;
	nLnk.strPath=buf;
	SaveLnkData(&nLnk);
	SaveUnitToIndex(hFile,nUnit);
	delete nUnit;
}

void CreateBarStruct()
{
FirstFolder=NULL;
MainBar=new CToolBar2(BarhWnd,0,0,FullIcon,TRUE,hwndToolTip,0);
FolderBar=new CToolBar2(BarhWnd,0,0,FullIcon,TRUE,hwndToolTip,1);
MainBar->bCanDrag=TRUE;
FolderBar->bCanDrag=TRUE;
if (!DirectoryExists(MainBarDir))
{
        CreateDirectory(MainBarDir,NULL);
		// Создаем folder.dat
	HANDLE	hFile;
 
	lstrcpy(buf,MainBarDir);
	lstrcat(buf,"folder.dat");
	hFile = CreateFile(buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile,&folderlab,4,&dwd,NULL);

		lstrcpy(buf,MainBarDir);
		lstrcat(buf,TOPIC1);
		CreateDirectory(buf,NULL);
		WriteString(hFile,TOPIC1);
		COLORREF bk=0x010101;
		COLORREF bk2=0x512101;
		COLORREF tit=0xffffff;
		WriteFile(hFile,&bk,sizeof(COLORREF),&dwd,NULL);
		WriteFile(hFile,&bk2,sizeof(COLORREF),&dwd,NULL);
		WriteFile(hFile,&tit,sizeof(COLORREF),&dwd,NULL);
		buf[0]=0;
		WriteFile(hFile,buf,10,&dwd,NULL);
		lstrcpy(buf,MainBarDir);
		lstrcat(buf,TOPIC2);
		CreateDirectory(buf,NULL);
		WriteString(hFile,TOPIC2);
		WriteFile(hFile,&bk,sizeof(COLORREF),&dwd,NULL);
		WriteFile(hFile,&bk2,sizeof(COLORREF),&dwd,NULL);
		WriteFile(hFile,&tit,sizeof(COLORREF),&dwd,NULL);
		buf[0]=0;
		WriteFile(hFile,buf,10,&dwd,NULL);
		DWORD dwd;
		byte Kol=0;
		WriteFile(hFile,&Kol,1,&dwd,NULL);
		CloseHandle(hFile);
		//Создаем корневой index.dat
		lstrcpy(buf,MainBarDir);
		lstrcat(buf,"index.dat");
		hFile = CreateFile((LPCTSTR)&buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
		if (hFile!=INVALID_HANDLE_VALUE)
		{
			nUnit=new BarUnit();
			WriteFile(hFile,lab,4,&dwd,NULL);
			Kol=2;
			WriteFile(hFile,&Kol,1,&dwd,NULL);
			nUnit->Hidden=FALSE;
			nUnit->Splitter=FALSE;
			lstrcpy(buf,MainBarDir);
			lstrcat(buf,"Мой компьютер.lnk");
			*nUnit->FilePath=buf;
			HANDLE	hFil2;
			hFil2 = CreateFile(buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
			if (hFil2!=INVALID_HANDLE_VALUE)
			{
				WriteFile(hFil2,&ComputerLink,104,&dwd,NULL);
				CloseHandle(hFil2);
			}
			SaveUnitToIndex(hFile,nUnit);
			lstrcpy(buf,MainBarDir);
			lstrcat(buf,"Контрольная панель.lnk");
			*nUnit->FilePath=buf;
			hFil2 = CreateFile(buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
			if (hFil2!=INVALID_HANDLE_VALUE)
			{
				WriteFile(hFil2,&PanelLink,124,&dwd,NULL);
				CloseHandle(hFil2);
			}
			SaveUnitToIndex(hFile,nUnit);
			CloseHandle(hFile);
			delete nUnit;
		}

		//Создаем index.dat в Органайзере
		lstrcpy(buf,MainBarDir);
		lstrcat(buf,"Органайзер\\index.dat");
		hFile = CreateFile((LPCTSTR)&buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
		if (hFile!=INVALID_HANDLE_VALUE)
		{
			WriteFile(hFile,lab,4,&dwd,NULL);
			Kol=3;
			WriteFile(hFile,&Kol,1,&dwd,NULL);

			CreateUnit(hFile,TOPIC1,"Записная книжка.lnk",MainDir,"notebook.exe");
			CreateUnit(hFile,TOPIC1,"Дневник.lnk",MainDir, "diary.exe");
			CreateUnit(hFile,TOPIC1,"Записки.lnk",MainDir, "notepad.exe");
			CloseHandle(hFile);
		}
		//Создаем index.dat в Утилиты
		lstrcpy(buf,MainBarDir);
		lstrcat(buf,"Утилиты\\index.dat");
		hFile = CreateFile(buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
		if (hFile!=INVALID_HANDLE_VALUE)
		{
			WriteFile(hFile,lab,4,&dwd,NULL);
			Kol=3;
			WriteFile(hFile,&Kol,1,&dwd,NULL);
			CreateUnit(hFile,TOPIC2,"Paint.lnk","","pbrush.exe");
			CreateUnit(hFile,TOPIC2,"Конвертер.lnk",MainDir,"UnitConv.exe");
			CreateUnit(hFile,TOPIC2,"Хранитель паролей.lnk",MainDir,"PassKeeper.exe");
			CloseHandle(hFile);
		}
	}
}
ZeroMemory(buf,1000);
lstrcpy(buf,MainBarDir);
lstrcat(buf,"Icons.bmp");
CMyBitmap* Bit=new CMyBitmap(buf);
WORD	Index=0;
MainFolder=LoadFolder("Главная",MainBarDir,NULL,&Index,Bit->hdc, BarCon.BackGr, BarCon.BackGr2, 0, 0);
ActiveFolder=NULL;

if (MainFolder!=NULL)
{
	HANDLE	hFile;
	lstrcpy(buf,MainBarDir);
	lstrcat(buf,"folder.dat");
	if ((hFile=CreateFile(buf,GENERIC_READ,0,NULL,OPEN_EXISTING,
	 FILE_ATTRIBUTE_NORMAL,0))!=INVALID_HANDLE_VALUE)
	{
		byte FolderPathLen;
		DWORD	dwd;
		char filver[5];
		ZeroMemory(filver,5);
		BOOL rf=ReadFile(hFile,filver,4,&dwd,NULL);
		int bFileVersion=2;
		if (lstrcmp(filver,"fol3")==0 ) { bFileVersion = 3; }
		if (lstrcmp(filver,"fol4")==0 ) { bFileVersion = 4; }
		if (lstrcmp(filver,"fol5")==0 ) { bFileVersion = 5; }

		if (bFileVersion==2) SetFilePointer(hFile,0,0,FILE_BEGIN);

		PBarFolder     prevF=NULL;
        while (TRUE) 
		{
			DWORD Reserved=0;

			rf=ReadFile(hFile,&FolderPathLen,1,&dwd,NULL);
			if ((FolderPathLen==0)||(!dwd)) break;

			ZeroMemory(&buf,1000);
            rf=ReadFile(hFile,buf,FolderPathLen,&dwd,NULL);
			if (buf[1]!=':')
			{
				lstrcpy(buf2,buf);
				lstrcpy(buf,MainBarDir);
				lstrcat(buf,buf2);
				if (buf[lstrlen(buf)-1]!=0x5c) lstrcat(buf,"\\");
			}
			byte i=lstrlen(buf)-2;
			while (buf[i]!=0x5c) i--;
			ZeroMemory(buf2,200);
			byte k=0;
			i++;
			while (buf[i]!=0x5c) buf2[k++]=buf[i++];

			COLORREF bkCol=BarCon.BackGr;
			COLORREF bkCol2=BarCon.BackGr2;
			COLORREF titCol=0;
			if (bFileVersion>=3)
			{
				rf=ReadFile(hFile,&bkCol,sizeof(COLORREF),&dwd,NULL);
			}
			if (bFileVersion>=4)
			{
				rf=ReadFile(hFile,&bkCol2,sizeof(COLORREF),&dwd,NULL);
			}
			else bkCol2=bkCol;
			if (bFileVersion>=3)
			{
				rf=ReadFile(hFile,&titCol,sizeof(COLORREF),&dwd,NULL);
			}
			byte iAlpha=0;
			if (bFileVersion>=5)
			{
				char cReserv[10];
				rf=ReadFile(hFile,cReserv,10,&dwd,NULL);
				iAlpha=cReserv[0];
			}
            nFolder=LoadFolder(buf2,buf,prevF,&Index,Bit->hdc, bkCol, bkCol2, titCol, iAlpha);
            if (prevF==NULL) FirstFolder=nFolder;
            prevF=nFolder;
		}
		CloseHandle(hFile);
		ActiveFolder=FirstFolder;
		for (int nom=1; nom<BarCon.LastFolderIndex; nom++) if (ActiveFolder!=NULL) ActiveFolder=ActiveFolder->NextFolder;
        if (!ActiveFolder) ActiveFolder=FirstFolder;
		else SetSwitchValue(BarCon.LastFolderIndex);
	}
}
if (Bit->hdc==NULL) UpdateIcon();
delete Bit;
PlaceIcon();
}

void SaveFolderIndex(PBarFolder fol)
{
if (!fol->Modified) return;
lstrcpy(buf,fol->Path->Text);
lstrcat(buf,"index.dat");
HANDLE hFile = CreateFile(buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
if (hFile==INVALID_HANDLE_VALUE) return;
DWORD	dwd;
WriteFile(hFile,lab,4,&dwd,NULL);
WriteFile(hFile,&fol->Kol,1,&dwd,NULL);
nUnit=fol->FirstUnit;
while (nUnit!=NULL)
{
	SaveUnitToIndex(hFile,nUnit);
	nUnit=nUnit->NextUnit;
}
CloseHandle(hFile);
fol->Modified=FALSE;
}

void SaveBarStruct()
{
	SaveFolderIndex(MainFolder);
	nFolder=FirstFolder;
	while (nFolder!=NULL) 
	{
		SaveFolderIndex(nFolder);
		nFolder=nFolder->NextFolder;
	}
}

void DeleteUnit(BarUnit* uni)
{
if (MessageDlg(BarhWnd,"Дествительно удалить ярлык ",uni->Caption->Text,"Удаление ярлыка")==IDNO) return;
DeleteFile(uni->FilePath->Text);
uni->ParentFolder->Modified=TRUE;
nUnit=uni->PrevUnit;
nFolder=uni->ParentFolder;
if (uni->NextUnit!=NULL) uni->NextUnit->PrevUnit=nUnit;
if (nUnit!=NULL) nUnit->NextUnit=uni->NextUnit;
else nFolder->FirstUnit=uni->NextUnit;
MySound();
delete uni;
nFolder->Kol--;
PlaceIcon();
InvalidateRect(BarhWnd,NULL,TRUE);
}

void SaveFolderDat()
{	
	HANDLE	hFile;
 
	lstrcpy(buf,MainBarDir);
	lstrcat(buf,"folder.dat");
	hFile = CreateFile(buf,GENERIC_WRITE, 0,0, CREATE_ALWAYS,0,NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		char* f=(char*)malloc(250);
		WriteFile(hFile,&folderlab,4,&dwd,NULL);
		nFolder=FirstFolder;
		while (nFolder)
		{
			lstrcpy(f,nFolder->Path->Text);
			char* i=f+lstrlen(f)-1;
			*i=0;
			i--;
			while (*i!=92) i--;
			i++;
			WriteString(hFile,i);
			WriteFile(hFile,&(nFolder->BkColor),sizeof(COLORREF),&dwd,NULL);
			WriteFile(hFile,&(nFolder->BkColor2),sizeof(COLORREF),&dwd,NULL);
			WriteFile(hFile,&(nFolder->TitColor),sizeof(COLORREF),&dwd,NULL);
			buf[0]=nFolder->Alpha;
			WriteFile(hFile,buf,10,&dwd,NULL);

			nFolder=nFolder->NextFolder;
		}
		free (f);
		DWORD dwd;
		byte Kol=0;
		WriteFile(hFile,&Kol,1,&dwd,NULL);
		CloseHandle(hFile);
	}
}

void DeleteFolder(PBarFolder fol)
{
if (MessageDlg(BarhWnd,"Действительно удалить раздел",fol->Caption->Text,"Удаление раздела")==IDNO) return;
nUnit=fol->FirstUnit;
BarUnit*	oldUnit;
while (nUnit)
{
        oldUnit=nUnit->NextUnit;
        delete nUnit;
        nUnit=oldUnit;
}
lstrcpy(buf,fol->Path->Text);
buf[lstrlen((LPTSTR)&buf)-1]=0;
SHFILEOPSTRUCT	Struc;
Struc.hwnd=BarhWnd;
Struc.wFunc=FO_DELETE;
Struc.pFrom=(LPCTSTR)&buf;
Struc.pTo=NULL;
Struc.fFlags= FOF_NOCONFIRMATION | FOF_RENAMEONCOLLISION;
Struc.fAnyOperationsAborted= FALSE;
Struc.hNameMappings=NULL;
Struc.lpszProgressTitle= NULL;
SHFileOperation(&Struc);

nFolder=fol->PrevFolder;

if (fol->NextFolder!=NULL) fol->NextFolder->PrevFolder=nFolder;
if (nFolder!=NULL) nFolder->NextFolder=fol->NextFolder;
else FirstFolder=fol->NextFolder;
MySound();
nFolder=fol->NextFolder;
while (nFolder!=NULL) 
{
        nFolder->Index--;
        nFolder=nFolder->NextFolder;
}
delete fol->Caption;
delete fol->Path;
ActiveFolder=fol->NextFolder;
free(fol);
if (ActiveFolder==NULL) ActiveFolder=FirstFolder;
SaveBarStruct();
SaveFolderDat();
nFolder=FirstFolder;
byte Kol=0;
byte Ind=1;
while (nFolder!=NULL) 
{
	Kol++;
	if (nFolder==ActiveFolder) Ind=Kol;
	nFolder=nFolder->NextFolder;
}
Switch->SetValue(Ind,Kol);
PlaceIcon();
}

void RenameFolder(PBarFolder fol)
{
	ZeroMemory(buf,1000);
	if (!InputQuery(BarhWnd,"Переименование раздела","Введите новое название раздела",fol->Caption->Text,buf,100)) return;
	if (lstrlen(buf)==0) return;
	nFolder=FirstFolder;
	while (nFolder!=NULL) 
	{
		if (lstrcmpi(nFolder->Caption->Text,buf)==0 && nFolder!=fol) 
		{
		    MessageBox(BarhWnd,"Раздел с таким именем уже существует","Ошибка",MB_OK | MB_ICONSTOP);
            return;
		}
        nFolder=nFolder->NextFolder;
	}
	lstrcpy(buf2,MainBarDir);
	lstrcat(buf2,buf);
	lstrcat(buf2,"\x5c");
	MoveFile(fol->Path->Text,buf2);
	*fol->Path=buf2;
	*fol->Caption=buf;
	SaveFolderDat();
	Switch->SetValue(fol->Index,fol->Index);
}

void NewFolder()
{
	ZeroMemory(buf,1000);
	if (!InputQuery(BarhWnd,"Новый раздел","Введите название нового раздела",NULL,buf,100)) return;
	if (lstrlen(buf)==0) return;
	nFolder=FirstFolder;
	PBarFolder lastFol=NULL;
	byte Kol=1;
	while (nFolder!=NULL) 
	{
		if (lstrcmpi(nFolder->Caption->Text,buf)==0) 
		{
		    MessageBox(BarhWnd,"Раздел с таким именем уже существует","Ошибка",MB_OK | MB_ICONSTOP);
            return;
		}
		Kol++;
        lastFol=nFolder;
        nFolder=nFolder->NextFolder;
	}
	nFolder=CreateFolder(lastFol);
	if (lastFol==NULL) FirstFolder=nFolder;
	nFolder->Modified=TRUE;
	lstrcpy(buf2,MainBarDir);
	lstrcat(buf2,buf);
	lstrcat(buf2,"\x5c");
	*nFolder->Path=buf2;
	CreateDirectory(nFolder->Path->Text,NULL);
	*nFolder->Caption=buf;
	nFolder->Alpha=0;
	if (ActiveFolder!=NULL) HideButton(ActiveFolder->FirstUnit);
	ActiveFolder=nFolder;
	SaveBarStruct();
	SaveFolderDat();
	Switch->SetValue(Kol,Kol);
	PlaceIcon();
}

PBarFolder FindUnitInFolder(BarUnit* nUnit, PBarFolder fol)
{
	BarUnit* nn;
	if (!fol) return NULL;
	nn=fol->FirstUnit;
	while (nn)
	{
		if (nn==nUnit) return fol;
		nn=nn->NextUnit;
	}
	return NULL;
}

void MoveShortCutToFolder(BarUnit* nUnit, PBarFolder nFolder)
{
	lstrcpy(buf,nUnit->FilePath->Text);
	ExtractFileName(nUnit->FilePath->Text,buf2);
	CheckExistsUnitName(buf2);
	char*	temp=(char*)malloc(300);
	lstrcpy(temp,nFolder->Path->Text);
	lstrcat(temp,buf2);
	*nUnit->FilePath=temp;
	free(temp);
	MoveFile(buf,nUnit->FilePath->Text);
}

void UnitDropEnd(BarUnit* nUnit, POINT* lpt)
{
	PBarFolder FromFolder=FindUnitInFolder(nUnit, MainFolder);
	if (!FromFolder)
		FromFolder=FindUnitInFolder(nUnit, ActiveFolder);
	if (!FromFolder) return;
	

	PBarFolder onFolder=FindBarFolder((WORD)lpt->x,(WORD)lpt->y);
	if (!onFolder) return;

	CToolBar2* toolbar;
	if (onFolder==MainFolder) toolbar=MainBar;
	else toolbar=FolderBar;

	RECT rc;
	GetWindowRect(toolbar->GetHWND(),&rc);
	if ((lpt->x<rc.left) || (lpt->x>rc.right) || (lpt->y<rc.top) || (lpt->y>rc.bottom)) return;

	BarUnit* nPrevUnit=(BarUnit*)toolbar->GetPrevUnit((WORD)(lpt->x-rc.left),(WORD)(lpt->y-rc.top));
	if (nPrevUnit) if (nPrevUnit->NextUnit==nUnit) return;

	//Удаляем значок из старого места
	if (nUnit->PrevUnit) nUnit->PrevUnit->NextUnit=nUnit->NextUnit;
	else FromFolder->FirstUnit=nUnit->NextUnit;
	if (nUnit->NextUnit) nUnit->NextUnit->PrevUnit=nUnit->PrevUnit;
	FromFolder->Kol--;

	if (FromFolder!=onFolder) //Перемещаем файл ярлыка в другой каталог если надо
	{
		MoveShortCutToFolder(nUnit, nFolder);
	}

	onFolder->Kol++;
	if (nPrevUnit)
	{
		nUnit->NextUnit=nPrevUnit->NextUnit;
		if (nUnit->NextUnit) nUnit->NextUnit->PrevUnit=nUnit;
		nPrevUnit->NextUnit=nUnit;
	}
	else
	{
		nUnit->NextUnit=onFolder->FirstUnit;
		onFolder->FirstUnit=nUnit;
		nUnit->PrevUnit=NULL;
	}
	nUnit->PrevUnit=nPrevUnit;
	nUnit->ParentFolder=onFolder;

	onFolder->Modified=TRUE;
	FromFolder->Modified=TRUE;
	PlaceIcon();
	SaveMainBitmap();
	SaveBarStruct();
	if (onFolder) InvalidateRect(BarhWnd,NULL,TRUE);
}

void RenameUnit(BarUnit* uni)
{
if (!InputQuery(BarhWnd,"Переименование","Введите новое название",uni->Caption->Text,buf,100)) return;
if (lstrcmpi(buf,uni->Caption->Text)==0 || lstrlen(buf)<2) return;
*uni->Caption=buf;
ExtractFileExt(uni->FilePath->Text,buf2);
lstrcat(buf,buf2);
ExtractFilePath(uni->FilePath->Text,buf2);
lstrcat(buf2,buf);
CheckExistsUnitName(buf2);
MoveFile(uni->FilePath->Text,buf2);
*uni->FilePath=buf2;
PlaceIcon();
uni->ParentFolder->Modified=TRUE;
SaveBarStruct();
}

BarUnit::BarUnit()
{
ButImage=NULL;
BigImage=NULL;
Splitter = NULL;
ParentFolder = NULL;
PrevUnit = NULL;
FilePath=new CMyString();
Caption=new CMyString();
IconPath=new CMyString();
strPath=new CMyString();
IconIndex = 0;
NextUnit = NULL;
Hidden = 0;
}

void BarUnit::DeleteBitmap()
{
	if (ButImage!=NULL) 
	{
		delete ButImage;
		ButImage=NULL;
	}
	if (BigImage!=NULL) 
	{
		delete BigImage;
		BigImage=NULL;
	}
}

BarUnit::~BarUnit()
{
	DeleteBitmap();
	delete FilePath;
	delete Caption;
	delete IconPath;
	delete strPath;
}
