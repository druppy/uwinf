#ifndef __LOG_H
#define __LOG_H

#ifdef DBG_LOG_ALL
#  define DBG_LOG_TEXT
#  define DBG_LOG_FORMAT
#  define DBG_LOG_LONG
#  define DBG_LOG_PROC
#  define DBG_LOG_METHOD
#endif

#include <stdarg.h>

class Log {
        // a generic log
        Log *prev; //previous log instance
public:
        Log();
        virtual ~Log();

        // printf should have been virtual, but unfortunatly Watcom 9.5
        // rejects it
        void printf(const char *fmt, ...);
        void vprintf(const char *fmt, va_list ap);

        virtual void out(const char *t) =0;
        virtual void out(const char *t, long l);
        virtual void out(const char *t, unsigned long ul);

        virtual void out(const char *t, int i)
          { out(t,(long)i); }
        virtual void out(const char *t, unsigned int i)
          { out(t,(unsigned long)i); }
};

class FileLog : public Log {
        // a log that resides in a file
protected:
        const char *filename;        // name of logfile
        int IO;                        // did logging previously succeed
public:
        FileLog(const char *fn);
        ~FileLog();


        virtual void out(const char *t);
        virtual void out(const char *t, long l)
          { Log::out(t,l); }
        virtual void out(const char *t, unsigned long ul)
          { Log::out(t,ul); }

        virtual void out(const char *t, int i)
          { Log::out(t,i); }
        virtual void out(const char *t, unsigned int i)
          { Log::out(t,i); }
};

extern Log *LOGPtr;

#if defined(DBG_LOG_MSG) || defined(DBG_LOG_TEXT) || defined(DBG_LOG_LONG) || defined(DBG_LOG_PROC) || defined(DBG_LOG_METHOD) || defined(DBG_LOG_FORMAT) || defined(DBG_LOG_OS2ERROR)
#  define LOG_NEW(Class,arg) Class log_instance_(arg)
#else
#  define LOG_NEW(Class,arg) 
#endif



class LOGProcedure {
protected:
        const char *n;
public:
        LOGProcedure(const char *name);
        ~LOGProcedure();
};

class LOGMethod{
protected:
        const void *t;
        const char *n;
public:
        LOGMethod(const void *that, const char *name);
        ~LOGMethod();
};



#ifdef DBG_LOG_TEXT
#  define LOG_TEXT(t) if(LOGPtr) LOGPtr->out(t)
#else
#  define LOG_TEXT(t) ((void)0)
#endif


#ifdef DBG_LOG_FORMAT
  extern void LOG_FORMAT(const char *fmt, ...);
#else
  inline void LOG_FORMAT(const char *, ...) {}
#endif


#ifdef DBG_LOG_LONG
#  define LOG_LONG(t,l) if(LOGPtr) LOGPtr->out(t,l)
#else
#  define LOG_LONG(t,l) ((void)0)
#endif


#ifdef DBG_LOG_PROC
#  define LOG_PROC(name) LOGProcedure log_proc_(name)
#else
#  define LOG_PROC(name) ((void)0)
#endif


#ifdef DBG_LOG_METHOD
#  define LOG_METHOD(name) LOGMethod log_method_(this,name)
#else
#  define LOG_METHOD(name) ((void)0)
#endif

#endif /* __LOG_H */

