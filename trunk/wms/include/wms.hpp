#ifndef __WMSTYPES_DECLARED
#define __WMSTYPES_DECLARED

#define INCL_NOPMAPI
///////////////////////////////////////////////////////////////////////////////
//types/classes

class FThread;
class Region;
#include "porttype.hpp"

#define DECLARE_HANDLE(name) struct _##name##_ { int notused; }; typedef _##name##_ *name;

DECLARE_HANDLE(WmsHAB);
DECLARE_HANDLE(WmsHMQ);
DECLARE_HANDLE(WmsHWND);
DECLARE_HANDLE(WmsHDC);

struct _WmsMPARAM_ { int notused; }; typedef _WmsMPARAM_ *WmsMPARAM;
typedef uint16 WmsMSG;
struct _WmsMRESULT_ { int notused; }; typedef _WmsMRESULT_ *WmsMRESULT;
struct WmsQMSG;

typedef uint32 TimeStamp;
enum Bool {False, True};

#define MPFROMP(p)                      ((WmsMPARAM)((uint32)(p)))
#define MPFROMHWND(hwnd)                ((WmsMPARAM)(WmsHWND)(hwnd))
#define MPFROMUINT8(ch)                 ((WmsMPARAM)(uint8)(ch))
#define MPFROM4UINT8(ch1,ch2,ch3,ch4)   ((WmsMPARAM)MAKEUINT32(MAKEUINT16(ch4,ch3),MAKEUINT16(ch2,ch1)))
#define MPFROMUINT16(s)                 ((WmsMPARAM)(uint16)(s))
#define MPFROM2UINT16(s1, s2)           ((WmsMPARAM)MAKEUINT32(s2,s1))
#define MPFROMUINT32(l)                 ((WmsMPARAM)(uint32)(l))

#define PFROMMP(mp)                     ((void *)(mp))
#define HWNDFROMMP(mp)                  ((WmsHWND)(mp))
#define UINT81FROMMP(mp)                ((uint8)(mp))
#define UINT82FROMMP(mp)                ((uint8)(((uint32)(mp))>>8))
#define UINT83FROMMP(mp)                ((uint8)(((uint32)(mp))>>16))
#define UINT84FROMMP(mp)                ((uint8)(((uint32)(mp))>>24))
#define UINT161FROMMP(mp)               LOWUINT16((uint32)(mp))
#define UINT162FROMMP(mp)               HIGHUINT16((uint32)(mp))
#define UINT32FROMMP(mp)                ((uint32)(mp))

#define MRFROMP(p)                      ((WmsMRESULT)(p))
#define MRFROMHWND(hwnd)                ((WmsMRESULT)(hwnd))
#define MRFROMUINT16(s)                 ((WmsMRESULT)(uint16)(s))
#define MRFROM2UINT16(s1,s2)            ((WmsMRESULT)MAKEUINT32(s2,s1))
#define MRFROMUINT32(l)                 ((WmsMRESULT)(l))

#define PFROMMR(mp)                     ((void *)(mr))
#define HWNDFROMMR(mr)                  ((WmsHWND)(mr))
#define UINT161FROMMR(mr)               LOWUINT16((uint32)(mr))
#define UINT162FROMMR(mr)               HIGHUINT16((uint32)(mr))
#define UINT32FROMMR(mr)                ((uint32)(mr))

#include "wmsmisc.hpp"

#endif /*__WMSTYPES_DECLARED*/

Bool InitializeWMS();
Bool TerminateWMS();

#if defined(WMSINCL_ANCHORBLOCK) && !defined(WMSANCHORBLOCKAPI_DECLARED)
#define WMSANCHORBLOCKAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Anchor blocks
WmsHAB          WmsInitialize(uint32 /*options*/);
Bool            WmsTerminate(WmsHAB hab);
WmsHAB          WmsQueryThreadAnchorBlock(unsigned long tid);
WmsHMQ          WmsQueryAnchorBlockMsgQueue(WmsHAB hab);
unsigned long   WmsQueryAnchorBlockThread(WmsHAB hab);
#endif




#if defined(WMSINCL_MSGQUEUE) && !defined(WMSMSGQUEUEAPI_DECLARED)
#define WMSMSGQUEUEAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Message queues
#include "msgqueue.hpp"
#endif


#if defined(WMSINCL_BW) && !defined(WMSBWAPI_DECLARED)
#define WMSBWAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Windows
#include "bw.hpp"
#endif


#if defined(WMSINCL_REGION) && !defined(WMSREGIONAPI_DECLARED)
#define WMSREIONAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Windows
#include "region.hpp"
#endif


#if defined(WMSINCL_CPI) && !defined(WMSCPIAPI_DECLARED)
#define WMSCPIAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Character Presentation Interface
#include "cpi.hpp"
#endif


#if defined(WMSINCL_SYSVALUE) && !defined(WMSSYSVALUEAPI_DECLARED)
#define WMSSYSVALUEAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//System values
#include "sysvalue.hpp"
#endif

#if defined(WMSINCL_FOCUS) && !defined(WMSFOCUSAPI_DECLARED)
#define WMSFOCUSAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Keyboard focus
#include "focus.hpp"
#endif


#if defined(WMSINCL_CAPTURE) && !defined(WMSCAPTUREAPI_DECLARED)
#define WMSCAPTUREAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Mouse Capture
#include "capture.hpp"
#endif


#if defined(WMSINCL_DESKTOP) && !defined(WMSDESKTOPAPI_DECLARED)
#define WMSDESKTOPAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Desktop
#include "desktop.hpp"
#endif


#if defined(WMSINCL_TIMER) && !defined(WMSTIMERAPI_DECLARED)
#define WMSTIMERAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Timer API
Bool WmsStartTimer(WmsHWND hwnd, uint32 id, uint32 interval, Bool oneShot);
Bool WmsStopTimer(WmsHWND hwnd, uint32 id);
#endif


#if defined(WMSINCL_TIMESTAMP) && !defined(WMSTIMESTAMPAPI_DECLARED)
#define WMSTIMESTAMPAPI_DECLARED
///////////////////////////////////////////////////////////////////////////////
//Time-stamp API
#include "timestmp.hpp"
#endif

#if defined(WMSINCL_MSGS) && !defined(WMSMSGS_DEFINED)
#define WMSMSGS_DEFINED
///////////////////////////////////////////////////////////////////////////////
//Messages
#include "wmsmsgs.hpp"
#endif

#if defined(WMSINCL_LOGKEYS) && !defined(WMSLOGKEYS_DEFINED)
#define WMSLOGKEYS_DEFINED
///////////////////////////////////////////////////////////////////////////////
//Logical keys
#include "logkey.hpp"
#endif

#if defined(WMSINCL_KBDSTATE) && !defined(WMSKBDSTATE_DEFINED)
#define WMSKBDSTATE_DEFINED
///////////////////////////////////////////////////////////////////////////////
//Keyboard states
#include "kbdstate.hpp"
#endif

#if defined(WMSINCL_HARDWAREMOUSE) && !defined(WMSHARDWAREMOUSEAPI_DEFINED)
#define WMSHARDWAREMOUSEAPI_DEFINED
///////////////////////////////////////////////////////////////////////////////
//low-level pointer functions
#include "hwmou.hpp"
#endif

