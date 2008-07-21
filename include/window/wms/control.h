#ifndef __CONTROL_H_INCLUDED
#define __CONTROL_H_INCLUDED

//control styles:
#define WS_GROUP   0x10000000L
#define WS_TABSTOP 0x20000000L

//response flags to WM_QUERYDLGCODE
#define DLGC_STATIC             0x0001
#define DLGC_PUSHBUTTON         0x0002 /*understands BM_QUERYDEFAULT/BM_SETDEFAULT*/
#define DLGC_RADIOBUTTON        0x0003
#define DLGC_CHECKBOX           0x0004
#define DLGC_ENTRYFIELD         0x0005
#define DLGC_SLE                0x0006
#define DLGC_MLE                0x0007
#define DLGC_SCROLLBAR          0x0008
#define DLGC_LISTBOX            0x0009
#define DLGC_SLIDER             0x000a
#define DLGC_MENU               0x000b
#define DLGC_DEFAULT            0x4000
#define DLGC_CONTROL            0x8000


class BIFCLASS FOwnerNotifyEvent : public FEvent {
public:
        int GetItem() const { return (int)UINT161FROMMP(GetMP1()); }
        int GetNotifyCode() const { return (int)UINT162FROMMP(GetMP1()); }
        uint32 GetNotifySpec() const { return UINT32FROMMP(GetMP2()); }
};

class BIFCLASS FOwnerNotifyHandler : public FHandler {
public:
        FOwnerNotifyHandler(FWnd *pwnd);

protected:
        Bool Dispatch(FEvent&);

        virtual Bool Notify(FOwnerNotifyEvent&) { return False; }
};


//controls ===================================================================
class WMSControlHandler : public FHandler {
        //Special handler for controls since under WMS we have to do
        //everything ourselves
protected:
        Bool invalidateWhenTextChanges;
public:
        WMSControlHandler(FWnd *pwnd)
          : FHandler(pwnd), invalidateWhenTextChanges(False)
        { SetDispatch(dispatchFunc_t(WMSControlHandler::Dispatch)); }
        Bool Dispatch(FEvent &ev);
        virtual Bool Paint() =0;
        virtual Bool MatchMnemonic(char /*c*/) { return False; }
        virtual unsigned QueryDlgCode() =0;
};

class BIFCLASS FControl : public FWnd, public WMSControlHandler {
public:
        FControl()
          : FWnd(), WMSControlHandler(this)
          {}

        Bool Create(FWnd *pParent, int resID);
        Bool Create(FWnd *pParent, int ID,
                    const char *pszControlText,
                    uint32 flStyle, FRect *rect);


        int GetID();
protected:
        virtual Bool ProcessResourceSetup(int props, char *name[], char *value[]);
};

//mix-in handler for controls that supports text query/set
class BIFCLASS FControlTextHandler : public FHandler {
public:
        FControlTextHandler(FWnd *pwnd) : FHandler(pwnd) {}

        int GetTextLen();
        Bool GetText(char *buffer, int maxbuf);
        Bool SetText(const char *pszText);
};

#endif  //__CONTROL_H_INCLUDED

#ifdef BIFINCL_ALLCONTROLS
#  define BIFINCL_SLE
#  define BIFINCL_MLE
#  define BIFINCL_CHECKBOX
#  define BIFINCL_RADIOBUTTON
#  define BIFINCL_STATICTEXT
#  define BIFINCL_GROUPBOX
#  define BIFINCL_PUSHBUTTON
#  define BIFINCL_LISTBOX
#  define BIFINCL_COMBOBOX
#  define BIFINCL_OWNERDRAW
#  define BIFINCL_SLIDER
#  define BIFINCL_SCROLLBAR
#endif

//dependencies
#if defined(BIFINCL_LISTBOX) || defined(BIFINCL_COMBOBOX)
#  define BIFINCL_OWNERDRAW
#endif
#if defined(BIFINCL_CHECKBOX) || defined(BIFINCL_RADIOBUTTON) || defined(BIFINCL_PUSHBUTTON)
#  define BIFINCL_LOWLEVELBUTTON
#endif

//fan-out include
#if defined(BIFINCL_LOWLEVELBUTTON) && !defined(__LOWLBUT_H_INCLUDED)
#  include <window\wms\lowlbut.h>
#endif
#if defined(BIFINCL_OWNERDRAW) && !defined(__OWNERDRW_H_INCLUDED)
#  include <window\wms\ownerdrw.h>
#endif
#if defined(BIFINCL_SLE) && !defined(__SLE_H_INCLUDED)
#  include <window\wms\sle.h>
#endif
#if defined(BIFINCL_MLE) && !defined(__MLE_H_INCLUDED)
#  include <window\wms\mle.h>
#endif
#if defined(BIFINCL_CHECKBOX) && !defined(__CHECKBOX_H_INCLUDED)
#  include <window\wms\checkbox.h>
#endif
#if defined(BIFINCL_RADIOBUTTON) && !defined(__RADIOBUT_H_INCLUDED)
#  include <window\wms\radiobut.h>
#endif
#if defined(BIFINCL_STATICTEXT) && !defined(__STEXT_H_INCLUDED)
#  include <window\wms\stext.h>
#endif
#if defined(BIFINCL_GROUPBOX) && !defined(__GROUPBOX_H_INCLUDED)
#  include <window\wms\groupbox.h>
#endif
#if defined(BIFINCL_PUSHBUTTON) && !defined(PUSHBUT_H_INCLUDED)
#  include <window\wms\pushbut.h>
#endif
#if defined(BIFINCL_LISTBOX) && !defined(__LISTBOX_H_INCLUDED)
#  include <window\wms\listbox.h>
#endif
#if defined(BIFINCL_COMBOBOX) && !defined(__COMBOBOX_H_INCLUDED)
#  include <window\wms\combobox.h>
#endif
#if defined(BIFINCL_SLIDER) && !defined(__BW_SLIDER_H_INCLUDED)
#  include <window\wms\slider.h>
#endif
#if defined(BIFINCL_SCROLLBAR) && !defined(__SCROLLBA_H_INCLUDED)
#  include <window\wms\scrollba.h>
#endif
