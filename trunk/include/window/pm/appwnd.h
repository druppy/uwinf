#ifndef __APPWND_H_INCLUDED
#define __APPWND_H_INCLUDED


class FAppWindow;

class BIFCLASS FCommandEater : public FHandler {
public:
        FCommandEater(FAppWindow *pwnd);
protected:
        Bool Dispatch(FEvent &);
};


class FClientWindow;
class BIFCLASS FAppWindow : public FFrameWindow, private FCommandEater
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
};


class BIFCLASS FClientWindow : public FWnd, public FSizeHandler {
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