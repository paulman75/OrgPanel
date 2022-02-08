#include "SensorLb.h"

class CDateLabel : CSensorLabel
{
protected:
 private:
	 void BeginWindow(HWND);
	 HWND hCalWnd;
	 BOOL bFCanNo;
 public:
	 CDateLabel*	next;
	 TDate Date;
	 HWND GetWnd() { return hWnd; };
	 void SetDate(PDate);
     LONG FAR PASCAL SensorProc(UINT msg, WPARAM wParam, LPARAM lParam);
	 CDateLabel(HWND, int, int, PDate, byte,BOOL);
	//Owner, XCenter, YCenter, OFFColor, OnColor, Caption
	//Owner, XCenter, YCenter, Day, Month, Year
	~CDateLabel();
};

