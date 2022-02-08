#include <windows.h>
#include "Units.h"
#include "res\resource.h"
#include "Utils.h"
#include "Login.h"
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "../OrgPanel/ToolBar.h"
#include "../Headers/ExTreeView.h"
#include <commctrl.h>
#include "magic.h"
#include "../Headers/BitMenu.h"

char MainDir[200];
HINSTANCE hInstance;
HWND hMainDlg;
extern BOOL	Modif;
HWND hWndToolTip=NULL;
CToolBar*	bar;
CBitMenu*	menu;
CExTreeView* trView=NULL;
HPEN		hGrayPen,hWhitePen;
HBITMAP	 hBitNewSite;
HBITMAP	 hBitNewGroup;
HBITMAP  hBitRename;
HBITMAP  hBitDel;

void OnGoto()
{
	char* url=(char*)malloc(300);
	SendMessage(GetDlgItem(hMainDlg,IDC_URL),WM_GETTEXT,300,(LPARAM)url);
	ShellExecute(hMainDlg,"open",url,NULL,NULL,SW_NORMAL);
	free(url);
}

BOOL FAR PASCAL MainDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	HICON hi;
	PTreeQuery ptq;
    switch (message) {
	case WM_INITDIALOG:
		hMainDlg=hDlg;
		PutOnCenter(hDlg);
		if (!LoadUnits())
		{
			EndDialog(hDlg,FALSE);
			return FALSE;
		}
		InitCommonControls();
		hWndToolTip=CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_CHILD | WS_POPUP | TTS_ALWAYSTIP, 
            CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, 
            hMainDlg, NULL, hInstance, NULL);
		bar=new CToolBar(hMainDlg,0,0,29,TRUE,hWndToolTip);
		bar->AddButton("Добавить группу",	 IDM_ADDGROUP, "ADDGROUP",	TBSTYLE_BUTTON);
		bar->AddButton("Добавить сайт",	 IDM_ADDSITE, "ADD",	TBSTYLE_BUTTON);
		bar->AddButton("",	IDC_ABOUT+11,"",	TBSTYLE_SEP);
		bar->AddButton("Переименовать",	 IDM_RENAME, "RENAME",	TBSTYLE_BUTTON);
		bar->AddButton("Удалить",	 IDM_DEL, "DEL",	TBSTYLE_BUTTON);
		bar->AddButton("Зайти на сайт",	 IDC_GOTO, "GOTO",	TBSTYLE_BUTTON);
		bar->AddButton("",	IDC_ABOUT+10,"",	TBSTYLE_SEP);
		bar->AddButton("Найти окна Explorer",	 IDC_MAGIC, "MAGIC",	TBSTYLE_BUTTON);
		bar->AddButton("О Программе",	 IDC_ABOUT, "ABOUT",	TBSTYLE_BUTTON);
		hi=LoadIcon(hInstance,"MAINICON");
		SendMessage(hMainDlg,WM_SETICON,ICON_BIG,(LPARAM)hi);
		trView=new CExTreeView(NULL,0,0,0,0,0,GetDlgItem(hDlg,IDC_TREE1));
		trView->SetBackBitmap("BKGND");
		trView->AddImage(LoadBitmap(hInstance,"WEBFOLDER"));
		trView->AddImage(LoadBitmap(hInstance,"WEBICON"));
		trView->hMenu=LoadMenu(hInstance,"TREEMENU");
		trView->hMenu=GetSubMenu(trView->hMenu,0);
		hBitNewSite=LoadBitmap(hInstance,"MENUNEWSITE");
		hBitNewGroup=LoadBitmap(hInstance,"MENUNEWGROUP");
		hBitRename=LoadBitmap(hInstance,"MENURENAME");
		hBitDel=LoadBitmap(hInstance,"MENUDEL");
		menu=new CBitMenu(trView->hMenu, hDlg);
		menu->SetBitmapToItem(hBitNewSite,IDM_ADDSITE);
		menu->SetBitmapToItem(hBitNewGroup,IDM_ADDGROUP);
		menu->SetBitmapToItem(hBitRename,IDM_RENAME);
		menu->SetBitmapToItem(hBitDel,IDM_DEL);

		FillTree();
		ShowInfo(NULL);
		return TRUE;
	case WM_MEASUREITEM:
		if (wParam==0)
		{
			menu->OnMeasureItem((LPMEASUREITEMSTRUCT) lParam);
			return TRUE;
		}
		break;
	case WM_DRAWITEM:
		if (wParam==0)
		{
			menu->OnDrawItem((LPDRAWITEMSTRUCT) lParam);
			return TRUE;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			SaveUnits();
			delete bar;
			delete trView;
			EndDialog(hDlg,TRUE);
			return TRUE;
		case IDM_ADDSITE:
			AddUnit();
			break;
		case IDM_ADDGROUP:
			AddGroup();
			break;
		case IDM_RENAME:
			Rename();
			break;
		case IDM_DEL:
			Del();
			break;
		case IDC_ABOUT:
			ShowAbout(hDlg,"Хранитель паролей.",hInstance);
			break;
		case IDC_URL:
		case IDC_LOGIN:
		case IDC_PASS:
		case IDC_COMMENT:
			if (HIWORD(wParam)==EN_CHANGE) Modif=TRUE;
			break;
		case IDC_GOTO:
			SaveInfo(NULL);
			OnGoto();
			break;
		case IDC_MAGIC:
			MakeMagic();
			break;
		}
		break;
	case WM_INITMENUPOPUP:
		EnableMenuItem(trView->hMenu,IDM_DEL, MF_BYCOMMAND	| (bar->GetEnableButton(IDM_DEL) ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
		EnableMenuItem(trView->hMenu,IDM_RENAME, MF_BYCOMMAND	| (bar->GetEnableButton(IDM_RENAME) ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
		break;
	case WM_NOTIFY:
		LPNMHDR nm;
		nm=(LPNMHDR)lParam;
		if (trView && nm->hwndFrom==trView->GetHWND())
		{
			trView->Notify(nm->code, (LPNMHDR)lParam);
			if (nm->code==TVN_SELCHANGED)
			{
				LPNM_TREEVIEW lptv;
				lptv=(LPNM_TREEVIEW)lParam;
				SaveInfo(lptv->itemOld.hItem);
				ShowInfo(lptv->itemNew.hItem);
			}
		}
		break;
	case TVM_CAN_AS_CHILD:
		ptq=(PTreeQuery)lParam;
		ptq->bResult=ptq->pTarget->ImageIndex==0;
		return TRUE;
	case TVM_DROPPED:
		ptq=(PTreeQuery)lParam;
		OnDropped(ptq->pNode->ImageIndex, ptq->pNode->Data ,ptq->pTarget->ImageIndex, ptq->pTarget->Data);
		ptq->bResult=ptq->pTarget->ImageIndex==0;
		break;
	}
	return FALSE;
}

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow)
{
	hInstance=hInst;
	GetDataDir(MainDir,200);
	if (!CheckPassword()) return 2;
	hWhitePen = CreatePen(PS_SOLID,1,0x0ffffff);
	hGrayPen = CreatePen(PS_SOLID,1,0x808080);
	DialogBox(hInst,"MAINDLG",NULL,MainDlgProc);
	DeleteObject(hWhitePen);
	DeleteObject(hGrayPen);
	DeleteObject(hBitNewSite);
	DeleteObject(hBitNewGroup);
	DeleteObject(hBitRename);
	DeleteObject(hBitDel);
	return 0;
}