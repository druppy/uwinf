#ifndef __BW_HPP
#define __BW_HPP

//special "windows"
#define HWND_TOP        ((WmsHWND)1)
#define HWND_BOTTOM     ((WmsHWND)2)


//predefined styles
#define WS_VISIBLE      0x00010000L
#define WS_CLIPCHILDREN 0x00020000L
#define WS_CLIPSIBLINGS 0x00040000L
#define WS_DISABLED     0x00080000L


//ownerparent relationship
WmsHWND WmsQueryWindowOwner(WmsHWND hwnd);
Bool WmsSetWindowOwner(WmsHWND hwnd, WmsHWND hwndOwner);
WmsHWND WmsQueryWindowParent(WmsHWND hwnd);
Bool WmsSetWindowParent(WmsHWND hwnd, WmsHWND hwndParent);
Bool WmsIsChild(WmsHWND hwndParent, WmsHWND hwndChild);
Bool WmsIsOwnee(WmsHWND hwndOwner, WmsHWND hwndOwnee);


//window enumeration
struct ChildEnum;
ChildEnum *WmsBeginEnum(WmsHWND hwndParent);
WmsHWND WmsQueryNextWindow(ChildEnum *ce);
void WmsEndEnum(ChildEnum *ce);

#define QW_PARENT       1
#define QW_OWNER        2
#define QW_FIRSTSIBLING 3
#define QW_PREVSIBLING  4
#define QW_NEXTSIBLING  5
#define QW_LASTSIBLING  6
#define QW_FIRSTCHILD   7
#define QW_PREVOWNEE    8
#define QW_NEXTOWNEE    9
#define QW_FIRSTOWNEE   10
WmsHWND WmsQueryWindow(WmsHWND hwnd, int fl);

//style
WmsHWND WmsSearchStyleBit(WmsHWND hwnd, uint32 b, uint32 v);
#define WmsIsWindowVisible(hwnd) (WmsSearchStyleBit(hwnd,WS_VISIBLE,0)==0)
#define WmsIsWindowEnabled(hwnd) (WmsSearchStyleBit(hwnd,WS_DISABLED,WS_DISABLED)==0)
uint32 WmsQueryWindowStyle(WmsHWND hwnd);
Bool WmsSetWindowStyle(WmsHWND hwnd, uint32 s);
Bool WmsSetWindowStyleBits(WmsHWND hwnd, uint32 bits, uint32 mask);

typedef void (*SetStyleHook_f)(WmsHWND, uint32 oldStyle, uint32 newStyle);
SetStyleHook_f WmsSetSetStyleHook(SetStyleHook_f pfn);


//window ID
uint16 WmsQueryWindowID(WmsHWND hwnd);
Bool WmsSetWindowID(WmsHWND hwnd, uint16 ID);
WmsHWND WmsWindowFromID(WmsHWND hwndParent, uint16 ID);


//user data
uint32 WmsQueryWindowUserData(WmsHWND hwnd);
Bool WmsSetWindowUserData(WmsHWND hwnd, uint32 ud);

//queue
WmsHMQ WmsQueryWindowMsgQueue(WmsHWND hwnd);


#define SWP_MOVE        0x0001
#define SWP_SIZE        0x0002
#define SWP_HIDE        0x0004
#define SWP_SHOW        0x0008
#define SWP_ZORDER      0x0010
#define SWP_CHANGING    0x8000

struct WmsSWP {
        int flags;
        int cx,cy;
        int x,y;
        WmsHWND hwndBehind;
        WmsHWND hwnd;
};

Bool WmsSetWindowPos(WmsHWND hwnd,
                     WmsHWND hwndBehind,
                     int x, int y,
                     int cx, int cy,
                     int flags
                    );
Bool WmsSetMultWindowPos(WmsSWP *pswp, unsigned swps);
Bool WmsQueryWindowRect(WmsHWND hwnd, FRect *prect);
Bool WmsMapWindowPoints(WmsHWND hwndFrom, WmsHWND hwndTo, FPoint *p, int points);
Bool WmsMapWindowRects(WmsHWND hwndFrom, WmsHWND hwndTo, FRect *r, int rects);
WmsHWND WmsWindowFromPoint(WmsHWND hwndTop, const FPoint *p, Bool fTestChildren, WmsHWND hwndBelow);
typedef void (*SWPHook_f)(WmsSWP *swp);
SWPHook_f WmsSetSWPHook(SWPHook_f pfn);

Bool WmsInvalidateRect(WmsHWND hwnd, const FRect *rect, Bool includeChildren);
Bool WmsValidateRect(WmsHWND hwnd, const FRect *rect, Bool includeChildren);
Bool WmsEnableWindowUpdate(WmsHWND hwnd, Bool visible);
Bool WmsQueryWindowUpdateRegion(WmsHWND hwnd, Region *r);
Bool WmsQueryWindowUpdateRect(WmsHWND hwnd, FRect *r);
Bool WmsQueryWindowClipRegion(WmsHWND hwnd, Region *r);
WmsHWND WmsQueryInvalidWindow();

WmsHWND WmsCreateWindow(WmsHWND hwndParent,
                     int x, int y,
                     int cx, int cy,
                     WmsHWND hwndBehind,
                     WmsHWND hwndOwner,
                     uint32 style,
                     uint16 id,
                     uint32 userData
                    );
Bool WmsDestroyWindow(WmsHWND hwnd);
Bool WmsIsWindow(WmsHWND hwnd);

typedef void (*DestroyHook_f)(WmsHWND);
DestroyHook_f WmsSetPreDestroyHook(DestroyHook_f pfn);
DestroyHook_f WmsSetPostDestroyHook(DestroyHook_f pfn);


#define QSR_CLIPCHILDREN        0x0001
#define QSR_NOCLIPCHILDREN      0x0002
#define QSR_CLIPSIBLINGSABOVE   0x0004
#define QSR_NOCLIPSIBLINGSABOVE 0x0008
#define QSR_CLIPSIBLINGSBELOW   0x0010
#define QSR_NOCLIPSIBLINGSBELOW 0x0020
#define QSR_PARENTCLIP          0x0040

Bool WmsQuerySpecialRegion(WmsHWND hwnd, uint32 fl, Region *rgn);

#endif
