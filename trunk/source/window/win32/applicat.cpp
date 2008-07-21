/*************************************************************************
FILE 
  Applicat.cpp
DESCRIPTION
  Implementation of the FApplication class.
HOST
  BC40, SC61, WC 10-10.6
HISTORY
  94-11-?? ISJ Creation
  95-01-20 BL  Changed to work only in a small core, and more ...
  95-02-28 ISJ hinstPrev stuff added
  95-03-01 ISJ Reduced number of static variables
  95-03-11 ISJ Made it DLLable
  96-06-02 ISJ Removed DLL support (was too ugly)
  96-10-06 ISJ Implemented support for quited arguments in lpCmdLine
  97-02-01 ISJ Implemented default resource module stuff
*************************************************************************/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#include <bif.h>
#include <dbglog.h>

FModule *FModule::defresmodule=0;

FModule *FModule::GetDefaultResourceModule() {
        return defresmodule?defresmodule:GetCurrentApp();
}

void FModule::SetDefaultResourceModule(FModule *m) {
        defresmodule=m;
}



static HINSTANCE app_init_hinst,app_init_hinstprev;
static FApplication *g_pGlobalApp=0;
static DWORD g_mainthreadTID=0;

FApplication * BIFFUNCTION GetCurrentApp( void )
{
        return( g_pGlobalApp );
}


FApplication::FApplication( void )
  : FModule(app_init_hinst),
    wndManager(app_init_hinst, app_init_hinstprev)
{
        TRACE_METHOD1("FApplication::FApplication");
        // Register the first instance
        g_pGlobalApp = this;
        g_mainthreadTID = GetCurrentThreadId();
}

/***
FUNCTION
  int FApplication::Main( int argc, char **argv )
PARAMETER
  argc : Number of arguments.
  argv : array of agument strings (zascii).
RETURN
  errorlevel.
DESCRIPTION
  This is a virtual function to make the start up and shut down of a
  application more logical, by calling the start and stop functions
  in the class.
  The user can overwrite this function any time if it is more convinient
  for the current use, but this will disable StartUp and ShutDown.
***/
int FApplication::Main( int argc, char **argv )
{
        TRACE_METHOD1("FApplication::Main");
        if(StartUp( argc, argv )) {
                MessageLoop();
                return( ShutDown() );
        } else
                return -1;
}

FApplication::~FApplication()
{
        TRACE_METHOD1("FApplication::~FApplication");
        g_pGlobalApp = NULL;
}

// messageboxes 
int FApplication::DoMessageBox(HWND hwnd, 
                               const char *pszText, const char *pszTitle, 
                               UINT fuStyle)
{
        return ::MessageBox(hwnd, pszText, pszTitle, fuStyle);
}


/***
FUNCTION
  Bool GetEvent( FEvent &event )
PARAMETER
  event : The place to put the new event
VISIBILITY
  public of FApplication
RETURN
  False if the event is a WM_QUIT
DESRIPTION
  Return a message from the messagequeue.
WARNING
  Only experts should call this function.
***/
Bool FApplication::GetEvent( FEvent &event )
{
        return (Bool)GetMessage(&event, NULL, 0, 0 );
}


/***
FUNCTION
  void MessageLoop( void )
VISIBILITY
  public of FApplication
DESCRIPTION
  Standard message-loop
***/
void FApplication::MessageLoop( void )
{
        FEvent e;

        while( GetEvent(e) )
                ::GetWndMan()->PumpEvent(e);
}


/***
FUNCTION
  void TerminateApp( void )
VISIBILITY
  public of FApplication
DESCRIPTION
  Terminate the application. The application is not terminated
  at once, but rather suggested that it should return from its
  message-loop.
***/
void FApplication::TerminateApp( void )
{
        TRACE_METHOD1("FApplication::TerminateApp");
        PostThreadMessage( g_mainthreadTID, WM_QUIT, (WPARAM)0, (LPARAM)0 );
}


/***
Visibility: public (but it's a hack and may be removed in future versions
***/
void FApplication::SetHINSTANCE(HINSTANCE hinst,HINSTANCE hinstprev) {
        app_init_hinst = hinst;
        app_init_hinstprev = hinstprev;
}


/***
Visibility: local to this module
Parameters:
        hInst           handle to the application
        lpszCmdLine     command line string
        argc            (output) argument count
        argv            (output) argument vector
        argv_buf        (output) buffer arguments
Description:
        Create argc and argv from the stupid Windows hinstance and lpCmdLine
***/
static void MakeArgcArgv(HINSTANCE hInst, LPSTR lpszCmdLine,
                         int *argc, char **argv, char *argv_buf
                        )
{
        TRACE_PROC0("MakeArgcArgv");

        //create argv[0]
        //ANSI says program name or ""
        //We choose program name
        GetModuleFileName(hInst, argv_buf, 140);
        argv[0]=argv_buf;
        *argc=1;

        LPSTR src=lpszCmdLine;
        char *dst=argv_buf+lstrlen(argv_buf)+1;

        //skip initial blanks in command line
        while(*src==' ' || *src=='\t')
                src++;

        //create the rest of the arguments
        while(*src) {
                //store argument start in vector
                argv[*argc]=dst;
                ++*argc;

                //copy until blank is found
                while(*src && *src!=' ' && *src!='\t') {
                        if(*src=='"') {
                                //handle quoted argument
                                src++;
                                while(*src && *src!='"')
                                        *dst++=*src++;
                                if(*src=='"') src++;
                        } else
                                *dst++=*src++;
                }
                *dst++='\0';    //terminate with '\0'

                //skip trailing blanks
                while(*src==' ' || *src=='\t')
                        src++;
        }
        argv[*argc]=0;   //terminate vector with a NULL pointer
}



/***
FUNCTION
  int _BIFWindowMain(HINSTANCE,HINSTANCE,LPSTR,int)
VISIBILITY
  global
PARAMETERS
  (OS-specific)
  hinst          current instance
  hinstPrev      previous instance
  lpCmdLine      command line
  nCmdShow       recommended initial window state
  pfn            pointer to function that will create a FApplication instance
DESCRIPTION
  Create the application instance and run it. This is done by calling
  MakeApplicationInstance() and
        
  Terminate the application. The application is not terminated
  at once, but rather suggested that it should return from its
  message-loop.
***/
int _BIFWindowMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR lpCmdLine, int nCmdShow,FApplication *(*pfn)()) {
        TRACE_PROC0("_BIFWindowMain");
        
        FApplication::SetHINSTANCE(hinst,hinstPrev);

        int r;
        
        int argc;
        char **argv;
        char *argv_buf;
        argv_buf = new char[256+lstrlen(lpCmdLine)+1];
        argv = new char*[1+lstrlen(lpCmdLine)/2+1];
        if(argv!=0 && argv_buf!=0) {
                MakeArgcArgv(hinst,lpCmdLine, &argc,argv,argv_buf);
                
                FApplication *app=pfn();
                if(app) {
                        
                        app->m_nCmdShow = nCmdShow;
                        
                        if(!GetWndMan()->RegisterThread(0)) {
                                r=-3;
                        } else {
                                r=app->Main(argc,argv);
                                GetWndMan()->DeregisterThread(0);
                        }
                        delete app;
                } else
                        r=-2;      //probably out of memory
                delete[] argv;
                delete[] argv_buf;
        } else
                r=-1;
        return r;
}
    
