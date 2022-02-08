#include <windows.h>
#include <commctrl.h>
#include "../Headers/general.h"
#include "../Headers/const.h"
#include "units.h"
#include "res/resource.h"
#include <richedit.h>
#include "../Headers/ExTreeView.h"
#include "../Headers/BitMenu.h"

//---------------------------------------------------------------------------
// Function declarations
//---------------------------------------------------------------------------
HWND hwndMain;
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow);
LONG FAR PASCAL FormWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//---------------------------------------------------------------------------
// Global Variables...
//---------------------------------------------------------------------------
HINSTANCE	hInstance;		  // Global instance handle for application
HBITMAP		hBack;
HPEN		hGrayPen,hWhitePen;
HWND		hRich;
HACCEL		accel;
CExTreeView* trView=NULL;
extern BOOL TextModif;
CBitMenu*	menu;
HBITMAP hBitNewGroup, hBitRename, hBitDel;

char MainDir[200];

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR lpstrCmdLine, int cmdShow)
{
    MSG        msgMain;
    WNDCLASS   wc;

	if (FindWindow(NotePadWindow,NULL)) return 0;
    hInstance = hInst;

	GetDataDir(MainDir,200);

		wc.lpszMenuName		= "MAINMENU";
		wc.lpszClassName	= NotePadWindow;
        wc.hInstance        = hInst;
		wc.hIcon			= LoadIcon(hInstance,"MainIcon");
        wc.hCursor          = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
        wc.style            = 0;
		wc.lpfnWndProc		= FormWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;

        if (!RegisterClass(&wc))
            return(0);

		if ((hwndMain = CreateWindowEx(0,NotePadWindow,
				 "ќргѕанель - Ѕлокнот",
                                 WS_OVERLAPPEDWINDOW,
                                 100, 200,
                                 590, 400,
                                 NULL, NULL, hInst, NULL)) == NULL)
        return(0);

	if (!CheckPassword(FALSE, hwndMain))
	{
		DestroyWindow(hwndMain);
		UnregisterClass(NotePadWindow,hInstance);
		return 0;
	}

	PutOnCenter(hwndMain);
	InitCommonControls();
	hBack=LoadBitmap(hInstance,"BACK");
	hWhitePen = CreatePen(PS_SOLID,1,0x0ffffff);
	hGrayPen = CreatePen(PS_SOLID,1,0x808080);

	trView=new CExTreeView(hwndMain,5,65,140,250,0,NULL);
	trView->SetBackBitmap("BKGND");
	trView->AddImage(LoadBitmap(hInstance,"GROUP"));
	trView->hMenu=LoadMenu(hInstance,"TREEMENU");
	trView->hMenu=GetSubMenu(trView->hMenu,0);


	hBitNewGroup=LoadBitmap(hInstance,"GROUP");
	hBitRename=LoadBitmap(hInstance,"MENURENAME");
	hBitDel=LoadBitmap(hInstance,"MENUDEL");

	menu=new CBitMenu(trView->hMenu, hwndMain);
	menu->SetBitmapToItem(hBitNewGroup,IDC_NEW);
	menu->SetBitmapToItem(hBitRename,IDC_RENAME);
	menu->SetBitmapToItem(hBitDel,IDC_DEL);

	CreateRichEdit(hwndMain,160,65,200,250,IDC_RICHEDIT);
	hRich=GetRichEditWnd(IDC_RICHEDIT);
	LoadUnits();
	FillTree();
	int SelID;
	if (ReadRegistry("NotePadID",(LPBYTE)&SelID,4))
	{
		TTreeNode tn;
		if (trView->FindNode(0, SelID, &tn)) trView->SelItem(tn.hHandle);
		else trView->SelItem(trView->GetRootItem());
	}
	else trView->SelItem(trView->GetRootItem());

	accel=LoadAccelerators(hInstance,"ACCEL");
	ShowWindow(hwndMain, SW_SHOW);
    UpdateWindow(hwndMain);	
	SetForegroundWindow(hwndMain);
	SetFocus(hRich);
	UpdateToolBar();
	
    while (GetMessage((LPMSG) &msgMain, NULL, 0, 0))
    {
		if (!TranslateAccelerator(hwndMain,accel,(LPMSG)&msgMain))
		{
			TranslateMessage((LPMSG) &msgMain);
			DispatchMessage((LPMSG) &msgMain);
		}
    }

	DeleteObject(hBack);
	DeleteObject(hGrayPen);
    DeleteObject(hWhitePen);
	DeleteAllRichEdit();
	DeleteObject(hBitNewGroup);
	DeleteObject(hBitRename);
	DeleteObject(hBitDel);
	UsePassword();
    return(0);
}

void MakeConvert(DWORD dir)
{
	UsePassword();
	DWORD st,en;
	int len;
	char* buf;
	SendMessage(hRich,EM_GETSEL,(WPARAM)&st,(LPARAM)&en);
	if (st==en)
	{
		len=SendMessage(hRich,WM_GETTEXTLENGTH,0,0)+1;
		buf=(char*)malloc(len);
		SendMessage(hRich,WM_GETTEXT,len,(WPARAM)buf);
		Convert(dir,buf);
		SetWindowText(hRich,buf);
		free(buf);
	}
	else
	{
		len=en-st+1;
		buf=(char*)malloc(len);
		ZeroMemory(buf,len);
		SendMessage(hRich,EM_GETSELTEXT,0,(LPARAM)buf);
		Convert(dir,buf);
		SendMessage(hRich,EM_REPLACESEL,0,(LPARAM)buf);
		free(buf);
		SendMessage(hRich,EM_SETSEL,st,en);
	}
}

LONG FAR PASCAL FormWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	int i;
	PTreeQuery ptq;

    switch(msg) {
		case WM_ACTIVATE:
			if (LOWORD(wParam)!=WA_INACTIVE)
			{
				LONG l;
				l=DefWindowProc(hwnd, msg, wParam, lParam);
				SetFocus(hRich);
				return l;
			}
			break;
        case WM_DESTROY:
		   SaveUnits();
           PostQuitMessage(0);
	    break;
		case WM_ERASEBKGND:
			GetClientRect(hwnd,&rc);
			FillBackGround((HDC)wParam,&rc,hBack);
			return TRUE;
		case WM_SIZE:
			i=(LOWORD(lParam)-5)/3;
			MoveWindow(trView->GetHWND(),5,5,i,HIWORD(lParam)-10,TRUE);
			MoveWindow(hRich,i+10,5,LOWORD(lParam)-i-15,HIWORD(lParam)-10,TRUE);
			InvalidateRect(hRich, NULL, TRUE);
			break;
		case WM_GETMINMAXINFO:
			LPMINMAXINFO	lpmi;
			lpmi=(LPMINMAXINFO)lParam;
			lpmi->ptMinTrackSize.x=330;
			lpmi->ptMinTrackSize.y=200;
			break;
		case WM_COMMAND:
			if (HIWORD(wParam)==BN_CLICKED || HIWORD(wParam)==1)
			switch (LOWORD(wParam))
			{
			case IDC_EXIT:
				SaveUnits();
				PostQuitMessage(0);
				break;
			case IDC_RENAME:
				RenameUnit();
				break;
			case IDC_NEW:
				AddGroup();
				break;
			case IDC_DEL:
				DeleteUnit();
				break;
			case IDC_HEL:
				char* buf;
				buf=(char*)malloc(230);
				lstrcpy(buf,MainDir);
				lstrcat(buf,"Orgpanel.hlp");
				WinHelp(hwnd,buf,HELP_CONTEXT,0);
				free (buf);
				break;
			case IDC_ABOUT:
				ShowAbout(hwnd,"Ѕлокнот.",hInstance);
				break;
			case ID_WINDOS:
			case ID_WINKOI:
			case ID_DOSWIN:
			case ID_DOSKOI:
			case ID_KOIWIN:
			case ID_KOIDOS:
				MakeConvert(LOWORD(wParam));
				break;
			case ID_SEARCH:
			case ID_SEARCHNEXT:
			case ID_REPLACE:
				Find(LOWORD(wParam));
				break;
			}
			if (LOWORD(wParam)==IDC_RICHEDIT && HIWORD(wParam)==EN_CHANGE)
			TextModif=TRUE;

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
				SaveInfo();
				ShowInfo(lptv->itemNew.hItem);
				UpdateToolBar();
			}
		}
		break;
	case TVM_CAN_AS_CHILD:
		ptq=(PTreeQuery)lParam;
		ptq->bResult=TRUE;
		return TRUE;
	case TVM_DROPPED:
		ptq=(PTreeQuery)lParam;
		OnDropped(ptq->pNode->Data, ptq->pTarget->Data);
		ptq->bResult=TRUE;
		break;
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
	case WM_INITMENUPOPUP:
		EnableMenuItem(trView->hMenu,IDC_DEL, MF_BYCOMMAND	| CanDelete() ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
		break;
	}
    return(DefWindowProc(hwnd, msg, wParam, lParam));
}
