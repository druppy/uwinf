#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>

int FWMainWindowThread::Main(int,char**) {
        return _BIFWindowMain(BIFTGet_hInstance(),
                              BIFTGet_hPrevInstance(),
                              BIFTGet_lpszCmdLine(),
                              BIFTGet_nCmdShow(),
                              pfn_makeappinst);
}

