#ifndef __MTAPPMWT_H_INCLUDED
#define __MTAPPMWT_H_INCLUDED

class BIFCLASS FMTApplication : public FApplication {
public:
        FMTApplication()
          : FApplication()
          {}
        //win32 doesn't need this class
        //It is only here for compability reasons
};


class BIFCLASS FWMainWindowThread : public FMainWindowThread {
        FMTApplication *(*pfn)();
        friend FMainThread *MakeMainThread(int,char **);
public:
        FWMainWindowThread(int argc, char *argv[])
          : FMainWindowThread(argc,argv)
          {}

        int Main(int argc, char *argv[]); 
};


#define DEFBIFMIXMAIN(appClass,mainThreadClass)                 \
FMTApplication *MakeApplicationInstance() {                     \
        return new appClass;                                    \
}                                                               \
FMainThread *MakeMainThread(int argc, char **argv) {            \
        FWMainWindowThread *t=new mainThreadClass(argc,argv);   \
        if(t)                                                   \
                t->pfn=MakeApplicationInstance;                 \
        return t;                                               \
}                                                               \
int PASCAL WinMain(HINSTANCE hinst, HINSTANCE hinstprev, LPSTR lpCmdLine, int nCmdShow) { \
        return _BIFThreadMain(hinst,hinstprev,lpCmdLine,nCmdShow,MakeMainThread); \
}

#endif

