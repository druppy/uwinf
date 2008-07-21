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
  97-02-01 ISJ Added Get/SetDefaultResourceModule()
*************************************************************************/
#include <windows.h>
#include <bifimex.w16>
#include <window\winc\wndmisc.h>
#include <window\win16\wndcore.h>
#include <window\win16\applicat.h>
#include <dbglog.h>


FModule *FModule::defresmodule=0;

FModule *FModule::GetDefaultResourceModule() {
        return defresmodule?defresmodule:GetCurrentApp();
}

void FModule::SetDefaultResourceModule(FModule *module) {
        defresmodule=module;
}


static HINSTANCE app_init_hinst,app_init_hinstprev;
static FApplication *g_pGlobalApp=0;

FApplication * BIFFUNCTION GetCurrentApp( void )
{
        return( g_pGlobalApp );
}


FApplication::FApplication( void )
  : FModule(app_init_hinst,0),
    wndManager(app_init_hinst, app_init_hinstprev)
{
        TRACE_METHOD1("FApplication::FApplication");
        // Register the first instance
        g_pGlobalApp = this;
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
        PostQuitMessage( 0 );
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
        
        app_init_hinst=hinst;
        app_init_hinstprev=hinstPrev;
        
        FApplication *app=pfn();
        if(app) {
                int argc;
                char *argv[1+64+1];
                char argv_buf[140+128];

                MakeArgcArgv(hinst,lpCmdLine, &argc,argv,argv_buf);
                
                app->m_nCmdShow = nCmdShow;
                int r=app->Main(argc,argv);
                delete app;
                return r;
        } else
                return -2;      //probably out of memory
}
    
