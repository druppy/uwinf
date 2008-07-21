/***
Filename: filedlg.cpp
Description:
  Implementation of standard file dialogs
Host:
  3.1: BC40, WC10, SC61
  NT: WC10.6
History:
  ISJ 94-11-?? Creation
  ISJ 95-03-01 Changed to use makeprocinstance on the "hook"
  ISJ 95-03-16 Made BC31 happy
  ISJ 96-08-04 Fixed a bug in multifile selection and made this module
               compilable under win32
***/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_DIALOG
#define BIFINCL_STDFILEDIALOG
#include <bif.h>
#include <dbglog.h>

#include <mem.h>
#include <string.h>

#include <commdlg.h>

#if defined(__BORLANDC__) && __BORLANDC__==0x410
//BC31 has a bug. It claims that & on member functions are superflous, when
//they in fact are required
#pragma warn -amp
#endif

//typedef to avoid ambiguous typecast later on
typedef UINT (CALLBACK *StdFileHookFunc_t)(HWND,UINT,WPARAM,LPARAM);

class BIFCLASS StdFileHack {
public:
static UINT CALLBACK StdFileInitProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

UINT CALLBACK StdFileHack::StdFileInitProc(HWND hwnd,
                                           UINT message,
                                           WPARAM wParam,
                                           LPARAM lParam)
{
        //This function is the "hook" specified in the OPENFILENAME structure

        
        //First when wm_initdialog arrives we can unhook the common dialog
        //and change button text
        if(message==WM_INITDIALOG) {
                //unhook and change default button text
                OPENFILENAME FAR *ofnp=(OPENFILENAME FAR *)lParam;
                //remove ourselves
                ofnp->lpfnHook = NULL;
                ofnp->Flags &= ~OFN_ENABLEHOOK;
                //change button text (This piece of code ought to be moved into
                // a handler)
                FStdFileDialog *dlg=(FStdFileDialog *)(ofnp->lCustData);
                if(dlg->okButtonText)
                        SetDlgItemText(hwnd,IDOK,dlg->okButtonText);
        }


        //Subclassing stuff
        
        //This extra check is necessarry because we cannot remove ourselves
        //before wm_initdialog is received (which is not necessarily the first)
        if(GetWndMan()->FindWnd(hwnd) != NULL) {
                //we are already subclassed, so any necessary processing
                //has already been done
                return FALSE;//not processed
        }
                
        //The windowmanager does not know about us 
        FWnd *pwnd=GetWndMan()->GetCreationWindow();
        if(pwnd) {
                //subclass and register the window
                GetWndMan()->SubRegWnd(pwnd,hwnd);
                //dispatch first event
                FEvent event(hwnd,message,wParam,lParam);
                if(pwnd->DispatchNoDefault(event)) {
                        //check for special cases
#if BIFOS_==BIFOS_WIN16_
                        if(message==WM_CTLCOLOR ||
#else
                        if(message==WM_CTLCOLORBTN ||
                           message==WM_CTLCOLORDLG ||
                           message==WM_CTLCOLOREDIT ||
                           message==WM_CTLCOLORLISTBOX ||
                           message==WM_CTLCOLORMSGBOX ||
                           message==WM_CTLCOLORSCROLLBAR ||
                           message==WM_CTLCOLORSTATIC ||
#endif
                           message==WM_COMPAREITEM ||
                           message==WM_VKEYTOITEM ||
                           message==WM_CHARTOITEM ||
                           message==WM_QUERYDRAGICON ||
                           message==WM_INITDIALOG)
                                return (BOOL)event.GetResult();
                        else
                                return TRUE;
                } else
                        return FALSE;
        } else
                return FALSE;   //this should not happen
}


Bool FStdFileDialog::DoModal(FWnd *pOwner, sfd_type type,
                             char *filename,
                             const char *filterMask,
                             const char *dlgTitle, const char *okButtonText
                            )
{
        TRACE_METHOD1("FStdFileDialog::DoModal");

        OPENFILENAME ofn;
        memset(&ofn,0,sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = pOwner?pOwner->GetHwnd():NULL;
        ofn.hInstance = NULL; //we dont use a dialog template

        //filter
        if(!filterMask)
                filterMask="*.*|*.*";
        char *filter=new char[lstrlen(filterMask)+2];
        if(!filter) return False;
        lstrcpy(filter,filterMask);
        char *p=filter;
        while(*p) {
                if(*p=='|') *p++='\0';
                else p++;
        }
        *++p='\0';
        ofn.lpstrFilter = filter;


        ofn.lpstrFile=filename;
        ofn.nMaxFile=128;

        if(dlgTitle)
                ofn.lpstrTitle = dlgTitle;

        ofn.Flags = OFN_ENABLEHOOK|OFN_PATHMUSTEXIST;

        ofn.lCustData = (long)this;
        
        FARPROC lpfnStdFileInitProc=MakeProcInstance((FARPROC)&StdFileHack::StdFileInitProc,GetWndMan()->GetInstance());
        ofn.lpfnHook = (StdFileHookFunc_t)lpfnStdFileInitProc;

        FStdFileDialog::okButtonText = okButtonText;

        GetWndMan()->SetCreationWindow(this);
        BOOL r;
        if(type==sfd_open)
                r=GetOpenFileName(&ofn);
        else
                r=GetSaveFileName(&ofn);
        GetWndMan()->GetCreationWindow();       //destructive read
        AfterDestroy();

#if BIFOS_!=BIFOS_WIN32_
        FreeProcInstance(lpfnStdFileInitProc);
#endif
                
        delete[] filter;

        if(r) return True;
        else return False;
}


Bool FStdFileDialog::DoModal(FWnd *pOwner, sfd_type type,
                             char *filenameBuffer, int maxFilenameBuffer,
                             char **filename, int maxFiles,
                             const char *filterMask,
                             const char *dlgTitle, const char *okButtonText
                            )
{
        TRACE_METHOD1("FStdFileDialog::DoModal");

        OPENFILENAME ofn;
        memset(&ofn,0,sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = pOwner?pOwner->GetHwnd():NULL;
        ofn.hInstance = NULL; //we dont use a dialog template

        //filter
        if(!filterMask)
                filterMask="*.*|*.*";
        char *filter=new char[lstrlen(filterMask)+2];
        if(!filter) return False;
        lstrcpy(filter,filterMask);
        {
                char *p=filter;
                while(*p) {
                        if(*p=='|') *p++='\0';
                        else p++;
                }
                *++p='\0';
        }
        ofn.lpstrFilter = filter;

        filenameBuffer[0]='\0';
        ofn.lpstrFile=filenameBuffer;
        ofn.nMaxFile=maxFilenameBuffer;

        if(dlgTitle)
                ofn.lpstrTitle = dlgTitle;

        ofn.Flags = OFN_ENABLEHOOK|OFN_PATHMUSTEXIST|OFN_ALLOWMULTISELECT;

        ofn.lCustData = (long)this;
        
        FARPROC lpfnStdFileInitProc=MakeProcInstance((FARPROC)&StdFileHack::StdFileInitProc,GetWndMan()->GetInstance());
        ofn.lpfnHook = (StdFileHookFunc_t)lpfnStdFileInitProc;

        FStdFileDialog::okButtonText = okButtonText;

        GetWndMan()->SetCreationWindow(this);
        BOOL r;
        if(type==sfd_open)
                r=GetOpenFileName(&ofn);
        else
                r=GetSaveFileName(&ofn);
        GetWndMan()->GetCreationWindow();       //zap creation window in case of error
        AfterDestroy();

#if BIFOS_!=BIFOS_WIN32_
        FreeProcInstance(lpfnStdFileInitProc);
#endif
                
        delete[] filter;

        if(r==0) 
                return False;

        if(strchr(filenameBuffer,' ')) {
                //buffer contains multiple names
                //split filename buffer and set pointers
                char *tmpbuf = new char [strlen(filenameBuffer)];
                if(!tmpbuf) return False;
                strcpy(tmpbuf,filenameBuffer);

                char *dirName=tmpbuf;
                char *src=strchr(tmpbuf,' ')+1;
                char *dst=filenameBuffer;
                int dirNameLen=src-dirName-1;
                int files=0;
                while(src && files<maxFiles-1) {
                        char *next=strchr(src,' ');
                        if(next) *next++='\0';
                        int bytesNeeded = dirNameLen+1+strlen(src)+1;
                        if( bytesNeeded + (dst-filenameBuffer) > maxFilenameBuffer) break;
                        memcpy(dst,dirName,dirNameLen);
                        dst[dirNameLen] = '\\';
                        strcpy(dst+dirNameLen+1,src);
                        filename[files++]=dst;
                        while(*dst) dst++;
                        *dst++='\0';
                        src=next;
                }
                filename[files] = 0;
                delete[] tmpbuf;
        } else {
                //buffer contains only one name
                filename[0]=filenameBuffer;
                filename[1]=0;
        }
        
        return True;
}

