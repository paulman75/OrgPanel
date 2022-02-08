#ifndef __COLUMN_H
#define __COLUMN_H

typedef struct
{
	int		iOrder;
	WORD	wWidth;
	BOOL	bVisible;
	BOOL	bInFilter;
} COLUMN, *PCOLUMN;

void ShowColOption();
void LoadColumns();
void UpdateColumns();
void SaveColumns();
void OnChangeHeadWidth(WPARAM wParam, LPARAM lParam);
void OnHideColumn(LPARAM lParam);
void OnChangeOrder(WPARAM wParam, LPARAM lParam);
void CheckSortOrder();

#endif