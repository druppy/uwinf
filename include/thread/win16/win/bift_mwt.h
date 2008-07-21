#ifndef __BIFT_MWT_H_INCLUDED
#define __BIFT_MWT_H_INCLUDED

class BIFCLASS FMainWindowThread : public virtual FThread,
                                   public FMainThread,
                                   public FWindowThread {
        int stackOk;
public:
        FMainWindowThread(int argc, char *argv[]);
        ~FMainWindowThread();
        virtual void Go() { FMainThread::Go(); }
        virtual int OkToGo() { return stackOk; }

        static FMainWindowThread *GetMWT();
#if defined(__BORLANDC__) && __BORLANDC__<=0x410
        //BC31 has a bug in its vtable generation
        int Main(int,char**) { return 42; }
#endif
};

int BIFFUNCTION BTMessageBox(HWND hwndParent, LPCSTR lpszText, LPCSTR lpszTitle, UINT fuStyle);

#endif
