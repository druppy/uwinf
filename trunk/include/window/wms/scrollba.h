#ifndef __SCROLLBA_H_INCLUDED
#define __SCROLLBA_H_INCLUDED

#ifndef __CLHNDL_H_INCLUDED
#  include <window\wms\clhndl.h>
#endif
#ifndef __KEYBOARD_H_INCLUDED
#  include <window\wms\keyboard.h>
#endif
#ifndef __MOUSE_H_INCLUDED
#  include <window\wms\mouse.h>
#endif

//scrollbar notifications:
#define SBN_START       0x0001
#define SBN_MUCHLESS    0x0002
#define SBN_ONELESS     0x0003
#define SBN_ONEMORE     0x0004
#define SBN_MUCHMORE    0x0005
#define SBN_END         0x0006
#define SBN_STARTTRACK  0x0007
#define SBN_TRACK       0x0008
#define SBN_ENDTRACK    0x0009
#define SBN_SETPOS      0x000a

class FScrollBarHandler : public FHandler {
public:
        FScrollBarHandler(FWnd *pwnd)
          : FHandler(pwnd)
          { SetDispatch(dispatchFunc_t(FScrollBarHandler::Dispatch)); }
protected:
        Bool Dispatch(FEvent &ev);

        virtual long sbm_querypos() =0;
        virtual Bool sbm_setpos(long pos) =0;
        virtual long sbm_queryrangestart() =0;
        virtual long sbm_queryrangeend() =0;
        virtual Bool sbm_setrange(long start, long end) =0;
        virtual Bool sbm_setthumbsize(long visible, long total) =0;
};

class FScrollBar : public FControl,
                   public FKeyboardHandler,
                   public FMouseHandler,
                   public FTimerHandler,
                   public FScrollBarHandler
{
        sint32 range_start,range_end;
        sint32 amount_visible, total_amount;
        sint32 current_pos;
        sint32 current_trackpos;

        void doOneLess();
        void doOneMore();
        void doMuchLess();
        void doMuchMore();

        enum { dooneless, domuchless, track, domuchmore, doonemore } mouse_action;
public:
        FScrollBar()
          : FControl(),
            FKeyboardHandler(this),
            FMouseHandler(this),
            FTimerHandler(this),
            FScrollBarHandler(this)
           {}

        enum scrollbar_orientation { horizontal=0x0000, vertical=0x0001 };
        Bool Create(FWnd *pParent, int resID);
        Bool Create(FWnd *pParent, int ID,
                    scrollbar_orientation orientation,
                    FRect *rect
                   );

        long GetScrollPos();
        Bool SetScrollPos(long pos);

        Bool GetScrollRange(long *start, long *end);
        Bool SetScrollRange(long start, long end);

        Bool SetThumbSize(long visible, long total);
protected:
        Bool Paint();

        virtual Bool KeyPressed(FKeyboardEvent&);

        virtual Bool Button1Down(FButtonDownEvent &ev);
        virtual Bool Button1Up(FButtonUpEvent &ev);
        virtual Bool MouseMove(FMouseMoveEvent &ev);

        virtual Bool Timer(FTimerEvent &ev);

        unsigned QueryDlgCode() { return DLGC_SCROLLBAR; }
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);

        virtual long sbm_querypos();
        virtual Bool sbm_setpos(long pos);
        virtual long sbm_queryrangestart();
        virtual long sbm_queryrangeend();
        virtual Bool sbm_setrange(long start, long end);
        virtual Bool sbm_setthumbsize(long visible, long total);
};

#endif
