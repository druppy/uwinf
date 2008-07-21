#ifndef __MOUSE_H_INCLUDED
#define __MOUSE_H_INCLUDED

//mouse -----------------------------------------------------------------------
//Left button=1 right button=2, middle button=3
class BIFCLASS FMouseEvent : public FEvent {
public:
        Bool IsButtonDown(int b) const;
        Bool IsButtonDown1() const { return IsButtonDown(1); }
        Bool IsButtonDown2() const { return IsButtonDown(2); }
        Bool IsButtonDown3() const { return IsButtonDown(3); }
        Bool IsCtrlDown() const;
        Bool IsShiftDown() const;

        int GetX() const { return (short)(SHORT1FROMMP(GetMP1())); }
        int GetY() const { return (short)(SHORT2FROMMP(GetMP1())); }
        FPoint GetPoint() const { return FPoint(GetX(),GetY()); }
};

class BIFCLASS FButtonDownEvent : public FMouseEvent {
public:
        int GetButton() const;
};

class BIFCLASS FButtonUpEvent : public FMouseEvent {
public:
        int GetButton() const;
};

class BIFCLASS FButtonDblClkEvent : public FMouseEvent {
public:
        int GetButton() const;
};

class BIFCLASS FMouseMoveEvent : public FMouseEvent {
public:
};

class BIFCLASS FControlPointerEvent : public FEvent {
public:
        void SetPointer(FPointer& p);
};

class BIFCLASS FMouseHandler : public FHandler {
public:
        FMouseHandler(FWnd *pwnd);
protected:
        Bool Dispatch(FEvent&);

        Bool SetCapture();
        Bool ReleaseCapture();
        Bool HasCapture() const { return (Bool)hasCapture; }

        Bool GetCurrentMousePosition(int *x, int *y);

        //notifications
        virtual Bool ButtonDown(FButtonDownEvent&);
        virtual Bool ButtonUp(FButtonUpEvent&);
        virtual Bool ButtonDblClk(FButtonDblClkEvent&);
        virtual Bool Button1Down(FButtonDownEvent&) { return False; }
        virtual Bool Button1Up(FButtonUpEvent&) { return False; }
        virtual Bool Button1DblClk(FButtonDblClkEvent&) { return False; }
        virtual Bool Button2Down(FButtonDownEvent&) { return False; }
        virtual Bool Button2Up(FButtonUpEvent&) { return False; }
        virtual Bool Button2DblClk(FButtonDblClkEvent&) { return False; }
        virtual Bool Button3Down(FButtonDownEvent&) { return False; }
        virtual Bool Button3Up(FButtonUpEvent&) { return False; }
        virtual Bool Button3DblClk(FButtonDblClkEvent&) { return False; }
        virtual Bool MouseMove(FMouseMoveEvent&) { return False; }
        virtual Bool ControlPointer(FControlPointerEvent&) { return False; }
        virtual Bool ControlPointerNonWindow(FControlPointerEvent&) { return False; }
private:
        int hasCapture;
};


class BIFCLASS FExtendedMouseHandler : public FMouseHandler {
public:
        FExtendedMouseHandler(FWnd *pwnd);
protected:
        Bool Dispatch(FEvent&);

        //notifications
        virtual Bool Button1MotionStart(FMouseEvent&) { return False; }
        virtual Bool Button1MotionEnd(FMouseEvent&) { return False; }
        virtual Bool Button1Click(FMouseEvent&) { return False; }
        virtual Bool Button2MotionStart(FMouseEvent&) { return False; }
        virtual Bool Button2MotionEnd(FMouseEvent&) { return False; }
        virtual Bool Button2Click(FMouseEvent&) { return False; }
        virtual Bool Button3MotionStart(FMouseEvent&) { return False; }
        virtual Bool Button3MotionEnd(FMouseEvent&) { return False; }
        virtual Bool Button3Click(FMouseEvent&) { return False; }
        virtual Bool ButtonChord(FMouseEvent&) { return False; }
};


class BIFCLASS FTranslatedMouseEvent : public FMouseEvent {
public:
        Bool IsFromMouse() const { return Bool(SHORT1FROMMP(GetMP2())); }
};

class BIFCLASS FTranslatedMouseHandler : public FExtendedMouseHandler {
public:
        FTranslatedMouseHandler(FWnd *pwnd);

protected:
        Bool Dispatch(FEvent&);

        //notifications
        virtual Bool MouseDragStart(FMouseEvent&) { return False; }
        virtual Bool MouseDragEnd(FMouseEvent&) { return False; }
        virtual Bool MouseSingleSelect(FMouseEvent&) { return False; }
        virtual Bool MouseOpen(FMouseEvent&) { return False; }
        virtual Bool MouseContextMenu(FMouseEvent&) { return False; }
        virtual Bool MouseTextEdit(FMouseEvent&) { return False; }
        virtual Bool MouseSelectStart(FMouseEvent&) { return False; }
        virtual Bool MouseSelectEnd(FMouseEvent&) { return False; }
};


#endif //__MOUSE_H_INCLUDED
