#if !defined (__CTIMER_H)
#define __CTIMER_H

class CTimer
{
 private:
  UINT milli;
  HWND OwnerWindow;
  byte FTag;
  byte id;
 public:
  CTimer(int milliseconds, HWND MainWindow, byte Tag);
  ~CTimer(void);
  virtual void SetInterval(int millis);
  virtual BOOL Start(void); // Start timer
  virtual void Stop(void);                   // Stop timer
  BOOL Working() { return id; };
};
#endif