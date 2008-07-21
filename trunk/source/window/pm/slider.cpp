/***
Filename: slider.cpp
Description:
  Implementation of FSlider
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_SLIDER
#include <bif.h>

Bool FSlider::Create(FWnd *pParent, int ID, int rangeMin, int rangeMax, int tickIncrement) {
        if(!FControl::Create(pParent,ID))
                return False;
        if(!ResetTranslation(rangeMin,rangeMax,tickIncrement)) {
                Destroy();
                return False;
        }
        return True;
}

/***
Visibility: public
Parameters:
        pParent       parent window
        ID            control ID
        rect          control rectangle
        style         combination of sls_xxxx  styles
        scale1inc     number of ticks on scale 1
        scale1space   pels between ticks on scale 1
        scale1inc     number of ticks on scale 2
        scale1space   pels between ticks on scale 2
Return:
        True on success
***/
Bool FSlider::Create(FWnd *pParent, int ID, 
                     FRect *rect, ULONG style,
                     int rangeMin, int rangeMax, int tickIncrement)
{
        //It just "happens" that the style enum bits are the same as the native bits :-)
        ULONG flStyle=(ULONG)style;
        SLDCDATA data;
        data.cbSize=sizeof(data);
        data.usScale1Increments=2;
        data.usScale1Spacing=0;
        data.usScale2Increments=0;
        data.usScale2Spacing=0;
        if(!FControl::Create(pParent,ID,
                             WC_SLIDER, "",
                             flStyle, rect,
                             &data
                            ))
                return False;
        if(!ResetTranslation(rangeMin,rangeMax,tickIncrement)) {
                Destroy();
                return False;
        }
        return True;
}

/***
Visibility: public
Parameters:
        pelPos     position of detent
Return:
        Detent id, 0 if an error ocurred
Description:
        Add a detent on the slider
***/
FSlider::DetentID FSlider::AddDetent(int pos) {
        return (DetentID)WinSendMsg(GetHwnd(), SLM_ADDDETENT, MPFROMSHORT(LV2pel(pos)), 0);
}

/***
Visibility: public
Parameters:
        did    ID of detent previously added with AddDetent()
Return:
        True on success
Description:
        Removes a detent previously added with AddDetent()
***/
Bool FSlider::RemoveDetent(DetentID did) {
        return (Bool)WinSendMsg(GetHwnd(), SLM_REMOVEDETENT, MPFROMLONG(did), 0);
}

/***
Visibility: public
Parameters:
        did     Detent ID
        scale   (output)  scale on which the detent is (can be NULL)
Return:
        non-negative: number of pixels the detent is positioned from home
        negative:     an error ocurred
***/
int FSlider::GetDetentPos(DetentID did, int *scale) {
        MRESULT mr=WinSendMsg(GetHwnd(), SLM_QUERYDETENTPOS, MPFROMLONG(did), 0);
        if(scale) *scale=SHORT2FROMMR(mr);
        return pel2LV((SHORT)SHORT1FROMMR(mr));
}

/***
Visibility: public
Parameters:
        tickNum    tick number
        pszText    text that is to be drawn at the tick
Return:
        True on success
Description:
        Each tick can be assigned a text string that will be draw at the tick.
***/
Bool FSlider::SetTickText(int pos, const char *pszText) {
        int tickNum=pos2ticknum(pos);
        if(tickNum==-1) return False;
        return (Bool)WinSendMsg(GetHwnd(), SLM_SETSCALETEXT, MPFROMSHORT(tickNum), MPFROMP(pszText));
}

/***
Visibility: public
Parameters:
        tickNum   tick number
        buf       (output) tick text
        buflen    size of <buf>
Return:
        True on success
***/
Bool FSlider::GetTickText(int pos, char *buf, int buflen) {
        int tickNum=pos2ticknum(pos);
        if(tickNum==-1) return False;
        MRESULT mr=WinSendMsg(GetHwnd(), SLM_QUERYSCALETEXT, MPFROM2SHORT(tickNum,buflen), MPFROMP(buf));
        if((SHORT)SHORT1FROMMR(mr)>=0)
                return True;
        else
                return False;
}

/***
Visibility: public
Parameters:
        tickNum  tick number
Return:
        size of the tick in pels. If negative an error occured
***/
int FSlider::GetTickSize(int pos) {
        int tickNum=pos2ticknum(pos);
        if(tickNum==-1) return False;
        return (SHORT)SHORT1FROMMR(WinSendMsg(GetHwnd(), SLM_QUERYTICKSIZE, MPFROMSHORT(tickNum), 0));
}

/***
Visibility: public
Parameters:
        tickNum   tick number (if -1 the size of all ticks are set
        pels      size of the tick. If zero the tick will be invisible
Return:
        True on success
***/
Bool FSlider::SetTickSize(int pos, int pels) {
        if(pos==-1)
                return (Bool)WinSendMsg(GetHwnd(), SLM_SETTICKSIZE, MPFROM2SHORT(SMA_SETALLTICKS,pels), 0);
        else {
                int tickNum=pos2ticknum(pos);
                if(tickNum==-1) return False;
                return (Bool)WinSendMsg(GetHwnd(), SLM_SETTICKSIZE, MPFROM2SHORT(tickNum,pels), 0);
        }
}

/***
Visibility: public
Return:
        the slider arm position
***/
int FSlider::GetArmPosition() {
        return pel2LV(SHORT1FROMMR(WinSendMsg(GetHwnd(), SLM_QUERYSLIDERINFO, MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_RANGEVALUE), 0)));
}

/***
Visibility: public
Parameters:
Return:
Description:
***/
Bool FSlider::SetArmPosition(int pos) {
        return (Bool)WinSendMsg(GetHwnd(), SLM_SETSLIDERINFO, MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_RANGEVALUE), MPFROMSHORT(LV2pel(pos)));
}       


Bool FSlider::ResetTranslation(int rangeMin, int rangeMax, int tickIncrement) {
        if(rangeMin==rangeMax) return False;
        if(tickIncrement==0) return False;
        if(rangeMin<rangeMax && tickIncrement<0) return False;
        if(rangeMin>rangeMax && tickIncrement>0) return False;

        int shaftlength = SHORT1FROMMR(WinSendMsg(GetHwnd(), SLM_QUERYSLIDERINFO, MPFROM2SHORT(SMA_SHAFTDIMENSIONS,0),0));

        SLDCDATA sliderdata;
        sliderdata.cbSize=sizeof(sliderdata);
        sliderdata.usScale1Increments = (USHORT) ((rangeMax-rangeMin)/tickIncrement+1);
        sliderdata.usScale1Spacing    = 0;
        sliderdata.usScale2Increments = 0;
        sliderdata.usScale2Spacing    = 0;

        WNDPARAMS wndparams;
        wndparams.fsStatus = WPM_CTLDATA;
        wndparams.cchText=0;
        wndparams.pszText=0;
        wndparams.cbPresParams=0;
        wndparams.pPresParams=0;
        wndparams.cbCtlData=0;
        wndparams.pCtlData = &sliderdata;
        if(SHORT1FROMMR(WinSendMsg(GetHwnd(), WM_SETWINDOWPARAMS, MPFROMP(&wndparams), 0)) == FALSE)
                return False; //could not set the control data

        //update our values
        FSlider::rangeMin=rangeMin;
        FSlider::rangeMax=rangeMax;
        FSlider::tickIncrement=tickIncrement;
        if(rangeMin<=rangeMax)
                FSlider::range = rangeMax-rangeMin+1;
        else
                FSlider::range = rangeMax-rangeMin-1;
        return True;
}


int FSlider::pel2LV(int pel) {
        int shaftlength = SHORT1FROMMR(WinSendMsg(GetHwnd(), SLM_QUERYSLIDERINFO, MPFROM2SHORT(SMA_SHAFTDIMENSIONS,0),0));
        if(shaftlength==0) return 0; //should not happen, but could happen
        return rangeMin +  (range*pel)/shaftlength;
}

int FSlider::LV2pel(int lv) {
        int shaftlength = SHORT1FROMMR(WinSendMsg(GetHwnd(), SLM_QUERYSLIDERINFO, MPFROM2SHORT(SMA_SHAFTDIMENSIONS,0),0));
        return (shaftlength*(lv-rangeMin))/range;
}

int FSlider::pos2ticknum(int pos) {
        if(rangeMin<rangeMax && (pos<rangeMin || pos>rangeMax)) return -1;
        if(rangeMin>rangeMax && (pos<rangeMax || pos>rangeMin)) return -1;
        int offset= pos-rangeMin;
        if(offset%tickIncrement!=0) return -1;
        return offset/tickIncrement;
}

int FSlider::ticknum2pos(int ticknum) {
        return rangeMin+tickIncrement*ticknum;
}

