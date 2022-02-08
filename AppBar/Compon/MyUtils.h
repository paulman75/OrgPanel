#ifndef __MYUTILS__
#define __MYUTILS__

void ExtractFileExt(LPCTSTR path,LPTSTR buf);
void ExtractFileName(LPCTSTR,LPTSTR);
void MyMoveFile(LPCTSTR sFrom, LPCTSTR sTo, LPCTSTR name);
void FillCity(HWND hCombo);
void ChangeCity(HWND hCombo,HWND hD, HWND hS);

CONST LOGFONT lf = {
    16,                        //int   lfHeight;
    0,                         //int   lfWidth;
    0,                         //int   lfEscapement;
    0,                         //int   lfOrientation;
    FW_DONTCARE,               //int   lfWeight;
    0,                         //BYTE  lfItalic;
    0,                         //BYTE  lfUnderline;
    0,                         //BYTE  lfStrikeOut;
    RUSSIAN_CHARSET,           //BYTE  lfCharSet;
    OUT_DEFAULT_PRECIS,        //BYTE  lfOutPrecision;
    CLIP_DEFAULT_PRECIS,       //BYTE  lfClipPrecision;
    DEFAULT_QUALITY,           //BYTE  lfQuality;
    DEFAULT_PITCH | FF_SWISS,  //BYTE  lfPitchAndFamily;
    "MS Sans Serif",           //BYTE  lfFaceName[LF_FACESIZE];
    };

#endif