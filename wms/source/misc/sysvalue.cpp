#define WMSINCL_SYSVALUE
#include <wms.hpp>

static uint32 sv_dblclktime;
static uint32 sv_cxdblclk;
static uint32 sv_cydblclk;
static uint32 sv_cxscreen;
static uint32 sv_cyscreen;

uint32 WmsQuerySysValue(WmsHWND, int iSysValue) {
        switch(iSysValue) {
                case SV_DBLCLKTIME:
                        return sv_dblclktime;
                case SV_CXDBLCLK:
                        return sv_cxdblclk;
                case SV_CYDBLCLK:
                        return sv_cydblclk;
                case SV_CXSCREEN:
                        return sv_cxscreen;
                case SV_CYSCREEN:
                        return sv_cyscreen;
                default:
                        return 0;
        }
}

Bool WmsSetSysValue(WmsHWND, int iSysValue, uint32 iValue) {
        switch(iSysValue) {
                case SV_DBLCLKTIME:
                        sv_dblclktime = iValue;
                        return True;
                case SV_CXDBLCLK:
                        sv_cxdblclk = iValue;
                        return True;
                case SV_CYDBLCLK:
                        sv_cydblclk = iValue;
                        return True;
                case SV_CXSCREEN:
                        sv_cxscreen=iValue;
                        return True;
                case SV_CYSCREEN:
                        sv_cyscreen=iValue;
                        return True;
                default:
                        return False;
        }
}

/*
#define SV_SWAPBUTTON              0
#define SV_CXSIZEBORDER            4
#define SV_CYSIZEBORDER            5
#define SV_ALARM                   6
#define SV_FIRSTSCROLLRATE         10
#define SV_SCROLLRATE              11
#define SV_WARNINGFREQ             13
#define SV_NOTEFREQ                14
#define SV_ERRORFREQ               15
#define SV_WARNINGDURATION         16
#define SV_NOTEDURATION            17
#define SV_ERRORDURATION           18
#define SV_CXVSCROLL               22
#define SV_CYHSCROLL               23
#define SV_CYVSCROLLARROW          24
#define SV_CXHSCROLLARROW          25
#define SV_CXBORDER                26
#define SV_CYBORDER                27
#define SV_CXDLGFRAME              28
#define SV_CYDLGFRAME              29
#define SV_CYTITLEBAR              30
#define SV_CYVSLIDER               31
#define SV_CXHSLIDER               32
#define SV_CXMINMAXBUTTON          33
#define SV_CYMINMAXBUTTON          34
#define SV_CYMENU                  35
#define SV_CXFULLSCREEN            36
#define SV_CYFULLSCREEN            37
#define SV_CXICON                  38
#define SV_CYICON                  39
#define SV_CXPOINTER               40
#define SV_CYPOINTER               41

#define SV_DEBUG                   42
#define SV_CMOUSEBUTTONS           43
#define SV_POINTERLEVEL            44
#define SV_CURSORLEVEL             45
#define SV_TRACKRECTLEVEL          46
#define SV_CTIMERS                 47
#define SV_MOUSEPRESENT            48

#define SV_CXBYTEALIGN             49
#define SV_CXALIGN                 49
#define SV_CYBYTEALIGN             50
#define SV_CYALIGN                 50
#define SV_MENUROLLDOWNDELAY       64
#define SV_MENUROLLUPDELAY         65
#define SV_CHORDTIME               70
#define SV_CXCHORD                 71
#define SV_CYCHORD                 72
#define SV_CXMOTIONSTART           73
#define SV_CYMOTIONSTART           74

#define SV_BEGINDRAG               75
#define SV_ENDDRAG                 76
#define SV_SINGLESELECT            77
#define SV_OPEN                    78
#define SV_CONTEXTMENU             79
#define SV_CONTEXTHELP             80
#define SV_TEXTEDIT                81
#define SV_BEGINSELECT             82
#define SV_ENDSELECT               83

#define SV_BEGINDRAGKB             84
#define SV_ENDDRAGKB               85
#define SV_SELECTKB                86
#define SV_OPENKB                  87
#define SV_CONTEXTMENUKB           88
#define SV_CONTEXTHELPKB           89
#define SV_TEXTEDITKB              90
#define SV_BEGINSELECTKB           91
#define SV_ENDSELECTKB             92

#define SV_ANIMATION               93
#define SV_ANIMATIONSPEED          94
*/
