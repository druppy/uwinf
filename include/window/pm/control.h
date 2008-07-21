#ifndef __CONTROL_H_INCLUDED
#define __CONTROL_H_INCLUDED

class BIFCLASS FOwnerNotifyEvent : public FEvent {
public:
        int GetItem() const { return SHORT1FROMMP(GetMP1()); }
        int GetNotifyCode() const { return SHORT2FROMMP(GetMP1()); }
        ULONG GetNotifySpec() const { return LONGFROMMP(GetMP2()); }    //os/2
};

class BIFCLASS FOwnerNotifyHandler : public FHandler {
public:
        FOwnerNotifyHandler(FWnd *pwnd);

protected:
        Bool Dispatch(FEvent&);

        virtual Bool Notify(FOwnerNotifyEvent&) { return False; }
};


//controls ===================================================================

class BIFCLASS FControl : public FWnd {
public:
        FControl()
          : FWnd()
          {}

        Bool Create(FWnd *pParent, int ID);
        Bool Create(FWnd *pParent, int ID,
                    const char *pszClassname, const char *pszControlText,
                    ULONG flStyle, FRect *rect,
                    PVOID pCtlData=0, PVOID pPresParam=0);


        int GetID();
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

//fan-out include
#if defined(BIFINCL_OWNERDRAW) && !defined(__OWNERDRW_H_INCLUDED)
#  include <window\pm\ownerdrw.h>
#endif
#if defined(BIFINCL_SLE) && !defined(__SLE_H_INCLUDED)
#  include <window\pm\sle.h>
#endif
#if defined(BIFINCL_MLE) && !defined(__MLE_H_INCLUDED)
#  include <window\pm\mle.h>
#endif
#if defined(BIFINCL_CHECKBOX) && !defined(__CHECKBOX_H_INCLUDED)
#  include <window\pm\checkbox.h>
#endif
#if defined(BIFINCL_RADIOBUTTON) && !defined(__RADIOBUT_H_INCLUDED)
#  include <window\pm\radiobut.h>
#endif
#if defined(BIFINCL_STATICTEXT) && !defined(__STEXT_H_INCLUDED)
#  include <window\pm\stext.h>
#endif
#if defined(BIFINCL_GROUPBOX) && !defined(__GROUPBOX_H_INCLUDED)
#  include <window\pm\groupbox.h>
#endif
#if defined(BIFINCL_PUSHBUTTON) && !defined(PUSHBUT_H_INCLUDED)
#  include <window\pm\pushbut.h>
#endif
#if defined(BIFINCL_LISTBOX) && !defined(__LISTBOX_H_INCLUDED)
#  include <window\pm\listbox.h>
#endif
#if defined(BIFINCL_COMBOBOX) && !defined(__COMBOBOX_H_INCLUDED)
#  include <window\pm\combobox.h>
#endif
#if defined(BIFINCL_SLIDER) && !defined(__BW_SLIDER_H_INCLUDED)
#  include <window\pm\slider.h>
#endif
#if defined(BIFINCL_SCROLLBAR) && !defined(__SCROLLBA_H_INCLUDED)
#  include <window\pm\scrollba.h>
#endif
