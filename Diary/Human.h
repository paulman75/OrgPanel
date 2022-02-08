#ifndef __HUMAN_H
#define __HUMAN_H

void InitHuman();
void OnStartExchange(int Size);
void FreeHuman();
void AddRov(PPage pp);
void AddImen(PPage pp);
void FillBioCombo(HWND hwnd);
int GetHumanByKey(DWORD Key);

#endif
