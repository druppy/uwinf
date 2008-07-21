/***
Filename: preparm.cpp
Description:
        Implementation of presentation parameter API
History:
        ???
        ISJ 1996-10-06 Changed inheritance from parent-child to owner-ownee, so
                       the listbox of comboboxes inherits from the combox and
                       menus inherits from their frame window
***/
#define BIFINCL_WINDOW
#define WMSINCL_MSGQUEUE
#define WMSINCL_MSGS
#include <bif.h>
#include "presparm.h"

Bool WmsSetPresParam(WmsHWND hwnd, int id, const void *buf, unsigned buflen) {
        if(!buf) return False;
        return (Bool)WmsSendMsg(hwnd, WM_SETPRESPARAM, MPFROMP(buf), MPFROM2UINT16(buflen,id));
}

Bool WmsRemovePresParam(WmsHWND hwnd, int id) {
        return (Bool)WmsSendMsg(hwnd, WM_SETPRESPARAM, MPFROMP(0), MPFROM2UINT16(0,id));
}

Bool WmsQueryPresParam(WmsHWND hwnd, int id1, int id2, int *idFound, void *buf, int buflen, Bool inherit) {
        if(!buf) return False;
        do {
                Bool b=(Bool)WmsSendMsg(hwnd, WM_QUERYPRESPARAM, MPFROMP(buf), MPFROM2UINT16(buflen,id1));
                if(b) {
                        if(idFound) *idFound=id1;
                        return True;
                }
                b=(Bool)WmsSendMsg(hwnd, WM_QUERYPRESPARAM, MPFROMP(buf), MPFROM2UINT16(buflen,id2));
                if(b) {
                        if(idFound) *idFound=id2;
                        return True;
                }
                hwnd = WmsQueryWindowOwner(hwnd);
                if(!hwnd) return False;
        } while(inherit);
        return False;
}

