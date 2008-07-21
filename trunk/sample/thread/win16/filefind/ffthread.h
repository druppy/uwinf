#ifndef __TESTFF_H
#define __TESTFF_H

#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#include <bif.h>

//Declare a file-find thread class - it must be a subclass of FWindowThread
//because it uses windowing functions
class BIFCLASS FileFindThread : public FWindowThread {
        HWND hWnd;
        char filespec[20];
        char drive;
        void Go();
        void find(const char *dir);

        int stopSearch;
        int addFile(const char *path, const char *name);
public:
        FileFindThread(HWND hWndOwner, const char *fspec, const char drive);
        ~FileFindThread();

        virtual int OkToGo();

        long files;     //number of files found
        long dirs;      //number of directories scanned
        char **fileNames;
};

#endif
