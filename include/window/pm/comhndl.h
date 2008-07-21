#ifndef __COMHNDL_H_INCLUDED
#define __COMHNDL_H_INCLUDED

//common handlers

//create/destroy
class BIFCLASS FCreatedEvent : public FEvent {
};

class BIFCLASS FDestroyingEvent : public FEvent {
};

class BIFCLASS FCreateDestroyHandler : public FHandler {
public:
        FCreateDestroyHandler(FWnd *pwnd);

protected:
        Bool Dispatch(FEvent &ev);

        virtual Bool Created(FCreatedEvent &) { return False; }
        virtual Bool Destroying(FDestroyingEvent &) { return False; }
};

//command----------------------------------------------------------------------
class BIFCLASS FCommandEvent : public FEvent {
public:
        int GetItem() const { return SHORT1FROMMP(GetMP1()); }
        USHORT GetSourceType() const { return SHORT1FROMMP(GetMP2()); }         //os/2
        Bool IsPointerOperation() const { return Bool(SHORT2FROMMP(GetMP2()));} //os/22
};

class BIFCLASS FSysCommandEvent : public FEvent {
public:
        int GetItem() const { return SHORT1FROMMP(GetMP1()); }
        USHORT GetSourceType() const { return SHORT1FROMMP(GetMP2()); }         //os/2
        Bool IsPointerOperation() const { return Bool(SHORT2FROMMP(GetMP2()));} //os/2
};

class BIFCLASS FCommandHandler : public FHandler {
public:
        FCommandHandler(FWnd *pwnd);
protected:
        Bool Dispatch(FEvent& ev);

        virtual Bool Command(FCommandEvent&) { return False; }
        virtual Bool SysCommand(FSysCommandEvent&) { return False; }
};


//enabled----------------------------------------------------------------------
class BIFCLASS FEnableEvent : public FEvent {
public:
        Bool GetFlag() const { return Bool(SHORT1FROMMP(GetMP1())); }   //os/2
};

class BIFCLASS FEnableHandler: public FHandler {
public:
        FEnableHandler(FWnd *pwnd);

        //Enable()
        //Disable()
        //IsEnabled()
protected:
        Bool Dispatch(FEvent&);

        virtual Bool Enabled(FEnableEvent&) { return False; }
        virtual Bool Disabled(FEnableEvent&) { return False; }
};

//focus----------------------------------------------------------------------
class BIFCLASS FFocusEvent_base : public FEvent {
public:
        Bool GetFocusFlag() const { return Bool(SHORT1FROMMP(GetMP2())); }
};

class BIFCLASS FSetFocusEvent : public FFocusEvent_base  {
public:
        HWND GetLosingWindow() const { return HWNDFROMMP(GetMP1()); }
};

class BIFCLASS FKillFocusEvent : public FFocusEvent_base  {
public:
        HWND GetGettingWindow() const { return HWNDFROMMP(GetMP1()); }
};

class BIFCLASS FFocusHandler : public FHandler {
        static ULONG wm_forwardfocus;
        static int useCount;
public:
        FFocusHandler(FWnd *pwnd);
        ~FFocusHandler();

        Bool SetFocus();
        Bool HasFocus();

protected:
        Bool Dispatch(FEvent&);

        virtual Bool LostFocus(FKillFocusEvent&) { return False; }
        virtual Bool GotFocus(FSetFocusEvent&) { return False; }

        Bool ForwardFocus(FWnd *pwnd);
};

//size----------------------------------------------------------------------
class BIFCLASS FSizeEvent : public FEvent {
public:
        int GetOldWidth() const { return SHORT1FROMMP(GetMP2()); }      //os/2
        int GetOldHeight() const { return SHORT2FROMMP(GetMP2()); }     //os/2
        int GetWidth() const { return SHORT1FROMMP(GetMP2()); }
        int GetHeight() const { return SHORT2FROMMP(GetMP2()); }
};

class BIFCLASS FSizeHandler : public FHandler {
public:
        FSizeHandler(FWnd *pwnd);

        Bool SetSize(int w, int h);
        Bool GetSize(int *w, int *h);

protected:
        Bool Dispatch(FEvent& ev);

        virtual Bool SizeChanged(FSizeEvent&) { return False; }
};


//position--------------------------------------------------------------------
class FMoveHandler;
class BIFCLASS FMoveEvent : public FEvent {
        int X,Y;
        friend class FMoveHandler;
public:
        int GetX() const { return X; }
        int GetY() const { return Y; }
};


class BIFCLASS FMoveHandler : public FHandler {
public:
        FMoveHandler(FWnd *pwnd);

        Bool Move(int x, int y);
        Bool GetPosition(int *x, int *y);

protected:
        Bool Dispatch(FEvent& ev);

        virtual Bool Moved(FMoveEvent&) { return False; }
};


//close----------------------------------------------------------------------

class BIFCLASS FCloseHandler : public FHandler {
public:
        FCloseHandler(FWnd *pwnd);

        void Close();
        void CloseUnconditionally();

protected:
        Bool Dispatch(FEvent& ev);

        virtual Bool Close(FEvent &) { return False; }
        virtual Bool CloseUnconditionally(FEvent &) { return False; }
};


#endif
