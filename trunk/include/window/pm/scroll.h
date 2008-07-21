#ifndef __SCROLL_H_INCLUDED
#define __SCROLL_H_INCLUDED

class FClientWindow;

class BIFCLASS FScrollEvent : public FEvent {
public:
        enum type {
                start, end,
                oneLess, oneMore,
                muchLess, muchMore,
                track, setPos,
                endScroll
        };
        type GetType() const;
        int where() const;
        int GetIdentifier() const { return SHORT1FROMMP(GetMP1()); }     //os/2
};

class BIFCLASS FClientScrollHandler : public FHandler {
        FClientWindow *client;
        HWND hwndVScroll,hwndHScroll;
protected:
        FClientScrollHandler(FClientWindow *pwnd);
public:
        int GetHScrollPos();
        Bool SetHScrollPos(int pos);
        Bool GetHScrollRange(int *low, int *high);
        Bool SetHScrollThumbSize(int visible, int total);
        Bool SetHScrollBar(int pos, int low, int high);

        int GetVScrollPos();
        Bool SetVScrollPos(int pos);
        Bool GetVScrollRange(int *low, int *high);
        Bool SetVScrollThumbSize(int visible, int total);
        Bool SetVScrollBar(int pos, int low, int high);
protected:
        Bool Dispatch(FEvent&);

        virtual Bool HScroll(FScrollEvent&);
        virtual Bool VScroll(FScrollEvent&);

        virtual Bool HScrollLineLeft(FScrollEvent&) { return False; }
        virtual Bool HScrollLineRight(FScrollEvent&) { return False; }
        virtual Bool HScrollPageLeft(FScrollEvent&) { return False; }
        virtual Bool HScrollPageRight(FScrollEvent&) { return False; }
        virtual Bool HScrollStart(FScrollEvent&) { return False; }
        virtual Bool HScrollEnd(FScrollEvent&) { return False; }
        virtual Bool HScrollTo(FScrollEvent&) { return False; }
        virtual Bool HScrollTrack(FScrollEvent&) { return False; }

        virtual Bool VScrollLineUp(FScrollEvent&) { return False; }
        virtual Bool VScrollLineDown(FScrollEvent&) { return False; }
        virtual Bool VScrollPageUp(FScrollEvent&) { return False; }
        virtual Bool VScrollPageDown(FScrollEvent&) { return False; }
        virtual Bool VScrollTop(FScrollEvent&) { return False; }
        virtual Bool VScrollBottom(FScrollEvent&) { return False; }
        virtual Bool VScrollTo(FScrollEvent&) { return False; }
        virtual Bool VScrollTrack(FScrollEvent&) { return False; }
};


#endif
