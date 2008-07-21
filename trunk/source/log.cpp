#define DBG_LOG_ALL
#ifdef __OS2__
#  define INCL_DOSPROCESS
#  define INCL_DOSFILEMGR
#  define INCL_DOSSEMAPHORES
#endif

#include "log.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#if defined(_Windows) || defined(__WINDOWS__) || defined(_WINDOWS) || defined(__NT__) || defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#elif defined(__MSDOS__) || defined(__DOS__)
#  include <io.h>
#  include <fcntl.h>
#  include <sys\stat.h>
#elif defined(__OS2__)
#  include <os2.h>
#endif


//***************************************************************************
// LOGProcedure / LOGMethod / LOG?????

LOGProcedure::LOGProcedure(const char *name) {
        n=name;
        if(LOGPtr) LOGPtr->printf("Procedure entered --> %s",name);
}

LOGProcedure::~LOGProcedure() {
        if(LOGPtr) LOGPtr->printf("Procedure exited <-- %s",n);
}


LOGMethod::LOGMethod(const void *that, const char *name) {
        t=that;
        n=name;
        if(LOGPtr) LOGPtr->printf("Method entered --> %s (obj=%08lx)",name,(unsigned long)that);
}

LOGMethod::~LOGMethod() {
        if(LOGPtr) LOGPtr->printf("Method exited <--  %s (obj=%08lx)",n,(unsigned long)t);
}



#ifdef DBG_LOG_FORMAT
void LOG_FORMAT(const char *fmt, ...) {
        if(LOGPtr) {
                va_list ap;
                va_start(ap,fmt);
                LOGPtr->vprintf(fmt,ap);
                va_end(ap);
        }
}
#endif


//***************************************************************************
// Log

#define PRINTFBUFSIZE 512
Log *LOGPtr=0;

Log::Log() {
        prev=LOGPtr;
        LOGPtr=this;
}

Log::~Log() {
        LOGPtr=prev;
}

void Log::printf(const char *fmt, ...) {
        va_list ap;
        va_start(ap,fmt);
        vprintf(fmt,ap);
        va_end(ap);
}

void Log::vprintf(const char *fmt, va_list ap) {
        char buf[PRINTFBUFSIZE];
        vsprintf(buf,fmt,ap);
        out(buf);
}

void Log::out(const char *t, long l) {
        char buf[PRINTFBUFSIZE];
        sprintf(buf,"%s %ld\n",t,l);
        out(buf);
}

void Log::out(const char *t, unsigned long ul) {
        char buf[PRINTFBUFSIZE];
        sprintf(buf,"%s %lu\n",t,ul);
        out(buf);
}



#if defined(__OS2__) || defined(OS2)
// We have to create a mutex protection on the log file, because OS/2 does not
// provide open-for-append mecanism

class LogLock;

class FileLogMutex {
        HMTX hmtx;
        int ok;
public:
        FileLogMutex();
        ~FileLogMutex();
private:
        void lock() {
                if(ok) DosRequestMutexSem(hmtx,SEM_INDEFINITE_WAIT);
        }
        void unlock() {
                if(ok) DosReleaseMutexSem(hmtx);
        }
        friend class LogLock;
};

FileLogMutex::FileLogMutex() {
        if(DosCreateMutexSem((PSZ)0, &hmtx, 0, FALSE))
                ok=0;
        else
                ok=1;
}

FileLogMutex::~FileLogMutex() {
        if(ok) DosCloseMutexSem(hmtx);
}


static FileLogMutex fileMutex;

class LogLock {
        int locked;
public:
        LogLock() { fileMutex.lock(); locked=1; }
        ~LogLock() { unlock(); }
        void unlock() { if(locked) fileMutex.unlock(); locked=0; }
};

static TID GetCurrentTID() {
        PPIB ppib;
        PTIB ptib;
        DosGetInfoBlocks(&ptib, &ppib);
        return ptib->tib_ptib2->tib2_ultid;
}
#elif defined(__NT__) || defined(__WINNT__) || defined(__WIN32__) || defined(_WIN32)
class LogLock;

class FileLogMutex {
        CRITICAL_SECTION cs;
public:
        FileLogMutex() { InitializeCriticalSection(&cs); }
        ~FileLogMutex() { DeleteCriticalSection(&cs); }
private:
        void lock() {
                EnterCriticalSection(&cs);
        }
        void unlock() {
                LeaveCriticalSection(&cs);
        }
        friend class LogLock;
};


static FileLogMutex fileMutex;

class LogLock {
public:
        LogLock() { fileMutex.lock();  }
        ~LogLock() { fileMutex.unlock(); }
};

#endif


FileLog::FileLog(const char *fn) {
        filename=fn;
        IO=1;
        out("Debug session started");
}

FileLog::~FileLog() {
        out("Debug session ended");
}


void FileLog::out(const char *t) {
        if(!IO) return;
#if defined(__NT__) || defined(__WINNT__) || defined(__WIN32__) || defined(_WIN32)
        LogLock lock;

        HANDLE hf=CreateFile(filename,
                             GENERIC_WRITE,
                             FILE_SHARE_READ,
                             0,
                             OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                             0);
        if(hf==INVALID_HANDLE_VALUE) {
                IO=0;
                return;
        }

        SetFilePointer(hf, 0, 0, FILE_END);

        DWORD bytesWritten;
        
        char tid_buf[10];
        sprintf(tid_buf,"%06lx ",GetCurrentThreadId());
        if(!WriteFile(hf, tid_buf, 7, &bytesWritten, 0)) {
                CloseHandle(hf);
                IO=0;
                return;
        }

        int textlen=strlen(t);
        if(!WriteFile(hf,t,textlen,&bytesWritten,0)) {
                //write fail
                CloseHandle(hf);
                IO=0;
                return;
        }
        //append CR-LF
        if(!WriteFile(hf,"\r\n",2,&bytesWritten,0)) {
                CloseHandle(hf);
                IO=0;
                return;
        }

        CloseHandle(hf);
        
#elif defined(__WINDOWS__) || defined(_Windows)
        HFILE hf=_lopen(filename, WRITE|OF_SHARE_DENY_WRITE);

        if(hf==HFILE_ERROR) {
                hf=_lcreat(filename, 0);
                if(hf==HFILE_ERROR) {
                        IO=0;
                        return;
                }
        }
        if(_llseek(hf,0,2)==HFILE_ERROR) {
                _lclose(hf);
                IO=0;
                return;
        }


        int textlen=strlen(t);
        if(_lwrite(hf,t,textlen)!=textlen) {
                //write fail
                _lclose(hf);
                IO=0;
                return;
        }
        //append CR-LF
        if(_lwrite(hf,"\r\n",2)!=2) {
                _lclose(hf);
                IO=0;
                return;
        }
        
        _lclose(hf);

#elif defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)

        int fd=open(filename, O_RDWR|O_APPEND|O_CREAT|O_BINARY, S_IWRITE|S_IREAD);
        if(fd<0) {
                IO=0;
                return;
        }
        int textlen=strlen(t);
        if(write(fd,t,textlen)!=textlen) {
                //write fail
                close(fd);
                IO=0;
                return;
        }
        //append CR-LF
        if(write(fd,"\r\n",2)!=2) {
                close(fd);
                IO=0;
                return;
        }
        
        close(fd);

#elif defined(__OS2__) || defined(OS2)

        LogLock lock;

        HFILE hf;
        APIRET rc;
        ULONG iDontCare;

        rc = DosOpen((PSZ)filename,
                     &hf,
                     &iDontCare,
                     0,
                     FILE_NORMAL,
                     OPEN_ACTION_CREATE_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
                     OPEN_FLAGS_SEQUENTIAL|OPEN_FLAGS_NOINHERIT|OPEN_SHARE_DENYWRITE|OPEN_ACCESS_WRITEONLY,
                     (PEAOP2)NULL
                    );
        if(rc!=0) {
                IO=0;
                return;
        }

        rc = DosSetFilePtr(hf, 0, FILE_END, &iDontCare);
        if(rc!=0) {
                DosClose(hf);
                IO=0;
                return;
        }

        char tid_buf[10];
        sprintf(tid_buf,"%06lx ",GetCurrentTID());
        rc = DosWrite(hf, (PVOID)tid_buf, 7, &iDontCare);
        if(rc!=0) {
                DosClose(hf);
                IO=0;
                return;
        }

        int textlen=strlen(t);
        rc = DosWrite(hf, (PVOID)t, textlen, &iDontCare);
        if(rc!=0) {
                DosClose(hf);
                IO=0;
                return;
        }

        rc = DosWrite(hf, (PVOID)"\r\n", 2, &iDontCare);
        if(rc!=0) {
                DosClose(hf);
                IO=0;
                return;
        }

        DosClose(hf);

#else
#error unknown OS
#endif
}
