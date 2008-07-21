#ifndef __SIZEREST_H_INCLUDED
#define __SIZEREST_H_INCLUDED

class BIFCLASS FGetMinMaxInfoEvent : public FEvent {
public:
        MINMAXINFO FAR *GetMMI()
          { return (MINMAXINFO FAR*)lParam; }        //win16
};

class BIFCLASS FFrameWindow;
class BIFCLASS FSizeRestrictionClientHandler;
class BIFCLASS FSizeRestrictionFrameHandler : public FHandler {
        friend class FSizeRestrictionClientHandler;
        FSizeRestrictionClientHandler *clienthandler;
public:
        FSizeRestrictionFrameHandler(FFrameWindow* pwnd);
        Bool ApplySizeRestrictions();

protected:
        Bool Dispatch(FEvent&);
};

class BIFCLASS FSizeRestrictionClientHandler : public FHandler {
public:
        FSizeRestrictionClientHandler(FWnd* pwnd, FSizeRestrictionFrameHandler *pframehandler)
          : FHandler(pwnd),
            pFrame(pframehandler->GetWnd())
          { pframehandler->clienthandler=this; }

        Bool ApplySizeRestrictions();

protected:
        friend class FSizeRestrictionFrameHandler;
        void ModifyMMI(FGetMinMaxInfoEvent&);     //win16

        virtual Bool GetMinimumWidth(int &) {return False;}
        virtual Bool GetMinimumHeight(int &) {return False;}
        virtual Bool GetMaximumWidth(int &) {return False;}
        virtual Bool GetMaximumHeight(int &) {return False;}
        virtual Bool GetHorizontalGrid(int &) {return False;}
        virtual Bool GetVerticalGrid(int &) {return False;}

private:
        FWnd *pFrame;
};

#endif

