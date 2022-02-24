#include <windows.h>
#include "Units.h"
#include "res\resource.h"
#include "Utils.h"
#include "../Headers/general.h"
#include "magic.h"
#pragma warning(disable : 4192)
#import <shdocvw.dll> 
#include <mshtml.h>

extern HINSTANCE hInstance;
SHDocVw::IShellWindowsPtr m_spSHWinds;
const int BUF_SIZE=200;
extern char	buf[BUF_SIZE];
char name[20][100];
char url[20][200];
int SelName;

void FillIEList(HWND hl)
{
	CoInitialize(NULL);

	if(m_spSHWinds == 0)
	if(m_spSHWinds.CreateInstance(__uuidof(SHDocVw::ShellWindows)) != S_OK) return;

	IDispatchPtr spDisp;
	long nCount = m_spSHWinds->GetCount();

	for (long i = 0; i < nCount; i++)
	{
		_variant_t va(i, VT_I4);
		spDisp = m_spSHWinds->Item(va);

		SHDocVw::IWebBrowser2Ptr spBrowser(spDisp);
		if (spBrowser != NULL)
		{
			_bstr_t str = spBrowser->GetLocationName();
			strcpy_s(name[i],100,(char*)str);
			strcpy_s(buf, BUF_SIZE, (char*)str);
			strcat_s(buf, BUF_SIZE, "  (");
			str= spBrowser->GetLocationURL();
			strcpy_s(url[i],200,(char*)str);
			strcat_s(buf, BUF_SIZE, (char*)str);
			strcat_s(buf, BUF_SIZE,")");
			SendMessage(hl,LB_ADDSTRING,0,(LPARAM)buf);
		}
	}
	SendMessage(hl,LB_SETCURSEL,0,0);
}

BOOL FAR PASCAL MagicDlgProc(HWND hDlg, unsigned message, WPARAM wParam, LPARAM lParam)
{
	int i;
    switch (message) {
	case WM_INITDIALOG:
		PutOnCenter(hDlg);
		FillIEList(GetDlgItem(hDlg,IDC_LIST1));
		i=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
		EnableWindow(GetDlgItem(hDlg,IDC_MADD),i!=-1);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_MADD:
			SelName=SendMessage(GetDlgItem(hDlg,IDC_LIST1),LB_GETCURSEL,0,0);
			EndDialog(hDlg,TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg,FALSE);
			return TRUE;
		}
	}
	return FALSE;
}

void MakeMagic()
{
	if (DialogBox(hInstance,"MAGICDLG",NULL,MagicDlgProc))
	{
		AddUnit(name[SelName],url[SelName]);
	}
}
