#ifndef __SIZEREST_H_INCLUDED
#define __SIZEREST_H_INCLUDED

class BIFCLASS FQueryTrackInfoEvent : public FEvent {
public:
        USHORT GetFlags() const { return SHORT1FROMMP(GetMP1()); }
        PTRACKINFO GetTrackInfo() { return (PTRACKINFO)PVOIDFROMMP(GetMP2()); }
};

class FFrameWindow;
class FSizeRestrictionClientHandler;
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
        FSizeRestrictionClientHandler(FWnd* pwnd, FSizeRestrictionFrameHandler  *pframehandler)
          : FHandler(pwnd),
            pFrame(pframehandler->GetWnd())
          { pframehandler->clienthandler=this; }

        Bool ApplySizeRestrictions();

protected:
        friend class FSizeRestrictionFrameHandler;
        Bool QueryTrackInfo(FQueryTrackInfoEvent&);     //os/2
        Bool AdjustParentPos(FEvent &ev);               //os/2

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

