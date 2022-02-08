#include "windows.h"
#include "MyRich.h"
#define _MYDLL_
#include "../Headers/general.h"
#include "res/resource.h"

extern HINSTANCE hInstance;
extern HFONT	hMainFont;
static int		MyRichEditCounts=0;
static HCURSOR	hRichCursor=NULL;
static HCURSOR	hArrowCursor=NULL;
static HMENU	hRichMenu=NULL;
static HMENU	hRichMenu2=NULL;
static CMyRichEdit*	FirstRich=NULL;
CMyRichEdit*	ri;

void __stdcall CreateRichEdit(HWND hwnd, int x, int y, WORD w, WORD h, byte ID)
{
	ri=new CMyRichEdit(hwnd,x,y,w,h,ID);
	ri->next=NULL;
	if (FirstRich)
	{
		CMyRichEdit* r=FirstRich;
		while (r->next) r=r->next;
		r->next=ri;
	}
	else FirstRich=ri;
}

CMyRichEdit* FindRichEdit(byte ID)
{
	ri=FirstRich;
	while (ri)
	{
		if (GetWindowLong(ri->GetWnd(),GWL_ID)==ID) return ri;
		ri=ri->next;
	}
	return NULL;
}

HWND __stdcall GetRichEditWnd(byte ID)
{
	ri=FindRichEdit(ID);
	if (!ri) return NULL;
	return ri->GetWnd();
}

LONG __stdcall RichEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, byte ID)
{
	ri=FindRichEdit(ID);
	if (!ri) return 0;
	return ri->RichWndProc(hwnd,msg,wParam,lParam);
}

void __stdcall DeleteAllRichEdit()
{
	while (FirstRich)
	{
		ri=FirstRich->next;
		delete FirstRich;
		FirstRich=ri;
	}
}

void __stdcall SetRichEditText(byte ID, LPCTSTR lpstrText)
{
	ri=FindRichEdit(ID);
	if (!ri) return;
	ri->SetText(lpstrText);
}

void __stdcall AddTextToRichEdit(byte ID, LPCTSTR lpstrText)
{
	ri=FindRichEdit(ID);
	if (!ri) return;
	ri->AddText(lpstrText);
}

void CMyRichEdit::SetText(LPCTSTR lpstrText)
{
	bChanging=TRUE;
	SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)lpstrText);
	CHARRANGE cr;
	cr.cpMin=0;
	cr.cpMax=lstrlen(lpstrText);
	DetectUrls(&cr);
	ClearUndo();
	SetUndo();
	cr.cpMin=cr.cpMax=0;
	SendMessage(hWnd,EM_EXSETSEL,0,(LPARAM)&cr);
	SendMessage(hWnd,EM_SCROLL, SB_LINEUP, 0);
	bChanging=FALSE;
}

void CMyRichEdit::AddText(LPCTSTR lpstrText)
{
	int len=GetWindowTextLength(hWnd);
	char* buf=(char*)malloc(len+1+lstrlen(lpstrText));
	GetWindowText(hWnd,buf,len+1);
	strcat(buf,lpstrText);
	SetText(buf);
	free(buf);
	len=GetWindowTextLength(hWnd);
	SendMessage(hWnd,EM_SETSEL,len+1,len+1);
	SetUndo();
}

void CMyRichEdit::SetAttrib(int iStart, int iEnd, BOOL bLink, BOOL bAlways)
{
	if (!bAlways && !bLink) return;
	SendMessage(hWnd,EM_SETSEL,iStart,iEnd);
	CHARFORMAT f;
	f.dwMask=CFM_LINK | CFM_UNDERLINE | CFM_COLOR;
	f.cbSize=sizeof(f);
	if (bLink) f.dwEffects=CFE_LINK | CFE_UNDERLINE;
	else f.dwEffects=0;
	f.crTextColor=bLink ? 0xff0000 : 0;
    SendMessage(hWnd,EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&f);
}

void CMyRichEdit::DetectUrls(CHARRANGE* cr)
{
	int FirstLine=SendMessage(hWnd,EM_GETFIRSTVISIBLELINE,0,0);
	TEXTRANGE tr;
	tr.chrg=*cr;
	tr.chrg.cpMin++;
	int i;
	char ch[15];
	
	while (TRUE)
	{
		tr.chrg.cpMax=tr.chrg.cpMin;
		tr.chrg.cpMin-=15;
		if (tr.chrg.cpMin<0) tr.chrg.cpMin=0;
		tr.lpstrText=(char*)&ch;
		SendMessage(hWnd,EM_GETTEXTRANGE,0,(LPARAM)&tr);
		i=lstrlen(ch);
		while (i) if (ch[i-1]==32 && (i+tr.chrg.cpMin)<=cr->cpMin) break; else i--;
		if (i)
		{
			tr.chrg.cpMin+=i;
			break;
		}
		if (!tr.chrg.cpMin) break;
	}
	cr->cpMin=tr.chrg.cpMin;
	tr.chrg.cpMax=cr->cpMax;
	WORD len=GetWindowTextLength(hWnd);
	while (TRUE)
	{
		tr.chrg.cpMin=tr.chrg.cpMax;
		tr.chrg.cpMax+=15;
		if (tr.chrg.cpMax>len) tr.chrg.cpMax=len;
		tr.lpstrText=(char*)&ch;
		SendMessage(hWnd,EM_GETTEXTRANGE,0,(LPARAM)&tr);
		i=lstrlen(ch);
		while (i) if (ch[i-1]==32 && (i+tr.chrg.cpMin)>=cr->cpMax) break; else i--;
		if (i)
		{
			tr.chrg.cpMax=tr.chrg.cpMin+i;
			break;
		}
		if (tr.chrg.cpMax==len) break;
	}
	cr->cpMax=tr.chrg.cpMax;
	if (cr->cpMax==cr->cpMin) return;
	char* temp=(char*)malloc(cr->cpMax-cr->cpMin+1);
	tr.chrg=*cr;
	tr.lpstrText=temp;
	SendMessage(hWnd,EM_GETTEXTRANGE,0,(LPARAM)&tr);
	BOOL bModify=SendMessage(hWnd,EM_GETMODIFY,0,0);
	i=0;
	int j=0;
	bChanging=TRUE;
	int kk=SendMessage(hWnd,WM_SETREDRAW,0,0);
	SetAttrib(tr.chrg.cpMin,tr.chrg.cpMax,FALSE,TRUE);
	if (kk) j++;
	len=lstrlen(temp);
	while (i<len)
	{
		while (temp[i]!=32 && temp[i]!=0x0a && i<len) i++;
		if (i!=j)
		{
			byte index=0;
			char urls[10][8]={"www.","http:","ftp:","mailto:","file:","https:","nntp:","gopher:","telnet:","news:"};
			BOOL burl=FALSE;
			while (index<9)
			{
				if (burl=FindInString2((byte*)(temp+j),urls[index++],i-j,FALSE,FALSE)!=NULL) break;
			}
			if (!burl)
			{
				char *t=(char*)FindInString2((byte*)(temp+j),"@",i-j,FALSE,FALSE);
				if (t && t!=(temp+j))
				{
					char *t2=(char*)FindInString2((byte*)t,".",i-(int)t+(int)temp,FALSE,FALSE);
					if (t2 && (t2<temp+i)) burl=TRUE;
				}
			}
			SetAttrib(cr->cpMin+j,cr->cpMin+i,burl);
			i--;
		}
		i++;
		j=i;
	}
	SendMessage(hWnd,EM_SETSEL,0,0);
	FirstLine-=SendMessage(hWnd,EM_GETFIRSTVISIBLELINE,0,0);
	if (FirstLine)
		SendMessage(hWnd,EM_LINESCROLL,0,FirstLine);
	SendMessage(hWnd,WM_SETREDRAW,1,0);
	bChanging=FALSE;
	SendMessage(hWnd,EM_GETMODIFY,bModify,0);
	InvalidateRect(hWnd,NULL,FALSE);
}

void CMyRichEdit::SetUndo()
{
	if (UndoCount==MAX_UNDO)
	{
		if (pUndo[0]) free(pUndo[0]);
		for (int i=1; i<MAX_UNDO; i++)
		{
			pUndo[i-1]=pUndo[i];
			UndoCur[i-1]=UndoCur[i];
		}
		UndoCount--;
	}
	int len=SendMessage(hWnd,WM_GETTEXTLENGTH,0,0)+1;
	pUndo[UndoCount]=(char*)malloc(len);
	SendMessage(hWnd,WM_GETTEXT,len,(LPARAM)pUndo[UndoCount]);
	SendMessage(hWnd,EM_GETSEL,(WPARAM)&UndoCur[UndoCount],0);
	UndoCount++;
}

void CMyRichEdit::ClearUndo()
{
	for (int i=0; i<UndoCount; i++)	if (pUndo[i]) free(pUndo[i]);
	UndoCount=0;
}

void CMyRichEdit::MakeUndo()
{
	if (UndoCount<2) return;
	UndoCount--;
	bChanging=TRUE;
	SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)pUndo[UndoCount-1]);
	SendMessage(hWnd,EM_SETSEL,UndoCur[UndoCount-1],UndoCur[UndoCount-1]);
	bChanging=FALSE;
	crOld.cpMax=crOld.cpMin=0;
}

void CMyRichEdit::PasteFromClipboard()
{
	OpenClipboard(hWnd);
	HANDLE hh=GetClipboardData(CF_UNICODETEXT);
	LPVOID lpvoid=GlobalLock(hh);
	int lenw=lstrlenW((LPCWSTR)lpvoid);
	char * temp=(char*)malloc(lenw+1);
	WideCharToMultiByte(CP_ACP,0,(LPCWSTR)lpvoid,-1,temp,lenw+1,NULL,NULL);
	SendMessage(hWnd,EM_REPLACESEL,0,(LPARAM)temp);
	free(temp);
	GlobalUnlock(hh);
	CloseClipboard();
}

void CMyRichEdit::OnChange()
{
	HKL hkl=GetKeyboardLayout(0);
	CHARRANGE	cr;
	SendMessage(hWnd,EM_EXGETSEL,0,(LPARAM)&cr);
	CHARRANGE	crMax=cr;
	if (crOld.cpMin<cr.cpMin) crMax.cpMin=crOld.cpMin;
	if (crOld.cpMax>cr.cpMax) crMax.cpMax=crOld.cpMax;
	crOld=cr;
	DetectUrls(&crMax);
	SendMessage(hWnd,EM_EXSETSEL,0,(LPARAM)&cr);
	SetUndo();
	ActivateKeyboardLayout(hkl,0);
}

CMyRichEdit::CMyRichEdit(HWND Owner, WORD x, WORD y, WORD cx, WORD cy, byte MyRichID)
{
	if (!MyRichEditCounts++)
	{
		hRichModule=LoadLibraryA("RICHED32.DLL");
		if (!hRichModule) hRichModule=LoadLibraryA("RICHED20.DLL");
		hRichCursor=LoadCursor(hInstance,"HANDCUR");	
		hArrowCursor=LoadCursor(NULL,IDC_ARROW);	
		hRichMenu=LoadMenu(hInstance,"RICHMENU");
        hRichMenu2=GetSubMenu(hRichMenu,0);
	}
	UndoCount=0;
	bChanging=FALSE;
	hWnd=CreateWindow("RICHEDIT","",WS_BORDER | WS_VSCROLL | WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_NOHIDESEL | 0x8044,x,y,cx,cy,Owner,NULL,hInstance,NULL);

	SendMessage(hWnd,WM_SETFONT,(LPARAM)hMainFont,0);
	CHARFORMAT cf;
	ZeroMemory(&cf,sizeof(CHARFORMAT));
	cf.cbSize=sizeof(CHARFORMAT);
	cf.dwMask= CFM_CHARSET | CFM_BOLD | CFM_FACE;
	SendMessage(hWnd,EM_GETCHARFORMAT,TRUE,(LPARAM)&cf);
	cf.bCharSet=DEFAULT_CHARSET;
	cf.dwEffects=cf.dwEffects & ~CFM_BOLD;
	strcpy(cf.szFaceName,"MS Sans Serif");
	SendMessage(hWnd,EM_SETCHARFORMAT,SCF_DEFAULT,(LPARAM)(CHARFORMAT FAR *)&cf);
	SendMessage(hWnd,EM_SETMARGINS, EC_LEFTMARGIN,5);
	SendMessage(hWnd,EM_SETMARGINS, EC_RIGHTMARGIN,5);

	SetWindowLong(hWnd, GWL_ID, MyRichID);
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
	hOldProc=(WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC,(LONG)&StaticWndProc);
	SendMessage(hWnd,EM_SETEVENTMASK,0,ENM_LINK | ENM_CHANGE);
	crOld.cpMax=crOld.cpMin=0;
	SetUndo();
};

CMyRichEdit::~CMyRichEdit()
{
	DestroyWindow(hWnd);
	ClearUndo();
	if (!--MyRichEditCounts)
	{
		FreeLibrary(hRichModule);
		DestroyCursor(hRichCursor);
		DestroyCursor(hArrowCursor);
		DestroyMenu(hRichMenu);
	}
};

LRESULT CALLBACK CMyRichEdit::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMyRichEdit* ri=(CMyRichEdit*)GetWindowLong(hwnd,GWL_USERDATA);
	return ri->OwnWndProc(hwnd,msg,wParam,lParam);
}

LONG CMyRichEdit::OwnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bMenuShowing=FALSE;
	DWORD dws,dwe;
	switch (msg)
	{
	case WM_KEYDOWN:
		if ((int)wParam=='V')
		{
			short s1=GetKeyState(VK_RCONTROL);
			if ((s1&0x8000)==0) s1=GetKeyState(VK_LCONTROL);
			if ((s1&0x8000)==0) break;
			PasteFromClipboard();
			return 0;
		}
		if ((int)wParam=='Z')
		{
			short s1=GetKeyState(VK_RCONTROL);
			if ((s1&0x8000)==0) s1=GetKeyState(VK_LCONTROL);
			if ((s1&0x8000)==0) break;
			MakeUndo();
			return 0;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_UNDO:
			MakeUndo();
			break;
		case IDC_COPY:
		    SendMessage(hWnd, WM_COPY, 0, 0);
			break;
		case IDC_CUT:
		    SendMessage(hWnd, WM_CUT, 0, 0);
			break;
		case IDC_PASTE:
			PasteFromClipboard();
			break;
		case IDC_DELETE:
			SendMessage(hWnd, WM_CLEAR, 0, 0);
			break;
		case IDC_SELECTALL:
			SendMessage(hWnd, EM_SETSEL, 0, -1);
			break;
		}
		break;
	case WM_SETCURSOR:
		if (bMenuShowing)
		{
			SetCursor(hArrowCursor);
			return TRUE;
		}
		break;
	case WM_EXITMENULOOP:
		bMenuShowing=FALSE;
		break;
	case WM_RBUTTONDOWN:
		SendMessage(hWnd,WM_CANCELMODE,0,0);
		break;
	case WM_RBUTTONUP:
		POINT point;
		GetCursorPos(&point);
		bMenuShowing=TRUE;
		SendMessage(hWnd,EM_GETSEL,(WPARAM)&dws,(LPARAM)&dwe);
		EnableMenuItem(hRichMenu,IDC_PASTE, SendMessage(hWnd,EM_CANPASTE,0,0) ? MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hRichMenu,IDC_CUT, dws!=dwe ? MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hRichMenu,IDC_COPY, dws!=dwe ? MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hRichMenu,IDC_DELETE, dws!=dwe ? MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hRichMenu,IDC_UNDO, UndoCount>1 ? MF_ENABLED:MF_GRAYED);
		TrackPopupMenu(hRichMenu2, 0, point.x, point.y, 0, hwnd, NULL);
		break;
	}
	return CallWindowProc(hOldProc, hwnd,msg, wParam, lParam);
}

LONG CMyRichEdit::RichWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		if (HIWORD(wParam)==EN_CHANGE && !bChanging) OnChange();
		break;
	case WM_NOTIFY:
		ENLINK* enl=(ENLINK*)lParam;
		switch (enl->msg)
		{
		case WM_SETCURSOR:
			SetCursor(hRichCursor);
			return 1;
		case WM_LBUTTONDOWN:
			TEXTRANGE tr;
			tr.chrg=enl->chrg;
			char* bb=(char*)malloc(tr.chrg.cpMax-tr.chrg.cpMin+1);
			char* bb2=(char*)malloc(tr.chrg.cpMax-tr.chrg.cpMin+10);
			tr.lpstrText=bb;
			SendMessage(hWnd,EM_GETTEXTRANGE,0,(LPARAM)&tr);
			if (FindInString(bb,"@")!=-1)
			{
				if (FindInString(bb,"mailto:")!=-1) lstrcpy(bb2,bb);
				else
				{
					lstrcpy(bb2,"mailto:");
					lstrcat(bb2,bb);
				}
				ShellExecute(hWnd,"open",bb2,NULL,NULL,SW_SHOW);
			}
			else
			{
				if (FindInString(bb,":")!=-1) GotoUrl(hWnd,bb);
				else
				{
					lstrcpy(bb2,"http://");
					lstrcat(bb2,bb);
					GotoUrl(hWnd,bb2);
				}
			}
			free(bb);
			free(bb2);
			return 1;
		}
		break;
	}
	return 0;
}

