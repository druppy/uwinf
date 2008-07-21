/***
Filename: csi.cpp
Description:
  Implementation of FCSIxxxxx
Host:
  WC10-10.6
History:
  ISJ 94-11-?? Creation
  ISJ 95-10-20 Copied and modified for WMS
***/
#define BIFINCL_WINDOW
#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_CSIHANDLERS
#include <bif.h>

/***
Visibility: protected
Description:
        Test wether or not the window is a sibling to the ourselves
***/
Bool FCSIBaseHandler::IsSibling(FWnd *pWnd) {
        if(WmsQueryWindowParent(GetWnd()->GetHwnd()) == WmsQueryWindowParent(pWnd->GetHwnd()) )
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
        WmsHWND hwnd=pWnd->GetHwnd();

        WmsQueryWindowRect(hwnd,pr);
        WmsMapWindowRects(hwnd, WmsQueryWindowParent(hwnd), pr, 1);
}

//movement relative to parent
void FCSIMoveHandler::AlignTop(int newtop) {
        //move the window, so that the top becomes <newtop>
        FRect wr;
        gwripc(GetWnd(),&wr);
        WmsSetWindowPos(GetWnd()->GetHwnd(),
                     0,
                     wr.xLeft,newtop,
                     0,0,
                     SWP_MOVE
                    );
}

void FCSIMoveHandler::AlignBottom(int newbottom) {
        //move the window, so that the bottom becomes <newbottom>
        FRect wr;
        gwripc(GetWnd(),&wr);
        WmsSetWindowPos(GetWnd()->GetHwnd(),
                     0,
                     wr.xLeft,newbottom-wr.GetHeight(),
                     0,0,
                     SWP_MOVE
                    );
}

void FCSIMoveHandler::AlignLeft(int newleft) {
        //move the window, so that the left edge becomes <newLeft>
        FRect wr;
        gwripc(GetWnd(),&wr);
        WmsSetWindowPos(GetWnd()->GetHwnd(),
                     0,
                     newleft,wr.yTop,
                     0,0,
                     SWP_MOVE
                    );
}


void FCSIMoveHandler::AlignRight(int newright) {
        //move the window, so that the right edge becomes <newRight>
        FRect wr;
        gwripc(GetWnd(),&wr);
        WmsSetWindowPos(GetWnd()->GetHwnd(),
                     0,
                     newright,wr.yTop,
                     0,0,
                     SWP_MOVE
                    );
}



Bool FCSIMoveHandler::AlignTTParent() {
        //align top of window to top of parent
        AlignTop(0);
        return True;
}

Bool FCSIMoveHandler::AlignBBParent() {
        //align bottom of window to bottom of parent
        FRect pr;
        WmsQueryWindowRect(WmsQueryWindowParent(GetWnd()->GetHwnd()),&pr);
        AlignBottom(pr.GetHeight());
        return True;
}

Bool FCSIMoveHandler::AlignLLParent() {
        //align left of window to left of parent
        AlignLeft(0);
        return True;
}

Bool FCSIMoveHandler::AlignRRParent() {
        //align rightof window to right of parent
        FRect pr;
        WmsQueryWindowRect(WmsQueryWindowParent(GetWnd()->GetHwnd()),&pr);
        AlignRight(pr.GetWidth());
        return True;
}


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
        WmsSetWindowPos(GetWnd()->GetHwnd(),
                     0,
                     wr.xLeft,newtop,
                     wr.GetWidth(),wr.yBottom-newtop,
                     SWP_SIZE|SWP_MOVE
                    );
}

void FCSISizeHandler::SizeBottom(int newbottom) {
        FRect wr; gwripc(GetWnd(),&wr);
        WmsSetWindowPos(GetWnd()->GetHwnd(),
                     0,
                     0,0,
                     wr.GetWidth(),newbottom-wr.yTop,
                     SWP_SIZE
                    );
}

void FCSISizeHandler::SizeLeft(int newleft) {
        FRect wr; gwripc(GetWnd(),&wr);
        WmsSetWindowPos(GetWnd()->GetHwnd(),
                     0,
                     newleft,wr.yTop,
                     wr.xRight-newleft,wr.GetHeight(),
                     SWP_SIZE|SWP_MOVE
                    );
}

void FCSISizeHandler::SizeRight(int newright) {
        FRect wr; gwripc(GetWnd(),&wr);
        WmsSetWindowPos(GetWnd()->GetHwnd(),
                     0,
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
        WmsQueryWindowRect(WmsQueryWindowParent(GetWnd()->GetHwnd()),&pr);
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
        WmsQueryWindowRect(WmsQueryWindowParent(GetWnd()->GetHwnd()),&pr);

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