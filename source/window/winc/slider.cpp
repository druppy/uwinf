/***
Filename: slider.cpp
Description:
  Implementation of the slider control
Host:
  BC40, Watcom 10, SC61
History:
  ISJ 95-01-?? Creation
  ISJ 95-06-11 Changed calculation to logical values instead of pixels
  ISJ 96-11-16 Added support for BC4.52
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_SLIDER
#define BIFINCL_APPLICATION
#include <bif.h>
#include <stdlib.h>

#ifdef __BORLANDC__
//BC simply cannot understand that slider31.h should be found in the directory
//containing this file. The full path must be hardcoded
//Sorry for the inconvinience
#include "..\source\window\winc\slider31.h"
#else
#include "slider31.h"
#endif

/***
Visibility: public
Description:
        This constructor ensures that the native slider control class is registered
***/
FSlider::FSlider()
  : FControl()
{
        RegisterSlider(GetCurrentApp()->GetHINSTANCE());
}


/***
Visibility: public
Parameters:
        pParent       parent window
        ID            control ID
        rangeMin      lower end of range (inclusive)
        rangeMax      upper end of range (inclusive)
        tickIncrement interval between ticks
Return:
        True on success
***/
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
        rangeMin      lower end of range (inclusive)
        rangeMax      upper end of range (inclusive)
        tickIncrement interval between ticks
Return:
        True on success
***/
Bool FSlider::Create(FWnd *pParent, int ID, 
                     FRect *rect, LONG style,
                     int rangeMin, int rangeMax, int tickIncrement)
{
        //It just "happens" that the style enum bits are the same as the native bits :-)
        LONG flStyle=style;
        if(!FControl::Create(pParent,ID,
                             SLIDER_CLASS, "2",
                             flStyle, rect
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
        return (DetentID)SendMessage(GetHwnd(), SLM_ADDDETENT, LV2pel(pos),0);
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
        return (Bool)SendMessage(GetHwnd(), SLM_REMOVEDETENT, did, 0);
}

/***
Visibility: public
Parameters:
        did     Detent ID
Return:
        non-negative: number of pixels the detent is positioned from home
        negative:     an error ocurred
***/
int FSlider::GetDetentPos(DetentID did) {
        return pel2LV((int)SendMessage(GetHwnd(), SLM_QUERYDETENTPOS, (WPARAM)did, 0));
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
        return (Bool)SendMessage(GetHwnd(), SLM_SETSCALETEXT, tickNum, (LPARAM)(LPCSTR)pszText);
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
Bool FSlider::GetTickText(int pos, char *buf, int ) {
        int tickNum=pos2ticknum(pos);
        if(tickNum==-1) return False;
        LRESULT lr=SendMessage(GetHwnd(), SLM_QUERYSCALETEXT, tickNum, (LPARAM)(LPSTR)buf);
        if((int)lr>=0)
                return True;
        else
                return False;
}


/***
Visibility: public
Parameters:
        pos     tick number
Return:
        size of the tick in pels. If negative an error occured
***/
int FSlider::GetTickSize(int pos) {
        int tickNum=pos2ticknum(pos);
        if(tickNum==-1) return -1;
        return (int)SendMessage(GetHwnd(), SLM_QUERYTICKSIZE, tickNum, 0);
}

/***
Visibility: public
Parameters:
        pos       tick number (if -1 the size of all ticks are set
        pels      size of the tick. If zero the tick will be invisible
Return:
        True on success
***/
Bool FSlider::SetTickSize(int pos, int pels) {
        if(pos==-1)
                return (Bool)SendMessage(GetHwnd(), SLM_SETTICKSIZE, SMA_SETALLTICKS, pels);
        else {
                int tickNum=pos2ticknum(pos);
                if(tickNum==-1) return False;
                return (Bool)SendMessage(GetHwnd(), SLM_SETTICKSIZE, tickNum, pels);
        }
}

/***
Visibility: public
Return:
        the slider arm position
***/
int FSlider::GetArmPosition() {
        return pel2LV((int)SendMessage(GetHwnd(), SLM_QUERYSLIDERINFO, SMA_SLIDERARMPOSITION, 0));
}

/***
Visibility: public
Parameters:
Return:
Description:
***/
Bool FSlider::SetArmPosition(int pos) {
        return (Bool)SendMessage(GetHwnd(), SLM_SETSLIDERINFO, SMA_SLIDERARMPOSITION, LV2pel(pos));
}       


Bool FSlider::ResetTranslation(int rangeMin, int rangeMax, int tickIncrement) {
        if(rangeMin==rangeMax) return False;
        if(tickIncrement==0) return False;
        if(rangeMin<rangeMax && tickIncrement<0) return False;
        if(rangeMin>rangeMax && tickIncrement>0) return False;
        if((rangeMax-rangeMin)%tickIncrement!=0) return False;
        int ticks=(rangeMax-rangeMin)/tickIncrement+1;
        if(!SendMessage(GetHwnd(), SLM_SETSLIDERINFO, SMA_TICKCOUNT,ticks))
                return False;
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
        int shaftLength = (int)SendMessage(GetHwnd(), SLM_QUERYSLIDERINFO, SMA_SHAFTRANGE,0);
        if(shaftLength==0) return 0; //should not happen, but could
        return rangeMin + int(long(range)*pel/shaftLength);
}

int FSlider::LV2pel(int lv) {
        int shaftLength = (int)SendMessage(GetHwnd(), SLM_QUERYSLIDERINFO, SMA_SHAFTRANGE,0);
        int lvoffset=lv-rangeMin;
        return int(long(lvoffset)*shaftLength/(range));
}

int FSlider::pos2ticknum(int pos) {
        if(rangeMin<rangeMax && (pos<rangeMin || pos>rangeMax)) return -1;
        if(rangeMin>rangeMax && (pos<rangeMax || pos>rangeMin)) return -1;
        int offset=pos-rangeMin;
        if(offset%tickIncrement!=0) return -1;
        return offset/tickIncrement;
}

int FSlider::ticknum2pos(int ticknum) {
        return rangeMin+tickIncrement*ticknum;
}

