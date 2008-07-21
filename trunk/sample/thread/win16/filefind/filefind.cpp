#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#define BIFINCL_WINDOWTHREAD
#include <bif.h>

#include "filefind.idh"
#include "filefind.h"
#include "srchwin.hpp"

//Declare a main thread class
class BIFCLASS MyMainThread : public FMainWindowThread {
        void registerWndClass(HINSTANCE hInst);
public:
        MyMainThread(int argc, char *argv[])
          : FMainWindowThread(argc,argv)
          {}

        virtual int Main(int argc, char *argv[]);
};


#define MAXSEARCH 10
HWND searchWindow[ MAXSEARCH ];
int searchWindows=0;


BOOL CALLBACK _export StartSearchDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM/* lParam*/) {
        switch(msg) {
                case WM_INITDIALOG:
                        SendDlgItemMessage(hWnd,ID_FILESPEC,EM_LIMITTEXT,12,0);
                        SendDlgItemMessage(hWnd,ID_DRIVE,EM_LIMITTEXT,2,0);
                        return TRUE;

                case WM_COMMAND:
                        switch(wParam) {
                                case ID_CLOSE:
                                        PostQuitMessage(0);
                                        return TRUE;
                                case ID_START: {
                                        if(searchWindows==MAXSEARCH) {
                                                BTMessageBox(hWnd,"App-defined limit reached","FileFind",MB_OK|MB_ICONINFORMATION);
                                        } else {
                                                char drive[2];
                                                char fspec[13];
                                                GetDlgItemText(hWnd,ID_DRIVE,drive,2);
                                                GetDlgItemText(hWnd,ID_FILESPEC,fspec,12);

                                                HWND hWndSearch=CreateSearchWindow(BIFTGet_hInstance(),hWnd,drive[0],fspec);
                                                if(hWndSearch) {
                                                        searchWindow[searchWindows++]=hWndSearch;
                                                        ShowWindow(hWndSearch,SW_SHOW);
                                                } else
                                                        BTMessageBox(hWnd,"Could not create search window",NULL,MB_OK);
                                        }
                                        return TRUE;
                                }
                        }
                        return FALSE;

                case WM_SEARCHCLOSED: {
                        //remove window from list
                        HWND hWndSearch=(HWND)wParam;
                        for(int i=0; searchWindow[i]!=hWndSearch; i++);
                        for(;i<searchWindows-1; i++)
                                searchWindow[i]=searchWindow[i+1];
                        searchWindows--;
                        return TRUE;
                }

                case WM_DESTROY: {
                        //destroy all search windows
                        for(int i=searchWindows-1; i>=0; i--)
                                DestroyWindow(searchWindow[i]);
                        return TRUE;
                }

                default:
                        return FALSE;
        }
}


int MyMainThread::Main(int,char**) {
        HWND hWnd;
        hWnd=CreateDialog(BIFTGet_hInstance(), MAKEINTRESOURCE(DIALOG_STARTSEARCH), NULL, (DLGPROC)StartSearchDlgProc);
        if(hWnd) {
                ShowWindow(hWnd,BIFTGet_nCmdShow());

                MSG msg;
                int i;
                while(GetMessage(&msg)) {
                        if(IsDialogMessage(hWnd,&msg)) goto wasdlg ;
                        for(i=0; i<searchWindows; i++)
                                if(IsDialogMessage(searchWindow[i],&msg)) goto wasdlg;
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                wasdlg: ;
                }

                DestroyWindow(hWnd);
        }

        return 0;
}





//Use default BIF/Thread startup

DEFBIFTHREADMAIN(MyMainThread);


