#ifndef __SENSORLABEL__
#define __SENSORLABEL__

class CSensorLabel
{
 private:
	 int FXCenter,FYCenter;
	 byte TimerId;
 protected:
	 BOOL bDownClick;
 public:
	 CSensorLabel*	next;
	 void UpdateSize();
	 char	FCaption[30];
	 int ColorOn,ColorOff;
	 HWND   hWnd;
     virtual LONG FAR PASCAL SensorProc(UINT msg, WPARAM wParam, LPARAM lParam);
	 static LONG CALLBACK StaticSensorProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	CSensorLabel(HWND, int, int, int, int, LPCTSTR, byte, int);
	//Owner, XCenter, YCenter, OFFColor, OnColor, Caption, cursor,id
	~CSensorLabel();
};

#endif