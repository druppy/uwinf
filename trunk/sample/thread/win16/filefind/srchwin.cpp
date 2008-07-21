#include "srchwin.hpp"
#include "filefind.h"
#include "filefind.idh"
#include "ffthread.h"
#include <string.h>
#include <stdio.h>

struct StartUpData {
        char drive;
        char fspec[13];
        HWND hWndNotify;        //window to notify when window is closed
};


struct SearchData {
        FileFindThread *fft;
        long filesInListbox;
        HWND hWndNotify;
};

//TypeCast heaven:
static SearchData far *getSearchData(HWND hWnd) {
        WORD lo=(WORD)GetProp(hWnd,"SD_LO");
        WORD hi=(WORD)GetProp(hWnd,"SD_HI");
        return (SearchData far*)MAKELONG(lo,hi);
}

static void setSearchData(HWND hWnd, SearchData far *sd) {
        SetProp(hWnd,"SD_LO",(HANDLE)LOWORD((DWORD)sd));
        SetProp(hWnd,"SD_HI",(HANDLE)HIWORD((DWORD)sd));
}

static void removeSearchData(HWND hWnd) {
        RemoveProp(hWnd,"SD_LO");
        RemoveProp(hWnd,"SD_HI");
}

static void updateListbox(HWND hWnd) {
        SearchData *sd=getSearchData(hWnd);
        HWND hWndListbox=GetDlgItem(hWnd,ID_FILELIST);

        if(sd->filesInListbox<sd->fft->files) {
                //turn off redrawing while adding (possible) many items
                SendMessage(hWndListbox,WM_SETREDRAW,FALSE,0);

                while(sd->filesInListbox<sd->fft->files) {
                        SendMessage(hWndListbox,LB_ADDSTRING,0,(LPARAM)(LPCSTR)sd->fft->fileNames[int(sd->filesInListbox)]);
                        sd->filesInListbox++;
                }

                SendMessage(hWndListbox,WM_SETREDRAW,TRUE,0);
                InvalidateRect(hWndListbox,NULL,TRUE);

                //scroll to the end
                SendMessage(hWndListbox,LB_SETCURSEL,(WPARAM)sd->filesInListbox-1,0);
        }
}


static void CloseSearchWindow(HWND hWnd) {
        SearchData *sd=getSearchData(hWnd);
        if(sd) {
                delete sd->fft;
                SendMessage(sd->hWndNotify,WM_SEARCHCLOSED,(WPARAM)hWnd,0);
                delete sd;
        }
        DestroyWindow(hWnd);
}


BOOL CALLBACK _export SearchDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch(msg) {
                case WM_INITDIALOG: {
                        StartUpData far * sud = (StartUpData far *)lParam;
                        char title[80];
                        wsprintf(title,"Search - %c:%s",sud->drive,sud->fspec);
                        SetWindowText(hWnd,title);
                        SetDlgItemText(hWnd,ID_STATUS,"Searching");

                        SearchData *sd=new SearchData;
                        if(!sd) return TRUE;
                        setSearchData(hWnd,sd);
                        sd->fft=new FileFindThread(hWnd,sud->fspec,sud->drive);
                        sd->filesInListbox=0;
                        sd->hWndNotify = sud->hWndNotify;

                        if(sd->fft->Start()) {
                                MessageBox(GetParent(hWnd),"Could not start thread",NULL,MB_OK);
                        }

                        return TRUE;
                }

                case WM_COMMAND: {
                        switch(wParam) {
                                case ID_CLOSE:
                                        CloseSearchWindow(hWnd);
                                        return TRUE;
                        }
                        return FALSE;
                }

                case WM_FILESFOUND: {
                        SearchData *sd=getSearchData(hWnd);
                        FileFindThread *fft=sd->fft;

                        char buf[10];

                        wsprintf(buf,"%ld",fft->files);
                        SetDlgItemText(hWnd,ID_FILES,buf);

                        wsprintf(buf,"%ld",fft->dirs);
                        SetDlgItemText(hWnd,ID_DIRS,buf);

                        if(fft->files>sd->filesInListbox+50) updateListbox(hWnd);

                        return TRUE;
                }

                case WM_SEARCHFINISHED: {
                        SetDlgItemText(hWnd,ID_STATUS,"Finished");
                        updateListbox(hWnd);
                        return TRUE;
                }

                case WM_CLOSE: {
                        CloseSearchWindow(hWnd);
                        return TRUE;
                }
                case WM_DESTROY: {
                        removeSearchData(hWnd);
                        return FALSE;
                }
                default:
                        return FALSE;
        }
}


HWND CreateSearchWindow(HINSTANCE hInst, HWND hWndNotify, char drive, const char *fspec) {
        StartUpData sd;
        sd.drive=drive;
        strcpy(sd.fspec,fspec);
        sd.hWndNotify = hWndNotify;

        return CreateDialogParam(hInst,
                                 MAKEINTRESOURCE(DIALOG_SEARCH),
                                 NULL,
                                 (DLGPROC)SearchDlgProc,
                                 (LPARAM)(StartUpData far*)&sd
                                );

}

