#ifndef __FILTER_H
#define __FILTER_H

#include "Units.h"

#define IDC_FILTEREDIT	120

BOOL CheckUnitFilter(LPNOTEUNIT);
BOOL CheckFirmFilter(LPFIRMUNIT);
void OnFilterComboSelect();

#endif
