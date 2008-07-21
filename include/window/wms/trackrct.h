#ifndef __TRACKRCT_H_INCLUDED
#define __TRACKRCT_H_INCLUDED

#define TF_ALLINBOUNDARY        0x0001
#define TF_LEFT                 0x0002
#define TF_TOP                  0x0004
#define TF_RIGHT                0x0008
#define TF_BOTTOM               0x0010
#define TF_SIZE                 0x0020
#define TF_MOVE                 0x0040
#define TF_SETPOINTERPOS        0x0080

struct TrackInfo {
        FRect track;
        FRect bounding;
        unsigned minWidth;
        unsigned minHeight;
        unsigned maxWidth;
        unsigned maxHeight;
        uint32 fl;
};

Bool TrackRect(FWnd *pwnd, uint32 flDC, TrackInfo *info);

#endif
