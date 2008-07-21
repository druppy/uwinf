/***
Filename: grpbox.cpp
Description:
  Implementation of FGroupBox
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_GROUPBOX
#include <bif.h>

Bool FGroupBox::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText)
{
        return FControl::Create(pParent,ID,
                                "Button",pszText,
                                WS_GROUP|BS_GROUPBOX, rect);
}

