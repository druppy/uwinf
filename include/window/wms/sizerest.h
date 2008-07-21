#ifndef __SIZEREST_H_INCLUDED
#define __SIZEREST_H_INCLUDED

struct TrackInfo;
class BIFCLASS FQueryTrackInfoEvent : public FEvent {
public:
        uint16 GetFlags() const { return UINT161FROMMP(GetMP1()); }
        TrackInfo *GetTrackInfo() { return (TrackInfo*)PFROMMP(GetMP2()); }
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
        FSizeRestrictionClientHandler(FWnd* pwnd, FSizeRestrictionFrameHandler *pframehandler)
          : FHandler(pwnd),
            pFrame(pframehandler->GetWnd()),
            pFrameHandler(pframehandler)
          { pframehandler->clienthandler=this; }

        Bool ApplySizeRestrictions();

protected:
        friend class FSizeRestrictionFrameHandler;
        Bool QueryTrackInfo(FQueryTrackInfoEvent&);     //wms
        Bool AdjustParentPos(FEvent &ev);               //wms

        virtual Bool GetMinimumWidth(int &) {return False;}
        virtual Bool GetMinimumHeight(int &) {return False;}
        virtual Bool GetMaximumWidth(int &) {return False;}
        virtual Bool GetMaximumHeight(int &) {return False;}
        virtual Bool GetHorizontalGrid(int &) {return False;}
        virtual Bool GetVerticalGrid(int &) {return False;}

private:
        FWnd *pFrame;
        FSizeRestrictionFrameHandler *pFrameHandler;
};

#endif

