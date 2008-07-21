#ifndef __SLE_H_INCLUDED
#define __SLE_H_INCLUDED

#ifndef __CLHNDL_H_INCLUDED
#  include <window\wms\clhndl.h>
#endif
#ifndef __KEYBOARD_H_INCLUDED
#  include <window\wms\keyboard.h>
#endif
#ifndef __MOUSE_H_INCLUDED
#  include <window\wms\mouse.h>
#endif
#ifndef __COMHNDL_H_INCLUDED
#  include <window\wms\comhndl.h>
#endif

//WM_CONTROL notifications:
#define EN_CHANGE       0x0001
#define EN_GOTFOCUS     0x0002
#define EN_LOSTFOCUS    0x0003

class FSLEHandler : public FHandler {
public:
        FSLEHandler(FWnd *pwnd);
        Bool Dispatch(FEvent &ev);
protected:
        virtual Bool wm_cut() =0;
        virtual Bool wm_copy() =0;
        virtual Bool wm_clear() =0;
        virtual Bool wm_paste() =0;
        virtual Bool em_settextlimit(unsigned limit) =0;
        virtual unsigned em_queryfirstchar() =0;
        virtual Bool em_setfirstchar(unsigned pos) =0;
        virtual Bool em_queryreadonly() =0;
        virtual Bool em_setreadonly(Bool f) =0;
        virtual void em_queryselection(int *start, int *end) =0;
        virtual Bool em_setselection(int start, int end) =0;
        virtual void text_changed() =0;
};

class BIFCLASS FSLE : public FControl, 
                      public FControlTextHandler,
                      public FPaintHandler,
                      public FKeyboardHandler,
                      public FMouseHandler,
                      public FFocusHandler,
                      public FEnableHandler,
                      public FCaretHandler,
                      public FTimerHandler,
                      public FSLEHandler
{
        int display_start;
        int caret_pos;
        int sel_start,sel_end;
        unsigned textlimit;
        Bool insertMode;

        void updateCaretPos();
public:
        FSLE()
          : FControl(), 
            FControlTextHandler(this),
            FPaintHandler(this),
            FKeyboardHandler(this),
            FMouseHandler(this),
            FFocusHandler(this),
            FEnableHandler(this),
            FCaretHandler(this),
            FTimerHandler(this),
            FSLEHandler(this)
          { }

        enum {
                sle_password    = 0x0001,
                sle_autohscroll = 0x0002,
                sle_readonly    = 0x0004,
                sle_border      = 0x0008
        };

        Bool Create(FWnd *pParent, int resID);
        Bool Create(FWnd *pParent, int ID, FRect *rect, long style=sle_border, const char *pszText=0);


        //clipboard operations
        Bool Cut();
        Bool Copy();
        Bool Clear();
        Bool Paste();

        //text operations
        Bool LimitText(int limit);

        //readonly
        Bool IsReadonly();
        Bool SetReadonly(Bool f);

        //selection
        Bool GetSelection(int *start, int *end);
        Bool SetSelection(int start, int end);
protected:
        virtual Bool Paint() {return False;} //we don't use FWMSControlHandler's paint portion
        virtual Bool Paint(FPaintEvent &);
        virtual Bool LostFocus(FKillFocusEvent&);
        virtual Bool GotFocus(FSetFocusEvent&);
        virtual Bool CharInput(FKeyboardEvent&);
        virtual Bool KeyPressed(FKeyboardEvent&);
        virtual Bool Enabled(FEnableEvent&);
        virtual Bool Disabled(FEnableEvent&);
        virtual Bool Button1Down(FButtonDownEvent&);
        virtual Bool Button1Up(FButtonUpEvent&);
        virtual Bool Timer(FTimerEvent &);

        unsigned QueryDlgCode() { return DLGC_SLE; }
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);

        virtual Bool wm_cut();
        virtual Bool wm_copy();
        virtual Bool wm_clear();
        virtual Bool wm_paste();
        virtual Bool em_settextlimit(unsigned limit);
        virtual unsigned em_queryfirstchar();
        virtual Bool em_setfirstchar(unsigned pos);
        virtual Bool em_queryreadonly();
        virtual Bool em_setreadonly(Bool f);
        virtual void em_queryselection(int *start, int *end);
        virtual Bool em_setselection(int start, int end);
        virtual void text_changed();
};

#endif
