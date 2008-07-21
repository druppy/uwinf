#ifdef DEBUG_LOG_ALL
#undef DEBUG_LOG_ALL
#endif
#define DEBUG_LOG_ALL

#ifdef DEBUG_TRACE_WHERE
#undef DEBUG_TRACE_WHERE
#endif
#define DEBUG_TRACE_WHERE 0

#ifdef DEBUG_NOTE_WHERE
#undef DEBUG_NOTE_WHERE
#endif
#define DEBUG_NOTE_WHERE 0

#include "DbgLog.h"
#include <stdlib.h>
#include <stdarg.h>

#if defined(_Windows) || defined(__WINDOWS__) || defined(_WINDOWS) || defined(__NT__) || defined(_WIN32)
#  ifndef __WINDOWS_H
#    include <windows.h>
#  endif
#  if !(defined(__NT__) || defined(_WIN32))
#    include <toolhelp.h>
#  endif
#endif

//trace --------------------------------------------------------------------
void TRACE_FORMAT0(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
}
void TRACE_FORMAT1(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
}
void TRACE_FORMAT2(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
}

//note ---------------------------------------------------------------------
void NOTE0(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
}
void NOTE1(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
}
void NOTE2(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
}


//warning--------------------------------------------------------------------
void WARN(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
}


//error----------------------------------------------------------------------
#if defined(_Windows) || defined(__WINDOWS__) || defined(_WINDOWS) || defined(__WINNT__) || defined (__NT__) || defined(_WIN32)
#if defined(__WIN32__) || defined(__WINNT__) || defined(__FLAT__) || defined (__NT__) || defined(M_I386) || defined(_WIN32)
static void abortProgram(const char *type) {
        MessageBox(NULL,"An error occurred in the application",type,MB_TASKMODAL|MB_ICONSTOP|MB_OK);
        //now commit suicide
        TerminateProcess(GetCurrentProcess(),255);
}
#else
static void abortProgram(const char *type) {
        MessageBox(NULL,"An error occurred in the application",type,MB_TASKMODAL|MB_ICONSTOP|MB_OK);

        //Watcom C/C++ 10.0 does not have toolhelp functions in its default import
        //libraries, but in a seperate toolhelp.lib. To avoid to force the user to
        //add toolhelp.lib to the default libraries, toolhelp.dll is loaded dynamically
        HINSTANCE hinstToolhelp = LoadLibrary("toolhelp.dll");
        if(hinstToolhelp>HINSTANCE_ERROR) {
                FARPROC fpTerminateApp=GetProcAddress(hinstToolhelp,"TERMINATEAPP");
                if(fpTerminateApp) {
                        typedef void (WINAPI*TerminateApp_t)(HTASK hTask, WORD wFlags);
                        TerminateApp_t ta=(TerminateApp_t)fpTerminateApp;
                        (*ta)(NULL,NO_UAE_BOX);
                }
        } 
        //toolhelp.dll not found - use abort() instead
        abort();
}
#endif
#elif defined(__OS2__)
static void abortProgram(const char *) {
        exit(1);
}
#elif defined(__MSDOS__) || defined(__DOS__)
static void abortProgram(const char *) {
        exit(1);
}
#endif

#ifdef ERROR
//Windows uses ERROR for regions
#undef ERROR
#endif
void ERROR(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
        abortProgram("error");
}



//fatal----------------------------------------------------------------------
void FATAL(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
        abortProgram("fatal");
}

