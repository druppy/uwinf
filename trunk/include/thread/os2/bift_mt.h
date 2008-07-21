#ifndef __BIFT_MT_H_INCLUDED
#define __BIFT_MT_H_INCLUDED

class BIFCLASS FMainThread : public virtual FThread {
        int argc;
        char **argv;
        int retcode;
        friend BIFFUNCTION _BIFThreadMain(int argc, char *argv[], FMainThread *(*pfn)(int,char**));
public:
        FMainThread(int argc, char *argv[]);
        ~FMainThread();

        virtual void Go();
        virtual int Main(int argc, char *argv[])=0;
};

#define DEFBIFTHREADMAIN(mainThreadClassName)           \
FMainThread *MakeMainThread(int argc, char **argv) {    \
        return new mainThreadClassName(argc,argv);      \
}                                                       \
                                                        \
int main(int argc, char **argv) {                       \
        return _BIFThreadMain(argc,argv,MakeMainThread);\
}

#endif
