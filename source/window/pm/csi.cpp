/***
Filename: csi.cpp
Description:
  Implementation of FCSIxxxxx
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_CSIHANDLERS
#include <bif.h>

/***
Visibility: protected
Description:
        Test wether or not the window is a sibling to the ourselves
Return:
        True if the window is a sibling, False otherwise
***/
Bool FCSIBaseHandler::IsSibling(FWnd *pWnd) {
        if(WinQueryWindow(GetWnd()->GetHwnd(),QW_PARENT) == WinQueryWindow(pWnd->GetHwnd(),QW_PARENT) )
                return True;
        else
                return False;
}

/***
Visibility: protected
Description:
        Get the window rect of the window in parent coordinates
***/
void FCSIBaseHandler::gwripc(FWnd *pWnd, FRect *pr) {
        HWND hwnd=pWnd->GetHwnd();

        WinQueryWindowRect(hwnd,pr);
        pr->xRight -= pr->xLeft;
        pr->yTop -= pr->yBottom;
        pr->xLeft=pr->yBottom=0;

        HWND hwndParent=WinQueryWindow(hwnd,QW_PARENT);
        WinMapWindowPoints(hwnd,hwndParent,(POINTL*)pr,2);
}

//movement relative to parent
void FCSIMoveHandler::AlignTop(int newtop) {
        //move the window, so that the top becomes <newtop>
        FRect wr;
        gwripc(GetWnd(),&wr);
        WinSetWindowPos(GetWnd()->GetHwnd(),
                        NULL,
                        wr.xLeft,newtop-wr.GetHeight(),
                        0,0,
                        SWP_MOVE
                       );
}

void FCSIMoveHandler::AlignBottom(int newbottom) {
        //move the window, so that the bottom becomes <newbottom>
        FRect wr;
        gwripc(GetWnd(),&wr);
        WinSetWindowPos(GetWnd()->GetHwnd(),
                        NULL,
                        wr.xLeft,newbottom,
                        0,0,
                        SWP_MOVE
                       );
}

void FCSIMoveHandler::AlignLeft(int newleft) {
        //move the window, so that the left edge becomes <newLeft>
        FRect wr;
        gwripc(GetWnd(),&wr);
        WinSetWindowPos(GetWnd()->GetHwnd(),
                        NULL,
                        newleft,wr.yBottom,
                        0,0,
                        SWP_MOVE
                       );
}


void FCSIMoveHandler::AlignRight(int newright) {
        //move the window, so that the right edge becomes <newRight>
        FRect wr;
        gwripc(GetWnd(),&wr);
        WinSetWindowPos(GetWnd()->GetHwnd(),
                        NULL,
                        newright-wr.GetWidth(),wr.yBottom,
                        0,0,
                        SWP_MOVE
                       );
}



/***
Visibility: public
Description:
        Move the window so that its top is aligned with the top of its parent
           +-------+-------+------+
           |       | child |      |
           |       |       |      |
           |parent +-------+      |
           |                      |
           +----------------------+
Return:
        True on success
***/
Bool FCSIMoveHandler::AlignTTParent() {
        //align top of window to top of parent
        AlignTop(0);
        return True;
}

/***
Visibility: public
Description:
        Move the window so that its bottom is aligned with the bottom of its parent
           +----------------------+
           |                      |
           |       +-------+      |
           |parent | child |      |
           |       |       |      |
           +-------+-------+------+
Return:
        True on success
***/
Bool FCSIMoveHandler::AlignBBParent() {
        //align bottom of window to bottom of parent
        FRect pr;
        WinQueryWindowRect(WinQueryWindow(GetWnd()->GetHwnd(),QW_PARENT),&pr);
        AlignBottom(pr.GetHeight());
        return True;
}

/***
Visibility: public
Description:
        Move the window so that its left edge is aligned with the left edge of its parent
           +----------------------+
           |                      |
           +--------+             |
           | child  |     parent  |
           +--------+             |
           |                      |
           +----------------------+
Return:
        True on success
***/
Bool FCSIMoveHandler::AlignLLParent() {
        //align left of window to left of parent
        AlignLeft(0);
        return True;
}

/***
Visibility: public
Description:
        Move the window so that its right edge is aligned with the right edge of its parent
           +----------------------+
           |                      |
           |             +--------+
           | parent      |child   |
           |             +--------+
           |                      |
           +----------------------+
Return:
        True on success
***/
Bool FCSIMoveHandler::AlignRRParent() {
        //align rightof window to right of parent
        FRect pr;
        WinQueryWindowRect(WinQueryWindow(GetWnd()->GetHwnd(),QW_PARENT),&pr);
        AlignRight(pr.GetWidth());
        return True;
}


/***
Visibility: public
Description:
        Move the window so that its left edge is aligned with the left edge of the sibling
           +------------+
           | sibling    |
           +------------+
                 
           +--------+             
           | window |
           +--------+
Return:
        True on success
***/
//movement relative to siblings
Bool FCSIMoveHandler::AlignLLSibling(FWnd *pSibling) {
        //align left to left of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        AlignLeft(sr.GetLeft());

        return True;
}

Bool FCSIMoveHandler::AlignRRSibling(FWnd *pSibling) {
        //align right to right of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        AlignRight(sr.GetRight());

        return True;
}

Bool FCSIMoveHandler::AlignTTSibling(FWnd *pSibling) {
        //align top to top of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        AlignTop(sr.GetTop());

        return True;
}

Bool FCSIMoveHandler::AlignBBSibling(FWnd *pSibling) {
        //align bottom to bottom of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        AlignBottom(sr.GetBottom());

        return True;
}


Bool FCSIMoveHandler::AlignLRSibling(FWnd *pSibling) {
        //align left to right of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        AlignLeft(sr.GetRight());

        return True;
}

Bool FCSIMoveHandler::AlignRLSibling(FWnd *pSibling) {
        //align right to left of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        AlignRight(sr.GetLeft());

        return True;
}

Bool FCSIMoveHandler::AlignTBSibling(FWnd *pSibling) {
        //align top to bottom of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        AlignTop(sr.GetBottom());

        return True;
}

Bool FCSIMoveHandler::AlignBTSibling(FWnd *pSibling) {
        //align bottom to top of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        AlignBottom(sr.GetTop());

        return True;
}


//size relative to parent
void FCSISizeHandler::SizeTop(int newtop) {
        FRect wr; gwripc(GetWnd(),&wr);
        WinSetWindowPos(GetWnd()->GetHwnd(),
                        NULL,
                        0,0,
                        wr.GetWidth(),newtop-wr.yBottom,
                        SWP_SIZE
                       );
}

void FCSISizeHandler::SizeBottom(int newbottom) {
        FRect wr; gwripc(GetWnd(),&wr);
        WinSetWindowPos(GetWnd()->GetHwnd(),
                        NULL,
                        wr.xLeft,newbottom,
                        wr.GetWidth(),wr.yTop-newbottom,
                        SWP_SIZE|SWP_MOVE
                       );
}

void FCSISizeHandler::SizeLeft(int newleft) {
        FRect wr; gwripc(GetWnd(),&wr);
        WinSetWindowPos(GetWnd()->GetHwnd(),
                        NULL,
                        newleft,wr.yBottom,
                        wr.xRight-newleft,wr.GetHeight(),
                        SWP_SIZE|SWP_MOVE
                       );
}

void FCSISizeHandler::SizeRight(int newright) {
        FRect wr; gwripc(GetWnd(),&wr);
        WinSetWindowPos(GetWnd()->GetHwnd(),
                        NULL,
                        0,0,
                        newright-wr.xLeft,wr.GetHeight(),
                        SWP_SIZE
                       );
}

Bool FCSISizeHandler::SizeTTParent() {
        //size top of window to top of parent
        SizeTop(0);
        return True;
}

Bool FCSISizeHandler::SizeBBParent() {
        //size bottom of window to bottom of parent
        FRect pr;
        WinQueryWindowRect(WinQueryWindow(GetWnd()->GetHwnd(),QW_PARENT),&pr);
        SizeBottom(pr.GetHeight());

        return True;
}

Bool FCSISizeHandler::SizeLLParent() {
        //size left of window to left of parent
        SizeLeft(0);
        return True;
}

Bool FCSISizeHandler::SizeRRParent() {
        //size rightof window to right of parent
        FRect pr;
        WinQueryWindowRect(WinQueryWindow(GetWnd()->GetHwnd(),QW_PARENT),&pr);

        SizeRight(pr.GetWidth());

        return True;
}


//movement relative to siblings
Bool FCSISizeHandler::SizeLLSibling(FWnd *pSibling) {
        //size left to left of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        SizeLeft(sr.GetLeft());

        return True;
}

Bool FCSISizeHandler::SizeRRSibling(FWnd *pSibling) {
        //size right to right of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        SizeRight(sr.GetRight());

        return True;
}

Bool FCSISizeHandler::SizeTTSibling(FWnd *pSibling) {
        //size top to top of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        SizeTop(sr.GetTop());

        return True;
}

Bool FCSISizeHandler::SizeBBSibling(FWnd *pSibling) {
        //size bottom to bottom of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        SizeBottom(sr.GetBottom());

        return True;
}


Bool FCSISizeHandler::SizeLRSibling(FWnd *pSibling) {
        //size left to right of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        SizeLeft(sr.GetRight());

        return True;
}

Bool FCSISizeHandler::SizeRLSibling(FWnd *pSibling) {
        //size right to left of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        SizeRight(sr.GetLeft());

        return True;
}

Bool FCSISizeHandler::SizeTBSibling(FWnd *pSibling) {
        //size top to bottom of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        SizeTop(sr.GetBottom());

        return True;
}

Bool FCSISizeHandler::SizeBTSibling(FWnd *pSibling) {
        //size bottom to top of sibling
        if(!IsSibling(pSibling)) return False;

        FRect sr; gwripc(pSibling,&sr);
        SizeBottom(sr.GetTop());

        return True;
}

