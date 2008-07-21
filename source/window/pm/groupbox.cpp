/***
Filename: grpbox.cpp
Description:
  Implementation of FGroupBox
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_GROUPBOX
#include <bif.h>

Bool FGroupBox::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText)
{
        return FControl::Create(pParent, ID,
                                WC_STATIC, pszText,
                                WS_GROUP|SS_GROUPBOX, rect);
}

