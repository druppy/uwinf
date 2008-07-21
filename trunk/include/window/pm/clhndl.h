#ifndef __CLHNDL_H_INCLUDED
#define __CLHNDL_H_INCLUDED

//client handlers

//paint-----------------------------------------------------------------------
class BIFCLASS FPaintEvent : public FEvent {
};

class BIFCLASS FPaintHandler : public FHandler {
public:
        FPaintHandler(FWnd *pwnd);

        void Invalidate(FRect *pr=0, Bool includeDescendands=True);

protected:
        Bool Dispatch(FEvent&);

        virtual Bool Paint(FPaintEvent&) { return False; }
};

//caret-----------------------------------------------------------------------
class BIFCLASS FCaretHandler : public FHandler {
public:
        FCaretHandler(FWnd *pwnd);

        Bool SetCaretPos(int x, int y);

        enum Shape {
                //PM only:
                UnderscoreCaret,
                VerticalCaret,
                BlockCaret,
                //portable:
                InsertCaret = UnderscoreCaret,
                OverwriteCaret = BlockCaret
        };
        Bool SetCaretShape(Shape s) {
                return SetCaretShape(s,cx,cy);
        }
        Bool SetCaretShape(Shape s, int w, int h);

        Bool ShowCaret();
        Bool HideCaret();

protected:
        Bool Dispatch(FEvent &);

private:
        void UpdateCaret();
        void UpdateCaretPos();
        int cx,cy;
        int cw,ch;
        Shape cs;
        int cshow;
        Bool isCaretOwner;
};


//timer-----------------------------------------------------------------------
class BIFCLASS FTimerEvent : public FEvent {
public:
        int GetTimerID() { return SHORT1FROMMP(GetMP1()); }
};

class BIFCLASS FTimerHandler : public FHandler {
public:
        FTimerHandler(FWnd *pwnd)
          : FHandler(pwnd)
         { SetDispatch(dispatchFunc_t(FTimerHandler::Dispatch)); }
        Bool StartTimer(int interval, int timerid);
        Bool StopTimer(int timerid);
protected:
        Bool Dispatch(FEvent &);
        virtual Bool Timer(FTimerEvent &) { return False; }
};

#endif
