/* BIF/Window fan-out header file
 * WMS interface
 */

#ifdef BIFINCL_WINALL
// The programmer is lazy...
#  define BIFINCL_FRAME
#  define BIFINCL_APPWND
#  define BIFINCL_APPWINDOW
#  define BIFINCL_DIALOG
#  define BIFINCL_COMMONHANDLERS
#  define BIFINCL_FRAMEHANDLERS
#  define BIFINCL_CLIENTHANDLERS
#  define BIFINCL_MOUSEHANDLERS
#  define BIFINCL_KEYBOARDHANDLERS
#  define BIFINCL_ADVANCEDHANDLERS
#  define BIFINCL_CONTROLS
#  define BIFINCL_ALLCONTROLS
#  define BIFINCL_MODULE
#  define BIFINCL_APPLICATION
#  define BIFINCL_MESSAGEBOX
#  define BIFINCL_RESOURCETEMPLATE
#  define BIFINCL_DYNMSG
#  define BIFINCL_TRACKRECT
#endif

//dependencies
#if defined(BIFINCL_APPWND) || defined(BIFINCL_APPWINDOW)
#  define BIFINCL_FRAME
#  define BIFINCL_COMMONHANDLERS
#endif
#ifdef BIFINCL_DIALOG
#  define BIFINCL_FRAME
#  define BIFINCL_COMMONHANDLERS
#  define BIFINCL_RESOURCETEMPLATE
#endif
#ifdef BIFINCL_APPLICATION
#  define BIFINCL_MODULE
#endif

//include core&misc unconditionally:
//wndcore and wndmisc rely on wms beeing included ahead of them
#include <wms.hpp>
#if !defined(__BW_MISC_H_INCLUDED)
#  include <window\wms\wndmisc.h>
#endif
#if !defined(__BW_CORE_H_INCLUDED)
#  include <window\wms\wndcore.h>
#endif


//fan-out include (with optimization):

#if defined(BIFINCL_RESOURCETEMPLATE) && !defined(__RESTEMPL_H_INCLUDED)
#  include <window\wms\restempl.h>
#endif

#if defined(BIFINCL_COMMONHANDLERS) && !defined(__COMHNDL_H_INCLUDED)
#  include <window\wms\comhndl.h>
#endif

#if defined(BIFINCL_FRAME) && !defined(__FRAME_H_INCLUDED)
#  include <window\wms\frame.h>
#endif
#if (defined(BIFINCL_APPWND) || defined(BIFINCL_APPWINDOW)) && !defined(__APPWND_H_INCLUDED)
#  include <window\wms\appwnd.h>
#endif
#ifdef BIFINCL_DIALOG
// dialog.h fans out, can't optimze here
#  include <window\wms\dialog.h>
#endif

#ifdef BIFINCL_FRAMEHANDLERS
#  include <window\wms\frameh.h>
#endif
#ifdef BIFINCL_CLIENTHANDLERS
#  include <window\wms\clhndl.h>
#endif
#if defined(BIFINCL_MOUSEHANDLERS) && !defined(__MOUSE_H_INCLUDED)
#  include <window\wms\mouse.h>
#endif
#if defined(BIFINCL_KEYBOARDHANDLERS) && !defined(__KEYBOARD_H_INCLUDED)
#  include <window\wms\keyboard.h>
#endif
#ifdef BIFINCL_ADVANCEDHANDLERS
// adhndl.h fans out, can't optimize
#  include <window\wms\adhndl.h>
#endif

#ifdef BIFINCL_CONTROLS
// control.h fans out, can't optimze here
#  include <window\wms\control.h>
#endif

#if defined(BIFINCL_MODULE) && !defined(__MODULE_H_INCLUDED)
#  include <window\wms\module.h>
#endif

#if defined(BIFINCL_APPLICATION) && !defined(__APPLICAT_H_INCLUDED)
#  include <window\wms\applicat.h>
#endif

#if defined(BIFINCL_MESSAGEBOX) && !defined(__MSGBOX_H_INCLUDED)
#  include <window\wms\msgbox.h>
#endif

#if defined(BIFINCL_DYNMSG) && !defined(__DYNMSG_H_INCLUDED)
#  include <window\wms\dynmsg.h>
#endif

#if defined(BIFINCL_TRACKRECT) && !defined(__TRACKRCT_H_INCLUDED)
#  include <window\wms\trackrct.h>
#endif
