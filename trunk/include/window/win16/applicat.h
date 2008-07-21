#ifndef __APPLICAT_H_INCLUDED
#define __APPLICAT_H_INCLUDED
/************************************************************************
FILE
        Applicat.H
DESCRIPTION
        Contain the WinMain funktion, and make it posible to hide some
        of the system interface, to make it more simple to make a portable
        BIF application.
HISTORY
        20.01.95 : Merged bw_modu.h into this file.
*************************************************************************/

class FRect;    // Defined in WndMisc.h
class FEvent;   // Defined in WndCore.h
class FWndMan;  // Defined in WndCore.h

class BIFCLASS FModule {
        FModule(const FModule&);                // don't copy
        FModule& operator=(const FModule&);     // don't copy
public:
        
        FModule(HMODULE hmod) {
                hinstance=NULL;
                hmodule=hmod;
                zapModule=False;
        }
    
        FModule(HINSTANCE hinst, int /*dummy*/) {
                hinstance=hinst;
                hmodule=GetModuleHandle((LPCSTR)MAKELP(0,hinstance));
                zapModule=False;
        }
        

        FModule(const char *name) {
                hinstance=LoadLibrary(name);
                hmodule=GetModuleHandle((LPCSTR)MAKELP(0,hinstance));
                zapModule=True;
        }

        virtual ~FModule() {
        if(zapModule)
                FreeLibrary(hinstance);
        }
                                                                                           
        Bool Fail() const {
                return((Bool)(hmodule == NULL));
        }

        HMODULE GetHMODULE() const {
                return hmodule;
        }

        HINSTANCE GetHINSTANCE() const {
                return hinstance;
        }

        HBITMAP LoadBitmap(int resID)
          { return LoadBitmap(MAKEINTRESOURCE(resID)); }
        HBITMAP LoadBitmap(const char *resName) {
                return ::LoadBitmap(hmodule,resName);
        }

        HICON LoadIcon(int resID)
          { return LoadIcon(MAKEINTRESOURCE(resID)); }
        HICON LoadIcon(const char *resName) {
                return ::LoadIcon(hmodule,resName);
        }

        HCURSOR LoadCursor(int resID)
          { return LoadCursor(MAKEINTRESOURCE(resID)); }
        HCURSOR LoadCursor(const char *resName) {
                return ::LoadCursor(hmodule,resName);
        }

        HMENU LoadMenu(int resID)
          { return LoadMenu(MAKEINTRESOURCE(resID)); }
        HMENU LoadMenu(const char *resName) {
                return ::LoadMenu(hmodule, resName);
        }

        int LoadString(int resID, char *buf, int maxbuf) {
          return ::LoadString(hmodule,resID,buf,maxbuf);
        }

        static FModule *GetDefaultResourceModule();
        static void SetDefaultResourceModule(FModule *module);
private:
        HMODULE hmodule;
        HINSTANCE hinstance;
        Bool zapModule;

        static FModule *defresmodule;
};


class BIFCLASS FApplication : public FModule {
        friend int BIFFUNCTION _BIFWindowMain(HINSTANCE,HINSTANCE,LPSTR,int,FApplication *(*pfn)());
public:
        FApplication( void );
        virtual ~FApplication();

        // messages:
        virtual void MessageLoop();
        virtual Bool GetEvent( FEvent &event );

        //messagebox (use BIFINCL_MESSAGEBOX-functions instead)
        virtual int DoMessageBox(HWND hwnd, const char *pszText, const char *pszTitle, UINT fuStyle);
        
        // Stop all activity, please!
        void TerminateApp( void );


        // start and termination
        virtual Bool StartUp(int, char **) {return False;}
        virtual int ShutDown() {return 0;}
        virtual int Main( int argc, char **argv );
        
protected:
        int m_nCmdShow;
        FWndMan wndManager;
};

FApplication * BIFFUNCTION GetCurrentApp( void );



extern int BIFFUNCTION _BIFWindowMain(HINSTANCE,HINSTANCE,LPSTR,int, FApplication *(*pfn)());
FApplication *MakeApplicationInstance();

#define DEFBIFWINDOWMAIN(applicationClass)                                                 \
FApplication *MakeApplicationInstance()                                                    \
{                                                                                          \
        return new applicationClass;                                                       \
}                                                                                          \
                                                                                           \
int PASCAL WinMain(HINSTANCE hinst, HINSTANCE hprevInst, LPSTR lpCmdLine, int nCmdShow)    \
{                                                                                          \
        return _BIFWindowMain(hinst,hprevInst,lpCmdLine,nCmdShow,MakeApplicationInstance); \
}

#endif //__APPLICAT_H_INCLUDED

