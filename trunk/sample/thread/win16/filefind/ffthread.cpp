#include <dos.h>
#include <string.h>

#include "ffthread.h"
#include "filefind.h"


#define MAXFILES        5000

static char *strnewdup(const char *s) {
        char *p=new char[strlen(s)+1];
        if(p)
                strcpy(p,s);
        return p;
}

int FileFindThread::addFile(const char *path, const char *name) {
        char *p;
        if(files<MAXFILES-1) {
                char buf[128];
                wsprintf(buf,"%s\\%s",(LPSTR)path,(LPSTR)name);
                p=strnewdup(buf);
                if(p) {
                        fileNames[int(files++)]=p;
                        return 0;
                } else
                        return -1;
        } else {
                p=strnewdup("*File limit reached*");
                if(p)
			fileNames[int(files++)]=p;
                return -1;
        }
}


FileFindThread::FileFindThread(HWND hWndOwner, const char *fspec, const char drive)
  : FWindowThread(16384)
{
        hWnd=hWndOwner;
        strcpy(filespec,fspec);
        FileFindThread::drive=drive;
        files=0;
        dirs=0;
        fileNames=new char*[MAXFILES];
        stopSearch=0;
}


FileFindThread::~FileFindThread() {
        for(int i=0; i<files; i++)
                delete[] fileNames[i];
        delete[] fileNames;
}



int FileFindThread::OkToGo() {
        if(fileNames)
                return FWindowThread::OkToGo();
        else
                return 0;
}


void FileFindThread::Go() {
        char d[3];
        d[0]=drive;
        d[1]=':';
        d[2]='\0';
        find(d);
        PostMessage(hWnd,WM_SEARCHFINISHED,0,0);
}

void FileFindThread::find(const char *dir) {
        //find files in directory and below
        char fspec[128];
        struct find_t ff;
        int done;

        //find the files matching the filespec
        strcpy(fspec,dir);
        strcat(fspec,"\\");
        strcat(fspec,filespec);
        done=_dos_findfirst(fspec,_A_ARCH,&ff);
        while((!done) && (!stopSearch)) {
                if(addFile(dir,ff.name))
                        stopSearch=1;
		done=_dos_findnext(&ff);
	}
#ifdef __WATCOMC__
	_dos_findclose(&ff);
#endif
	dirs++;
	PostMessage(hWnd,WM_FILESFOUND,0,0);    //update display

	Yield();

	//find subdirs
	strcpy(fspec,dir);
	strcat(fspec,"\\*.*");
	done=_dos_findfirst(fspec,_A_SUBDIR,&ff);
	while((!done) && (!stopSearch)) {
		if((ff.attrib&_A_SUBDIR)!=0 && ff.name[0]!='.') {
			char subdir[128];
			strcpy(subdir,dir);
			strcat(subdir,"\\");
			strcat(subdir,ff.name);
			find(subdir);
		}
		done=_dos_findnext(&ff);
	}
#ifdef __WATCOMC__
	_dos_findclose(&ff);
#endif
	PostMessage(hWnd,WM_FILESFOUND,0,0);    //update display
        
        Yield();
}

