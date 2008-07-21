#ifndef __DBGLOG_H
#define __DBGLOG_H
/* logging things
 * define...           to...
 * DEBUG_LOG_ALL       enable all logging
 * DEBUG_LOG_TRACE     enable tracing
 * DEBUG_LOG_NOTE      enable notes
 * DEBUG_LOG_WARN      enable warnings
 * DEBUG_LOG_ERROR     enable errors
 * DEBUG_LOG_FATAL     enable fatals (ignored - always enabled)
 */
/* Trace/note levels
 * Define...               to enable tracing/logging in...
 * DEBUG_TRACE_WHERE=0     kernel, api and application
 * DEBUG_TRACE_WHERE=1     api and application
 * DEBUG_TRACE_WHERE=2     application (default)
 * DEBUG_NOTE_WHERE=0      kernel, api and application
 * DEBUG_NOTE_WHERE=1      api and application (default)
 * DEBUG_NOTE_WHERE=2      application
 */

//tell log.h to include text and printf-format logging
#define DBG_LOG_TEXT
#define DBG_LOG_FORMAT
#include "log.h"

#ifdef DEBUG_LOG_ALL
//define all
#define DEBUG_LOG_TRACE
#define DEBUG_LOG_NOTE
#define DEBUG_LOG_WARN
#define DEBUG_LOG_ERROR
#define DEBUG_LOG_FATAL
#endif

#ifndef DEBUG_LOG_TRACE
//no tracing => phony trace level 3
#define DBG_TRACE_WHERE 3
#endif

#ifndef DEBUG_LOG_NOTE
//no notes => phony note level 3
#define DEBUG_NOTE_WHERE 3
#endif

#ifndef DEBUG_TRACE_WHERE
//default trace level = 2
#define DEBUG_TRACE_WHERE  2
#endif
#ifndef DEBUG_NOTE_WHERE
//default note level = 1
#define DEBUG_NOTE_WHERE  1
#endif


//tracing--------------------------------------------------------------------
class TraceMethod {
        const void *t;
        const char *mn;
public:
        TraceMethod(const void *that, const char *methodname)
        {
                t=that;
                mn=methodname;
                LOG_FORMAT("Method entered     --> %s (obj=%p)",mn,t);
        }
        ~TraceMethod() {
                LOG_FORMAT("Method exited     <--  %s (obj=%p)",mn,t);
        }
};


#define TRACE_METHOD(methodname) \
        TraceMethod trace_method_(this,methodname)



class TraceProcedure {
        const char *pn;
public:
        TraceProcedure(const char *procedurename)
        {
                pn=procedurename;
                LOG_FORMAT("Procedure entered  --> %s",pn);
        }
        ~TraceProcedure() {
                LOG_FORMAT("Procedure exited  <--  %s",pn);
        }
};



#define TRACE_PROC(procedurename) \
        TraceProcedure trace_procedure_(procedurename)



#if DEBUG_TRACE_WHERE<1
#define TRACE_METHOD0(methodname)  TRACE_METHOD(methodname)
#define TRACE_PROC0(procedurename) TRACE_PROC(procedurename)
extern void TRACE_FORMAT0(const char *fmt, ...);
#define TRACE_TEXT0(txt) TRACE_FORMAT0("%s",txt)
#else
#define TRACE_METHOD0(methodname)
#define TRACE_PROC0(procedurename)
static inline void TRACE_FORMAT0(const char *, ...) {}
#define TRACE_TEXT0(txt)
#endif

#if DEBUG_TRACE_WHERE<2
#define TRACE_METHOD1(methodname)  TRACE_METHOD(methodname)
#define TRACE_PROC1(procedurename) TRACE_PROC(procedurename)
extern void TRACE_FORMAT1(const char *fmt, ...);
#define TRACE_TEXT1(txt) TRACE_FORMAT1("%s",txt)
#else
#define TRACE_METHOD1(methodname)
#define TRACE_PROC1(procedurename)
static inline void TRACE_FORMAT1(const char *, ...) {}
#define TRACE_TEXT1(txt)
#endif

#if DEBUG_TRACE_WHERE<3
#define TRACE_METHOD2(methodname)  TRACE_METHOD(methodname)
#define TRACE_PROC2(procedurename) TRACE_PROC(procedurename)
extern void TRACE_FORMAT2(const char *fmt, ...);
#define TRACE_TEXT2(txt) TRACE_FORMAT2("%s",txt)
#else
#define TRACE_METHOD2(methodname)
#define TRACE_PROC2(procedurename)
static inline void TRACE_FORMAT2(const char *, ...) {}
#define TRACE_TEXT2(txt)
#endif



//note------------------------------------------------------------------------
#if DEBUG_NOTE_WHERE<1
extern void NOTE0(const char *fmt, ...);
#define NOTE_TEXT0(txt) NOTE0("%s",txt)
#else
static inline void NOTE0(const char *, ...) {}
#define NOTE_TEXT0(txt)
#endif

#if DEBUG_NOTE_WHERE<2
extern void NOTE1(const char *fmt, ...);
#define NOTE_TEXT1(txt) NOTE1("%s",txt)
#else
static inline void NOTE1(const char *, ...) {}
#define NOTE_TEXT1(txt)
#endif

#if DEBUG_NOTE_WHERE<3
extern void NOTE2(const char *fmt, ...);
#define NOTE_TEXT2(txt) NOTE2("%s",txt)
#else
static inline void NOTE_FORMAT2(const char *, ...) {}
#define NOTE_TEXT2(txt)
#endif




//warning-------------------------------------------------------------------
#ifdef DEBUG_LOG_WARN
extern void WARN(const char *fmt, ...);
#define WARN_TEXT(txt) WARN("%s",txt)
#else
static inline void WARN(const char *, ...) {}
#define WARN_TEXT(txt)
#endif



//error---------------------------------------------------------------------
#ifdef ERROR
//win16 defines ERROR
#undef ERROR
#endif
#ifdef DEBUG_LOG_ERROR
extern void ERROR(const char *fmt, ...);
#define ERROR_TEXT(txt) ERROR("%s",txt)
#else
static inline void ERROR(const char *, ...) {}
#define ERROR_TEXT(txt)
#endif



//fatal---------------------------------------------------------------------
extern void FATAL(const char *fmt, ...);



//assert/verify-------------------------------------------------------------
//assert goes away when compiling without errors enabled or when NDEBUG is
//defined. When NDEBUG is defined even the expression evaluation goes away.
#ifndef NDEBUG
#define ASSERT(expr) \
        if(!(expr)) ERROR("Assertion failure, expr = '%s', line %d in %s",#expr,__LINE__,__FILE__);
#define ASSERT_TEXT(expr,comment) \
        if(!(expr)) ERROR("Assertion failure, expr = '%s', line %d in %s, comment=%s",#expr,__LINE__,__FILE__,comment);
#else
#define ASSERT(expr)
#define ASSERT_TEXT(expr,comment)
#endif

//verify does not go away
#define VERIFY(expr)  \
        if(!(expr)) FATAL("Verify failure, expr = '%s', line %d in %s",#expr,__LINE__,__FILE__);
#define VERIFY_TEXT(expr,comment)  \
        if(!(expr)) FATAL("Verify failure, expr = '%s', line %d in %s, comment=%s",#expr,__LINE__,__FILE__,comment);


#endif /*__DBGLOG_H*/
