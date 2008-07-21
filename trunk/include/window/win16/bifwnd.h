//Win16 fan-out includefile for BIF/Window

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
#endif

//dependencies
#if defined(BIFINCL_APPWND) || defined(BIFINCL_APPWINDOW)
#  define BIFINCL_FRAME
#  define BIFINCL_COMMONHANDLERS
#endif
#ifdef BIFINCL_DIALOG
#  define BIFINCL_FRAME
#  define BIFINCL_COMMONHANDLERS
#endif
#ifdef BIFINCL_APPLICATION
#  define BIFINCL_MODULE
#endif

//include core&misc unconditionally:
//wndcore uses FRect's, so wndmisc.h must be included first
#if !defined(__WNDMISC_H_INCLUDED)
#  include <window\winc\wndmisc.h>
#endif
#if !defined(__WNDCORE_H_INCLUDED)
#  include <window\win16\wndcore.h>
#endif


//fan-out include (with optimization):

#if defined(BIFINCL_COMMONHANDLERS) && !defined(__COMHNDL_H_INCLUDED)
#  include <window\winc\comhndl.h>
#endif

#if defined(BIFINCL_FRAME) && !defined(__FRAME_H_INCLUDED)
#  include <window\winc\frame.h>
#endif
#if (defined(BIFINCL_APPWND) || defined(BIFINCL_APPWINDOW)) && !defined(__APPWND_H_INCLUDED)
#  include <window\winc\appwnd.h>
#endif
#ifdef BIFINCL_DIALOG
// dialog.h fans out, can't optimze here
#  include <window\winc\dialog.h>
#endif

#ifdef BIFINCL_FRAMEHANDLERS
#  include <window\winc\frameh.h>
#endif
#ifdef BIFINCL_CLIENTHANDLERS
#  include <window\winc\clhndl.h>
#endif
#if defined(BIFINCL_MOUSEHANDLERS) && !defined(__MOUSE_H_INCLUDED)
#  include <window\winc\mouse.h>
#endif
#if defined(BIFINCL_KEYBOARDHANDLERS) && !defined(__KEYBOARD_H_INCLUDED)
#  include <window\winc\keyboard.h>
#endif
#ifdef BIFINCL_ADVANCEDHANDLERS
#  include <window\winc\adhndl.h>
#endif

//#ifdef BIFINCL_CONTROLS
// control.h fans out, can't optimze here
#  include <window\winc\control.h>
//#endif

#if (defined(BIFINCL_MODULE) || defined(BIFINCL_APPLICATION))  && !defined(__APPLICAT_H_INCLUDED)
#  include <window\win16\applicat.h>
#endif

#if defined(BIFINCL_MESSAGEBOX) && !defined(__MSGBOX_H_INCLUDED)
#  include <window\winc\msgbox.h>
#endif

