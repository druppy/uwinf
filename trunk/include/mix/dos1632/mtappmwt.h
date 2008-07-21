#ifndef __MTAPPMWT_H_INCLUDED
#define __MTAPPMWT_H_INCLUDED

class BIFCLASS FMTApplication : public FApplication {
public:
        FMTApplication()
          : FApplication()
          {}
        //DOS doesn't need this class
        //It is only here for compability reasons
};


class BIFCLASS FWMainWindowThread : public FMainWindowThread {
        FMTApplication *(*pfn)();
        friend FMainThread *MakeMainThread(int,char **);
public:
        FWMainWindowThread(int argc, char **argv, int queueSize=0)
          : FMainWindowThread(argc,argv,queueSize)
          { app=0; }

        int Main(int argc, char **argv); 
protected:
        int PreGo();
        int _PreGo();
        void PostGo();
        void _PostGo();
private:
        FApplication *app;
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
int main(int argc, char **argv) {                               \
        return _BIFThreadMain(argc,argv,MakeMainThread);        \
}

#endif

