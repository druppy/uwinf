/***
Filename: keyboardt.cpp
Description:
  Implementation of a FKeyboardHandler
Host:
  Watcom 10.5-10.6
History:
  ISJ 96-05-01 Created
  ISJ 96-05-19 Added TranslateAccelerator()
***/
#define BIFINCL_WINDOW
#define BIFINCL_KEYBOARDHANDLERS
#define BIFINCL_RESOURCETEMPLATE
#define BIFINCL_COMMONHANDLERS
#define WMSINCL_MSGS
#include <bif.h>

Bool FKeyboardHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_KEYBOARD) {
                FKeyboardEvent &ke=(FKeyboardEvent&)ev;
                if(ke.IsCharAvailable())
                        return CharInput(ke);
                else
                        return KeyPressed(ke);
        } else
                return False;
}


Bool TranslateAccelerator(const FAcceleratorTable &accel, const FKeyboardEvent &ev, FEvent &outev) {
        const AcceleratorEntry *ae = (const AcceleratorEntry *)accel.GetAccelTable();
        if(!ae) 
                return False;
        if((!ev.IsCharAvailable()) && (!ev.IsKeyAvailable())) 
                return False;
        Bool isKey = ev.IsKeyAvailable();
        char theChar = ev.GetChar();
        logical_key lk = ev.GetLogicalKey();
        unsigned shiftFlag = ev.IsShiftDown()?DOWN:UP;
        unsigned ctrlFlag = ev.IsCtrlDown()?DOWN:UP;
        unsigned altFlag = ev.IsAltDown()?DOWN:UP;
        while(ae->msg) {
                if(ae->isKey==isKey) {
                        if((!isKey && ae->theChar==theChar) ||
                           (isKey && ae->lk==lk)) {
                                if((ae->shiftFlag==UPDOWN || ae->shiftFlag==shiftFlag) &&
                                   (ae->ctrlFlag==UPDOWN  || ae->ctrlFlag==ctrlFlag) &&
                                   (ae->altFlag==UPDOWN   || ae->altFlag==altFlag)) {
                                         outev = ev;
                                         outev.msg = ae->msg;
                                         outev.mp1 = MPFROMUINT16(ae->ID);
                                         outev.mp2 = MPFROM2UINT16(CMDSRC_ACCELERATOR,False);
                                         return True;
                                 }
                        }
                }
                ae++;
        }
        return False;
}

