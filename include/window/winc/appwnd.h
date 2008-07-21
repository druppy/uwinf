#ifndef __APPWND_H_INCLUDED
#define __APPWND_H_INCLUDED

class BIFCLASS FAppWindow;

class BIFCLASS FClientForwardHandler : public FHandler {
public:
        FClientForwardHandler(FWnd *pwnd);
protected:
        Bool Dispatch(FEvent& ev);
        virtual Bool ForwardToClient(FEvent& ev) =0;
};

class BIFCLASS FAppWindowResizeHandler : public FSizeHandler {
public:
        FAppWindowResizeHandler(FWnd *pwnd)
          : FSizeHandler(pwnd)
          {}
        Bool SizeChanged(FSizeEvent& ev);
};

class BIFCLASS FFrameFormatHandler : public FHandler {
public:
        FFrameFormatHandler(FWnd *pwnd);
protected:
        Bool Dispatch(FEvent &ev);

        virtual Bool UpdateFrame(FEvent &) { return False; }
        virtual Bool QueryFrameCtlCount(FEvent &) { return False; }
        virtual Bool FormatFrame(FEvent &) { return False; }
};

class BIFCLASS FAppWindowFocusForwarder : public FFocusHandler {
        FAppWindow *appwnd;
public:
        FAppWindowFocusForwarder(FAppWindow *pwnd);
        Bool GotFocus(FSetFocusEvent& ev);
};

class BIFCLASS FClientWindow;
class BIFCLASS FAppWindow : public FFrameWindow,
                            private FClientForwardHandler,
                            protected FFrameFormatHandler
{
public:
        FAppWindow();
        ~FAppWindow();

        FWnd *SetClient(FWnd *pNewClient);
        FWnd *GetClient();

        Bool SavePosition();
        Bool RestorePosition();
protected:
        Bool ForwardToClient(FEvent& ev);
        
        Bool UpdateFrame(FEvent &);
        Bool QueryFrameCtlCount(FEvent &);
        Bool FormatFrame(FEvent &);
private:
        FWnd *theClient;
        FAppWindowResizeHandler resizehandler;
        FAppWindowFocusForwarder focusforwarder;
        friend class FAppWindowFocusForwarder;
};


#define FID_CLIENT 0x8008

class BIFCLASS FClientWindow : public FWnd,
                               public FSizeHandler
{
public:
        FClientWindow()
          : FWnd(), FSizeHandler(this)
          { appwnd=0; }
        ~FClientWindow();

        Bool Create(FAppWindow *pParent);

        FAppWindow *GetAppWindow() { return appwnd; }
        
private:
        FAppWindow *appwnd;
};

#endif

