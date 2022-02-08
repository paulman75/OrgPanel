#include <windows.h>
#include "textf.h"

// This structure is used by FillOut for passing data into LineDDA
typedef struct tagSEG {
  DWORD dwPos;
  LPPOINT lpPoints; 
} PTS;

#define MAXFILLPOINTS 64000
COLORREF TextColor,OColor;

VOID CALLBACK AddSegment(int x, int y, PTS *pts)
{
   pts->lpPoints[pts->dwPos].x = x;
   pts->lpPoints[pts->dwPos].y = y;
   pts->dwPos++;
} 

VOID CALLBACK CountPoints(int x, int y, LPDWORD lpdwNumPts)
{
   (*lpdwNumPts)++;
} 

BOOL FillOut(LPPOINT *lpPoints, LPDWORD lpdwNumPts)
{
   DWORD i;
   LPPOINT lpPts = *lpPoints;
   PTS pts;
   DWORD dwNumPts = 0;

   // Make sure we have at least two points
   if (*lpdwNumPts < 2) {
     MessageBox(NULL, "You need at least two points for a guide line.", "Not enough points!", MB_ICONSTOP);
     return FALSE;
   }

   // Find out how namy points are on the segments
   for (i=0; i<*lpdwNumPts - 1; i++) 
     LineDDA(lpPts[i].x, lpPts[i].y, lpPts[i+1].x, lpPts[i+1].y, (LINEDDAPROC)CountPoints, (LPARAM)&dwNumPts);
   
   // If there are too many points, print an anoying message so the user doesnt do it again
   if (dwNumPts > MAXFILLPOINTS) {
     MessageBox(NULL, "Make your guide lines a bit shorter please!", "Too many points!", MB_ICONSTOP);
	 return FALSE;  // Bail
   }
   
   // Allocate memory for the the points and initialize our "last point" index
   pts.lpPoints = (LPPOINT)GlobalAlloc(GPTR, dwNumPts * sizeof(POINT));
   pts.dwPos = 0;
                                                                    
   // Convert the segments to points
   for (i=0; i<*lpdwNumPts - 1; i++) 
     LineDDA(lpPts[i].x, lpPts[i].y, lpPts[i+1].x, lpPts[i+1].y, (LINEDDAPROC)AddSegment, (LPARAM)&pts);
   
   // Get rid of the original array of segments...
   GlobalFree(lpPts);
 
   // ... and replace it with the new points
   *lpPoints = pts.lpPoints;
   *lpdwNumPts = pts.dwPos;
   
   // Check to see if anything hit the fan
   if (!pts.dwPos)
     return FALSE;

   return TRUE;  
}

BOOL RenderPathPoints(HDC hDC, LPPOINT lpPoints, LPBYTE lpTypes, int iNumPts, BOOL bOutline)
{
  int i;
  
  BeginPath(hDC); // Draw into a path so that we can use FillPath()
  for (i=0; i<iNumPts; i++) 
    switch (lpTypes[i]) {
      case PT_MOVETO : 
         MoveToEx(hDC, lpPoints[i].x, lpPoints[i].y, NULL); 
         break;
      
      case PT_LINETO | PT_CLOSEFIGURE:
      case PT_LINETO : 
         LineTo(hDC, lpPoints[i].x, lpPoints[i].y); 
         break;
      
      case PT_BEZIERTO | PT_CLOSEFIGURE:
      case PT_BEZIERTO :
	     PolyBezierTo(hDC, &lpPoints[i], 3);
		 i+=2;
         break;
   }
  
  CloseFigure(hDC);
  EndPath(hDC);

  HPEN hp=CreatePen(PS_SOLID,1,OColor);
  HPEN hPen = (HPEN)SelectObject(hDC, hp); // Just say no to outlines
		
  HBRUSH hb=CreateSolidBrush(TextColor);
  HBRUSH hBrush = (HBRUSH)SelectObject(hDC, hb); // Paint It Black

  StrokeAndFillPath(hDC);

        // Restore the DC to its previous state
  SelectObject(hDC, hPen);
  SelectObject(hDC, hBrush);
  DeleteObject(hb);
  DeleteObject(hp);
  return TRUE;
}

void GetRealTextExtent(LPPOINT lpPoints, int iNumPts, LPSIZE size)
{
  int i;

  for (i=0; i<iNumPts; i++) {
    if (lpPoints[i].x > size->cx) size->cx = lpPoints[i].x;
    if (lpPoints[i].y > size->cy) size->cy = lpPoints[i].y;
  }
}

BOOL TextEffect(HDC hDC,            // DC to display into
                LPPOINT lpTop,  	// Top guide line
                LPPOINT lpBot, 		// Bottom guide line
                DWORD dwTopPts, 	// Number of points in top guide
                DWORD dwBotPts, 	// Number of points in bottom guide
                LPSTR szText, 		// Text string to apply effects to
                COLORREF OCol,
				COLORREF color)  // Print as outline or as solid text
{
   LPPOINT lpPoints;		  // Path data points
   LPBYTE lpTypes;			  // Path data types
   int i, iNumPts;
   SIZE size;				  // Text size info
   float fXScale, fYScale;	  // Scaling values
   int iTopInd, iBotInd;	  // Guide array indices

   TextColor=color;
   OColor=OCol;
   // Set to transparent so we dont get an outline around the text string
   SetBkMode(hDC, TRANSPARENT);
   // Output the text into a path
   BeginPath(hDC);
   TextOut(hDC, 0, 0, szText, lstrlen(szText));
   EndPath(hDC);
   // How many points are in the path
   iNumPts = GetPath(hDC, NULL, NULL, 0);
   if (iNumPts == -1) return FALSE;

   // Allocate room for the points
   lpPoints = (LPPOINT)GlobalAlloc(GPTR, sizeof(POINT) * iNumPts);
   if (!lpPoints) return FALSE;

   // Allocate room for the point types
   lpTypes = (byte*)GlobalAlloc(GPTR, iNumPts);
   if (!lpTypes) {
     GlobalFree(lpPoints);
  	 return FALSE;
   }
   
   // Get the points and types from the current path
   iNumPts = GetPath(hDC, lpPoints, lpTypes, iNumPts);

   // Even more error checking
   if (iNumPts == -1) {
     GlobalFree(lpTypes);
     GlobalFree(lpPoints);
	 return FALSE;
   }
   
   //Get extents of the text string for scaling purposes  
   GetTextExtentPoint32(hDC, szText, strlen(szText), &size);

   // OK, but lets make sure our extents are big enough (handle italics fonts)
   GetRealTextExtent(lpPoints, iNumPts, &size);
   
   // Relocate the points in the path based on the guide lines
   for (i=0; i < iNumPts; i++) {
   	 // How far along is this point on the x-axis
     fXScale = (float)lpPoints[i].x / (float)size.cx;

     // What point on the top guide does this coorespond to
     iTopInd = (int)(fXScale * (dwTopPts-1));
	 // What point on the bottom guide does this coorespond to
     iBotInd = (int)(fXScale * (dwBotPts-1));

     // How far along is this point on the y-axis
     fYScale = (float)lpPoints[i].y / (float)size.cy;

     // Scale the points to their new locations
     lpPoints[i].x = (int)((lpBot[iBotInd].x * fYScale) + (lpTop[iTopInd].x * (1.0f-fYScale)));
     lpPoints[i].y = (int)((lpBot[iBotInd].y * fYScale) + (lpTop[iTopInd].y * (1.0f-fYScale)));
   }

   // Draw the new path 
   RenderPathPoints(hDC, lpPoints, lpTypes, iNumPts, FALSE);
   
   GlobalFree(lpPoints);
   GlobalFree(lpTypes);

   return TRUE;
}
