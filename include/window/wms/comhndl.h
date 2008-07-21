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
#define CMDSRC_OTHER         0x0000
#define CMDSRC_PUSHBUTTON    0x0001
#define CMDSRC_MENU          0x0002
#define CMDSRC_ACCELERATOR   0x0003

class BIFCLASS FCommandEvent : public FEvent {
public:
        int GetItem() const { return UINT161FROMMP(GetMP1()); }
        uint16 GetSourceType() const { return UINT161FROMMP(GetMP2()); }         //os/2
        Bool IsPointerOperation() const { return Bool(UINT162FROMMP(GetMP2()));} //os/22
};

class BIFCLASS FSysCommandEvent : public FEvent {
public:
        int GetItem() const { return UINT161FROMMP(GetMP1()); }
        uint16 GetSourceType() const { return UINT161FROMMP(GetMP2()); }         //os/2
        Bool IsPointerOperation() const { return Bool(UINT162FROMMP(GetMP2()));} //os/2
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
        Bool GetFlag() const { return Bool(UINT161FROMMP(GetMP1())); }   //os/2
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
        Bool GetFocusFlag() const { return Bool(UINT161FROMMP(GetMP2())); }
};

class BIFCLASS FSetFocusEvent : public FFocusEvent_base  {
public:
        WmsHWND GetLosingWindow() const { return HWNDFROMMP(GetMP1()); }
};

class BIFCLASS FKillFocusEvent : public FFocusEvent_base  {
public:
        WmsHWND GetGettingWindow() const { return HWNDFROMMP(GetMP1()); }
};

class BIFCLASS FFocusHandler : public FHandler {
        static WmsMSG wm_forwardfocus;
public:
        FFocusHandler(FWnd *pwnd);

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
        int GetWidth() const { return GetSWP()->cx; }
        int GetHeight() const { return GetSWP()->cy; }
        WmsSWP *GetSWP() const { return (WmsSWP*)PFROMMP(GetMP1()); }
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
public:
        int GetX() const { return GetSWP()->x; }
        int GetY() const { return GetSWP()->y; }
        WmsSWP *GetSWP() const { return (WmsSWP*)PFROMMP(GetMP1()); }
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