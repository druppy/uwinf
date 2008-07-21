#ifndef __APPWND_H_INCLUDED
#define __APPWND_H_INCLUDED

class BIFCLASS FAppWindow;

class BIFCLASS FClientForwardHandler : public FHandler {
public:
        FClientForwardHandler(FWnd *pwnd);
protected:
        Bool Dispatch(FEvent& ev);
};

class BIFCLASS FAppWindowFocusForwarder : public FFocusHandler {
        FAppWindow *appwnd;
public:
        FAppWindowFocusForwarder(FAppWindow *pwnd);
        Bool GotFocus(FSetFocusEvent& ev);
};

class BIFCLASS FAppWindowFormatFrameHandler : public FHandler {
public: 
        FAppWindowFormatFrameHandler(FWnd *pwnd);
        Bool Dispatch(FEvent &ev);
};

class BIFCLASS FClientWindow;
class BIFCLASS FAppWindow : public FFrameWindow ,
                            private FClientForwardHandler
{
public:
        FAppWindow();
        ~FAppWindow();

        FWnd *SetClient(FWnd *pNewClient);
        FWnd *GetClient();

        Bool SavePosition();
        Bool RestorePosition();
private:
        FWnd *theClient;
        FAppWindowFocusForwarder focusforwarder;
        FAppWindowFormatFrameHandler formatframehandler;
        friend class FAppWindowFocusForwarder;
};


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

