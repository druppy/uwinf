/***
Filename: msgbox.cpp
Description:
  Implementation of MessageBox()
Host:
  Watcom 10.6
History:
  ISJ 96-05-28 Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_DIALOG
#define BIFINCL_CONTROLS
#define BIFINCL_STATICTEXT
#define BIFINCL_PUSHBUTTON
#define BIFINCL_MESSAGEBOX
#define WMSINCL_MSGS
#define WMSINCL_CPI
#include <bif.h>

class F_MessageBox : public FModalDialog {
        const char *pszText;
        const char *pszTitle;
        mb_buttons buttons;
        mb_icon icon;

        FStaticText stext;
        FPushButton pb[3];
public:
        mbid DoModal(FWnd *powner, const char *pszText, const char *pszTitle, mb_buttons buttons, mb_icon icon);
        Bool CreateControls();
        Bool Command(FCommandEvent& ev);
};

Bool F_MessageBox::CreateControls() {
        SetCaption(pszTitle);

        //create static text
        if(!stext.Create(this, -1, (FRect*)0, stext.st_left, pszText))
                return False;

        //create buttons
        char *pbtext[3];
        mbid pbid[3];
        switch(buttons) {
                case mb_abortretryignore:
                        pbtext[0] = "@Abort";
                        pbid[0] = mbid_abort;
                        pbtext[1] = "@Retry";
                        pbid[1] = mbid_retry;
                        pbtext[2] = "@Ignore";
                        pbid[2] = mbid_ignore;
                        break;
                case mb_ok:
                        pbtext[0] = "@Ok";
                        pbid[0] = mbid_ok;
                        pbtext[1] = 0;
                        pbtext[2] = 0;
                        break;
                case mb_okcancel:
                        pbtext[0] = "@Ok";
                        pbid[0] = mbid_ok;
                        pbtext[1] = "@Cancel";
                        pbid[1] = mbid_cancel;
                        pbtext[2] = 0;
                        break;
                case mb_retrycancel:
                        pbtext[0] = "@Retry";
                        pbid[0] = mbid_retry;
                        pbtext[1] = "@Cancel";
                        pbid[1] = mbid_cancel;
                        pbtext[2] = 0;
                        break;
                case mb_yesno:
                        pbtext[0] = "@Yes";
                        pbid[0] = mbid_yes;
                        pbtext[1] = "@No";
                        pbid[1] = mbid_no;
                        pbtext[2] = 0;
                        break;
                case mb_yesnocancel:
                        pbtext[0] = "@Yes";
                        pbid[0] = mbid_yes;
                        pbtext[1] = "@No";
                        pbid[1] = mbid_no;
                        pbtext[2] = "@Cancel";
                        pbid[2] = mbid_cancel;
                        break;
        }
        for(int i=0; i<3 && pbtext[i]; i++) {
                if(!pb[i].Create(this, pbid[i], (FRect*)0, pbtext[i], (Bool)(i==0)))
                        return False;
        }
        WmsSetWindowStyleBits(pb[0].GetHwnd(),WS_TABSTOP,WS_TABSTOP);

        //now size&move the dialog and controls
        WmsSWP swp[5];
        //measure the text
        int maxlinelength=0;
        if(pbtext[0]!=0) maxlinelength +=  9;
        if(pbtext[1]!=0) maxlinelength +=  1+9;
        if(pbtext[2]!=0) maxlinelength +=  1+9;
        const char *p=pszText;
        while(*p) {
                const char *np=p+1;
                while(*np && *np!='\n') np++;
                if(np-p > maxlinelength) maxlinelength = np-p;
                p=np;
        }
        if(maxlinelength>78) maxlinelength=78;
        FRect r(0,0,maxlinelength,2);
        WmsHDC hdc=WmsGetWindowDC(GetHwnd(), 0);
        if(hdc) {
                CpiDrawText(hdc, pszText, -1, &r,clr_black,clr_black, DT_LEFT|DT_CALCRECT|DT_WORDBREAK);
                WmsReleaseDC(hdc);
        }
        int textwidth = r.GetWidth();
        int textheight = r.GetHeight();

        //size&move the dialog
        WmsQueryWindowRect(WmsQueryWindowParent(GetHwnd()),&r);
        swp[0].hwnd = GetHwnd();
        swp[0].cx = textwidth + 4;
        swp[0].cy = textheight + 6;
        swp[0].x = (r.GetWidth() - swp[0].cx) / 2;
        swp[0].y = (r.GetHeight() - swp[0].cy) / 2;
        swp[0].flags = SWP_MOVE|SWP_SIZE;
        //size&move the text
        swp[1].hwnd = stext.GetHwnd();
        swp[1].x = 2;
        swp[1].y = 2;
        swp[1].cx = swp[0].cx - 4;
        swp[1].cy = swp[0].cy - 6;
        swp[1].flags = SWP_SIZE|SWP_MOVE;
        //size&move the buttons
        int left=2;
        for(i=0; i<3 && pbtext[i]; i++) {
                swp[2+i].hwnd = pb[i].GetHwnd();
                swp[2+i].x = left;
                swp[2+i].y = swp[0].cy - 3;
                swp[2+i].cx = 9;
                swp[2+i].cy = 2;
                swp[2+i].flags = SWP_SIZE|SWP_MOVE;
                left += 10;
        }
        WmsSetMultWindowPos(swp, 2+i);

        return True;
}

mbid F_MessageBox::DoModal(FWnd *powner, const char *pszText, const char *pszTitle, mb_buttons buttons, mb_icon icon) {
        F_MessageBox::pszText = pszText?pszText:"";
        F_MessageBox::pszTitle = pszTitle?pszTitle:"Error";
        F_MessageBox::buttons = buttons;
        F_MessageBox::icon = icon;

        FRect r(0,0,0,0);
        int rc = FModalDialog::DoModal(powner, 
                                       999,
                                       fcf_sysmenu|
                                       fcf_titlebar|
                                       fcf_dialogborder,
                                       alf_none,
                                       &r
                                      );
        return (mbid)rc;
}

Bool F_MessageBox::Command(FCommandEvent& ev) {
        if(ev.GetItem()==DID_CANCEL) {
                if(WmsWindowFromID(GetHwnd(),mbid_cancel)==0)
                        return True; //eat
        }
        return FModalDialog::Command(ev); //let default processing occur
}

mbid BIFFUNCTION MessageBox(FWnd *pOwner, const char *pszText, const char *pszTitle, mb_buttons buttons, mb_icon icon) {
        F_MessageBox msgbox;
        return msgbox.DoModal(pOwner,pszText,pszTitle,buttons,icon);
}

