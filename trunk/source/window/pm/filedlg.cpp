/***
Filename: csi.cpp
Description:
  Implementation of standard file dialogs
Host:
  Watcom 10.0a-11.0, VAC 3.0
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 97-06-28 Removed <mem.h> reference - memset() is in string.h
***/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_DIALOG
#define BIFINCL_STDFILEDIALOG
#include <bif.h>
#include <dbglog.h>

#include <string.h>

MRESULT EXPENTRY stdFileInitDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
        TRACE_FORMAT0("StdFileHack::stdFileHookProc");
        FWnd *pwnd=GetWndMan()->GetCreationWindow(0);
        if(pwnd) {
                WinSetWindowPtr(hwnd, QWP_PFNWP, (PVOID)(PFNWP)WinDefFileDlgProc);
                GetWndMan()->SubRegWnd(pwnd,hwnd);
                FEvent event(hwnd,msg,mp1,mp2);
                if(pwnd->DispatchNoDefault(event))
                        return event.GetResult();
                else
                        return WinDefFileDlgProc(hwnd,msg,mp1,mp2);
        } else {
                //should not happen
                return WinDefFileDlgProc(hwnd,msg,mp1,mp2);
        }
}


Bool FStdFileDialog::DoModal(FWnd *pOwner, sfd_type type,
                             char *filename,
                             const char *filterMask,
                             const char *dlgTitle, const char *okButtonText
                            )
{
        TRACE_METHOD1("FStdFileDialog::DoModal");

        memset(&fild,0,sizeof(fild));
        fild.cbSize=sizeof(fild);

        if(type==sfd_open)
                fild.fl|=FDS_OPEN_DIALOG;
        else
                fild.fl|=FDS_SAVEAS_DIALOG;
        fild.fl|=FDS_CENTER;

        //The documentation is incorrect, the default directory is not the root
        //when this bit is left out
        //fild.fl|=FDS_PRELOAD_VOLINFO;      
                                             

        fild.ulUser=(ULONG)this;

        fild.pszTitle=(PSZ)dlgTitle;         //PSZ but readonly anyway

        fild.pszOKButton=(PSZ)okButtonText;  //ditto

        fild.pfnDlgProc=stdFileInitDlgProc;

        fild.pszIType=NULL;
        fild.papszITypeList=NULL;

        fild.pszIDrive=NULL;            //default drive
        fild.papszIDriveList=NULL;

        fild.hMod=NULLHANDLE;           //no custom template

        if(filterMask)
                strcpy(fild.szFullFile,filterMask);
        else if(filename) {
                //make 'filename' a fully-qualified filename
                DosQueryPathInfo(filename, FIL_QUERYFULLNAME, fild.szFullFile, CCHMAXPATH);
        } else
                fild.szFullFile[0]='\0';

        GetWndMan()->SetCreationWindow(this,0);
        HWND hwndDlg=WinFileDlg(HWND_DESKTOP, pOwner?pOwner->GetHwnd():NULLHANDLE, &fild);
        AfterDestroy();         //just to be sure...
        
        if(hwndDlg==NULLHANDLE)
                return False;   //error
        if(fild.lReturn==DID_OK) {
                strcpy(filename,fild.szFullFile);
                return True;
        } else
                return False;

}


Bool FStdFileDialog::DoModal(FWnd *pOwner, sfd_type type,
                             char *filenameBuffer, int maxFilenameBuf,
                             char **filename, int maxFiles,
                             const char *filterMask,
                             const char *dlgTitle, const char *okButtonText
                            )
{
        TRACE_METHOD1("FStdFileDialog::DoModal");

        memset(&fild,0,sizeof(fild));
        fild.cbSize=sizeof(fild);

        fild.fl|=FDS_MULTIPLESEL;

        if(type==sfd_open)
                fild.fl|=FDS_OPEN_DIALOG;
        else
                fild.fl|=FDS_SAVEAS_DIALOG;
        fild.fl|=FDS_CENTER;

        //The documentation is incorrect, the default directory is not the root
        //when this bit is left out
        //fild.fl|=FDS_PRELOAD_VOLINFO;      
                                             

        fild.ulUser=(ULONG)this;

        fild.pszTitle=(PSZ)dlgTitle;         //PSZ but readonly anyway

        fild.pszOKButton=(PSZ)okButtonText;  //ditto

        fild.pfnDlgProc=stdFileInitDlgProc;

        fild.pszIType=NULL;
        fild.papszITypeList=NULL;

        fild.pszIDrive=NULL;            //default drive
        fild.papszIDriveList=NULL;

        fild.hMod=NULLHANDLE;           //no custom template

        if(filterMask)
                strcpy(fild.szFullFile,filterMask);
        else 
                fild.szFullFile[0]='\0';


        GetWndMan()->SetCreationWindow(this,0);
        HWND hwndDlg=WinFileDlg(HWND_DESKTOP, pOwner?pOwner->GetHwnd():NULLHANDLE, &fild);
        GetWndMan()->GetCreationWindow(0);
        AfterDestroy();         //just to be sure...
        

        if(hwndDlg==NULLHANDLE)
                return False;   //error
        if(fild.lReturn==DID_OK) {
                int d=0;
                int f=0;
                while(f<fild.ulFQFCount && f<maxFiles-1) {
                        int fnlen=strlen(fild.papszFQFilename[0][f]);
                        if(d+fnlen+1>maxFilenameBuf)
                                break;
                        filename[f]=filenameBuffer+d;
                        strcpy(filenameBuffer+d, fild.papszFQFilename[0][f]);
                        d+=fnlen+1;
                        f++;
                }
                filename[f]=0;

                WinFreeFileDlgList(fild.papszFQFilename);

                return True;
        } else
                return False;

}

