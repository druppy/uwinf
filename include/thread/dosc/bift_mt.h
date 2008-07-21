#ifndef __BIFT_MT_H_INCLUDED
#define __BIFT_MT_H_INCLUDED

class BIFCLASS FMainThread : public virtual FThread {
        int argc;
        char **argv;
        int retcode;
#if BIFOS_ == BIFOS_WIN16_
        friend BIFFUNCTION _BIFThreadMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow, FMainThread *(*pfn)(int,char**));
#else
        friend _BIFThreadMain(int,char**, FMainThread *(*pfn)(int,char**));
#endif
public:
        FMainThread(int argc, char *argv[]);
        ~FMainThread();

        virtual void Go();
        virtual int Main(int argc, char *argv[])=0;
};


#if BIFOS_ == BIFOS_WIN16_

#define DEFBIFTHREADMAIN(mainThreadClassName)                                              \
FMainThread *MakeMainThread(int argc, char *argv[]) {                                      \
        return new mainThreadClassName(argc,argv);                                         \
}                                                                                          \
                                                                                           \
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)  \
{                                                                                          \
        return _BIFThreadMain(hInst,hPrevInst,lpszCmdLine,nCmdShow,MakeMainThread);        \
}

#else /*_Windows*/

#define DEFBIFTHREADMAIN(mainThreadClassName)                                   \
FMainThread *MakeMainThread(int argc, char *argv[]) {                           \
        return new mainThreadClassName(argc,argv);                              \
}                                                                               \
                                                                                \
int main(int argc, char **argv) {                                               \
        return _BIFThreadMain(argc,argv,MakeMainThread);                        \
}
#endif /*_Windows*/

#endif
