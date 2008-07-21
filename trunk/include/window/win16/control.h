#ifndef __CONTROL_H_INCLUDED
#define __CONTROL_H_INCLUDED

class BIFCLASS FOwnerNotifyEvent : public FEvent
{
public:
        int GetItem() const { return wParam; }
        HWND GetItemHwnd() const { return (HWND)LOWORD(lParam); } //win16
        int GetNotifyCode() const { return HIWORD(lParam); }
};

class BIFCLASS FOwnerNotifyHandler : public FHandler
{
public:
        FOwnerNotifyHandler(FWnd *pwnd);

protected:
        Bool Dispatch(FEvent&);

        virtual Bool Notify(FOwnerNotifyEvent&) { return False; }
};


///////////////////////
// controls 

class BIFCLASS FControl : public FWnd
{
public:
        FControl()
          : FWnd()
          {}

        Bool Create(FWnd *pParent, int ID);
        Bool Create(FWnd *pParent, int ID,
                    const char *pszClassname, const char *pszControlText,
                    DWORD dwStyle, FRect *rect);

        int GetID();
};

//mix-in handler for controls that supports text query/set
class BIFCLASS FControlTextHandler : public FHandler
{
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
#endif

//dependencies
#if defined(BIFINCL_LISTBOX) || defined(BIFINCL_COMBOBOX)
#  define BIFINCL_OWNERDRAW
#endif

//fan-out include
#if defined(BIFINCL_OWNERDRAW) && !defined(__OWNERDRW_H_INCLUDED)
#  include <window\win16\ownerdrw.h>
#endif
#if defined(BIFINCL_SLE) && !defined(__SLE_H_INCLUDED)
#  include <window\win16\sle.h>
#endif
#if defined(BIFINCL_MLE) && !defined(__MLE_H_INCLUDED)
#  include <window\win16\mle.h>
#endif
#if defined(BIFINCL_CHECKBOX) && !defined(__CHECKBOX_H_INCLUDED)
#  include <window\win16\checkbox.h>
#endif
#if defined(BIFINCL_RADIOBUTTON) && !defined(__RADIOBUT_H_INCLUDED)
#  include <window\win16\radiobut.h>
#endif
#if defined(BIFINCL_STATICTEXT) && !defined(__STEXT_H_INCLUDED)
#  include <window\win16\stext.h>
#endif
#if defined(BIFINCL_GROUPBOX) && !defined(__GROUPBOX_H_INCLUDED)
#  include <window\win16\groupbox.h>
#endif
#if defined(BIFINCL_PUSHBUTTON) && !defined(PUSHBUT_H_INCLUDED)
#  include <window\win16\pushbut.h>
#endif
#if defined(BIFINCL_LISTBOX) && !defined(__LISTBOX_H_INCLUDED)
#  include <window\win16\listbox.h>
#endif
#if defined(BIFINCL_COMBOBOX) && !defined(__COMBOBOX_H_INCLUDED)
#  include <window\win16\combobox.h>
#endif
#if defined(BIFINCL_SLIDER) && !defined(__BW_SLIDER_H_INCLUDED)
#  include <window\win16\slider.h>
#endif

