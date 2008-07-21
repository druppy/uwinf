#ifndef __APPLICAT_H_INCLUDED
#define __APPLICAT_H_INCLUDED

class FRect;    // Defined in WndMisc.h
class FEvent;   // Defined in WndCore.h

class BIFCLASS FApplication : public FModule {
public:
        FApplication();
        virtual ~FApplication();

        //messages:
        virtual void MessageLoop();
        virtual Bool GetEvent( FEvent &event );

        //termination
        virtual void TerminateApp();

        //application start and termination
        virtual int Main(int, char **);
        virtual Bool StartUp(int, char **) {return False;}
        virtual int ShutDown() {return 0;}


protected:
        FWndMan windowManager;
};

FApplication * BIFFUNCTION GetCurrentApp();


//startup stuff
extern int BIFFUNCTION _BIFWindowMain(int,char**,FApplication *(*pfn)());

#define DEFBIFWINDOWMAIN(applicationClass)                         \
FApplication *MakeApplicationInstance() {                          \
        return new applicationClass;                               \
}                                                                  \
                                                                   \
int main(int argc, char **argv) {                                  \
        return _BIFWindowMain(argc,argv,MakeApplicationInstance);  \
}

#endif //__APPLICAT_H_INCLUDED

