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
        int GetItem() const { return GetWParam(); }
        HWND GetHwnd() const { return (HWND)LOWORD(lParam); }
        WORD GetNotifyCode() const { return HIWORD(lParam); }
};

class BIFCLASS FSysCommandEvent : public FEvent {
public:
        int GetItem() const { return GetWParam(); }
        int GetX() const { return LOWORD(lParam); }
        int GetY() const { return HIWORD(lParam); }
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
        Bool GetFlag() const { return (Bool)wParam; }
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
class BIFCLASS FSetFocusEvent : public FEvent {
public:
        HWND GetLosingWindow() const { return (HWND)wParam; }
};

class BIFCLASS FKillFocusEvent : public FEvent {
public:
        HWND GetGettingWindow() const { return (HWND)wParam; }
};

class BIFCLASS FFocusHandler : public FHandler {
public:
        FFocusHandler(FWnd *pwnd);

        Bool HasFocus();
        Bool SetFocus();
protected:
        Bool Dispatch(FEvent&);

        virtual Bool LostFocus(FKillFocusEvent&) { return False; }
        virtual Bool GotFocus(FSetFocusEvent&) { return False; }

        Bool ForwardFocus(FWnd *pwnd);
};

//size----------------------------------------------------------------------
class BIFCLASS FSizeEvent : public FEvent {
public:
        int GetWidth() const { return LOWORD(GetLParam()); }
        int GetHeight() const { return HIWORD(GetLParam()); }
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

class BIFCLASS FMoveEvent : public FEvent {
public:
        int GetX() const { return LOWORD(GetLParam()); }
        int GetY() const { return HIWORD(GetLParam()); }
};


class BIFCLASS FMoveHandler : public FHandler {
        FWnd *pWnd;
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
        UINT wm_saveapplication;
public:
        FCloseHandler(FWnd *pwnd);

        void Close();
        void CloseUnconditionally();

protected:
        Bool Dispatch(FEvent& ev);
        
        virtual Bool Close(FEvent &) { return False; }
        virtual Bool CloseUnconditionally(FEvent &) { return False; }
};


#endif //__COMHNDL_H_INCLUDED

