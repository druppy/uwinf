/***
Filename: csi.cpp
Description:
  Implementation of FCSIxxxxx
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
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
        if(GetParent(GetWnd()->GetHwnd()) == GetParent(pWnd->GetHwnd()) )
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

        GetWindowRect(hwnd,pr);
        pr->right -= pr->left;
        pr->bottom -= pr->top;
        pr->left=pr->top=0;

        HWND hwndParent=GetParent(hwnd);
        MapWindowPoints(hwnd,hwndParent,(POINT*)pr,2);
}

//movement relative to parent
void FCSIMoveHandler::AlignTop(int newtop) {
        //move the window, so that the top becomes <newtop>
        FRect wr;
        gwripc(GetWnd(),&wr);
        SetWindowPos(GetWnd()->GetHwnd(),
                     NULL,
                     wr.left,newtop,
                     0,0,
                     SWP_NOZORDER|SWP_NOSIZE
                    );
}

void FCSIMoveHandler::AlignBottom(int newbottom) {
        //move the window, so that the bottom becomes <newbottom>
        FRect wr;
        gwripc(GetWnd(),&wr);
        SetWindowPos(GetWnd()->GetHwnd(),
                     NULL,
                     wr.left,newbottom-wr.GetHeight(),
                     0,0,
                     SWP_NOZORDER|SWP_NOSIZE
                    );
}

void FCSIMoveHandler::AlignLeft(int newleft) {
        //move the window, so that the left edge becomes <newLeft>
        FRect wr;
        gwripc(GetWnd(),&wr);
        SetWindowPos(GetWnd()->GetHwnd(),
                     NULL,
                     newleft,wr.top,
                     0,0,
                     SWP_NOZORDER|SWP_NOSIZE
                    );
}


void FCSIMoveHandler::AlignRight(int newright) {
        //move the window, so that the right edge becomes <newRight>
        FRect wr;
        gwripc(GetWnd(),&wr);
        SetWindowPos(GetWnd()->GetHwnd(),
                     NULL,
                     newright,wr.top,
                     0,0,
                     SWP_NOZORDER|SWP_NOSIZE
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
        GetClientRect(GetParent(GetWnd()->GetHwnd()),&pr);
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
        GetClientRect(GetParent(GetWnd()->GetHwnd()),&pr);
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
        SetWindowPos(GetWnd()->GetHwnd(),
                     NULL,
                     wr.left,newtop,
                     wr.GetWidth(),wr.bottom-newtop,
                     SWP_NOZORDER
                    );
}

void FCSISizeHandler::SizeBottom(int newbottom) {
        FRect wr; gwripc(GetWnd(),&wr);
        SetWindowPos(GetWnd()->GetHwnd(),
                     NULL,
                     0,0,
                     wr.GetWidth(),newbottom-wr.top,
                     SWP_NOZORDER|SWP_NOMOVE
                    );
}

void FCSISizeHandler::SizeLeft(int newleft) {
        FRect wr; gwripc(GetWnd(),&wr);
        SetWindowPos(GetWnd()->GetHwnd(),
                     NULL,
                     newleft,wr.top,
                     wr.right-newleft,wr.GetHeight(),
                     SWP_NOZORDER
                    );
}

void FCSISizeHandler::SizeRight(int newright) {
        FRect wr; gwripc(GetWnd(),&wr);
        SetWindowPos(GetWnd()->GetHwnd(),
                     NULL,
                     0,0,
                     newright-wr.left,wr.GetHeight(),
                     SWP_NOZORDER|SWP_NOMOVE
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
        GetClientRect(GetParent(GetWnd()->GetHwnd()),&pr);
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
        GetClientRect(GetParent(GetWnd()->GetHwnd()),&pr);

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

