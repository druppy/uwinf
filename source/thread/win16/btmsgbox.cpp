/***
Filename: btmsgbox.cpp
Description:
  Implementation of BTMessageBox() - a BIF/Thread friendly MessageBox() function
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-11-?? Creation
  ISJ 94-12-23 Moved this module into BIF/Thread project
  ISJ 95-02-17 Changed the registration to not use static variables
               Fixed window class so that default button, keys, etc works
**/
#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>

#define MB_CLASSNAME "Bif/Thread-MessageBox"
#define WM_MBFINISHED (WM_USER+1)

#define ID_TEXT 101
#define ID_ICON 102

/***
Visibility: local to this module
Description:
        Create the icon in the messagebox if required
Parameters:
        hwnd      hwnd of the messagebox
        iconRect  (output) rectangle of the icon
        hwndIcon  (output) hwnd of the icon
        style     style bits of the messagebox
        hinst     hinstance of the application
Return:
        TRUE on success
***/
static BOOL IconSetup(HWND hwnd, RECT *iconRect, HWND *hwndIcon, UINT style, HINSTANCE hInst) {
        if((style&(MB_ICONASTERISK|MB_ICONEXCLAMATION|MB_ICONHAND|MB_ICONQUESTION))==0) {
                //no icon
                SetRect(iconRect,0,0,0,0);
                *hwndIcon=NULL;
                return TRUE;
        }

        SetRect(iconRect,0,0,32,32);

        *hwndIcon = CreateWindow("STATIC",
                                 "",
                                 WS_VISIBLE|WS_CHILD|SS_ICON,
                                 0,0,
                                 iconRect->right-iconRect->left,iconRect->bottom-iconRect->top,
                                 hwnd,
                                 (HMENU)ID_ICON,
                                 hInst,
                                 NULL
                                );
        if(!*hwndIcon)
                return FALSE;

        HICON hicon;
        switch(style&0x00f0) {
                case MB_ICONASTERISK:
                        hicon=LoadIcon(NULL,IDI_ASTERISK);
                        break;
                case MB_ICONEXCLAMATION:
                        hicon=LoadIcon(NULL,IDI_EXCLAMATION);
                        break;
                case MB_ICONHAND:
                        hicon=LoadIcon(NULL,IDI_HAND);
                        break;
                case MB_ICONQUESTION:
                        hicon=LoadIcon(NULL,IDI_QUESTION);
                        break;
        }
        if(!hicon) return FALSE;
        SendMessage(*hwndIcon,STM_SETICON,(WPARAM)hicon,0);
        return TRUE;
}


/***
Visibility: local to this module
Description:
        Create the static text in the messagebox
Parameters:
        hwnd      hwnd of the messagebox
        textRect  (output) rectangle (size) of the text
        hwndText  (output) hwnd of the text
        lpszText  the text
        hinst     hinstance of the application
Returns:
        TRUE on success
***/
static BOOL TextSetup(HWND hwnd, RECT *textRect, HWND *hwndText, LPCSTR lpszText, HINSTANCE hInst) {
        SetRect(textRect,0,0,GetSystemMetrics(SM_CXSCREEN),0);
        HDC hDC=GetDC(hwnd);
        DrawText(hDC, lpszText, -1, textRect, DT_CALCRECT|DT_LEFT|DT_NOPREFIX|DT_WORDBREAK);
        ReleaseDC(hwnd,hDC);

        *hwndText=CreateWindow("STATIC",
                               lpszText,
                               WS_VISIBLE|WS_CHILD|SS_LEFT|SS_NOPREFIX,
                               0,0,
                               textRect->right-textRect->left,textRect->bottom-textRect->top,
                               hwnd,
                               (HMENU)ID_TEXT,
                               hInst,
                               NULL
                              );
        if(!*hwndText)
                return FALSE;
        else
                return TRUE;
}

/***
Visibility: local to this module
Description:
        Create the buttons of the messagebox
Parameters:
        hwnd        hwnd of the messagebox
        buttonRect  (output) rectangle (size) of the button area
        hwndButton  (output) array of the button hwnd's
        fuStyle     style bits of the messagebox
        hinst       hinstance of the application
Returns:
        TRUE on success
***/
static BOOL ButtonSetup(HWND hwnd, RECT *buttonRect, HWND *hwndButton, UINT fuStyle, HINSTANCE hInst) {
        const char *bt[3];
        bt[0]=bt[1]=bt[2]=0;
//      Watcom 10.0a makes an additional writable datasegment on the line
//      below, so we use the line above
//      const char *bt[3]={0,0,0};

        //If anybody knows how to retreive the yes/no/cancel/retry/...-strings
        //depending on the language of Windows we please let us know!  We know
        //that they are in a stringtable in USER.EXE, but it is not documented
        //so that would probably not work in Windows95.  Currently, the button
        //strings are hardcoded.
        int id[3];
        switch(fuStyle&0x000f) {
                case MB_OK:
                        bt[0]="Ok";
                        id[0]=IDOK;
                        break;
                case MB_OKCANCEL:
                        bt[0]="Ok";
                        id[0]=IDOK;
                        bt[1]="Cancel";
                        id[1]=IDCANCEL;
                        break;
                case MB_ABORTRETRYIGNORE:
                        bt[0]="Abort";
                        id[0]=IDABORT;
                        bt[1]="Retry";
                        id[1]=IDRETRY;
                        bt[2]="Ignore";
                        id[2]=IDIGNORE;
                        break;
                case MB_YESNOCANCEL:
                        bt[0]="Yes";
                        id[0]=IDYES;
                        bt[1]="No";
                        id[1]=IDNO;
                        bt[2]="Cancel";
                        id[2]=IDCANCEL;
                        break;
                case MB_YESNO:
                        bt[0]="Yes";
                        id[0]=IDYES;
                        bt[1]="No";
                        id[1]=IDNO;
                        break;
                case MB_RETRYCANCEL:
                        bt[0]="Retry";
                        id[0]=IDRETRY;
                        bt[1]="Cancel";
                        id[1]=IDCANCEL;
                        break;
                default:
                        return -1;
        }
        int buttons;
        if(bt[2]) buttons=3;
        else if(bt[1]) buttons=2;
        else buttons=1;

        //calc default button
        int defButton=0;
        if(fuStyle&MB_DEFBUTTON2 && buttons>=2)
                defButton=1;
        if(fuStyle&MB_DEFBUTTON3 && buttons>=3)
                defButton=2;


        const DWORD dwDlgBaseUnits=GetDialogBaseUnits();
        const WORD dlgXU=LOWORD(dwDlgBaseUnits)/4;
        const WORD dlgYU=HIWORD(dwDlgBaseUnits)/8;

        buttonRect->left=0;
        buttonRect->top=0;
        buttonRect->bottom=dlgYU*12;
        buttonRect->right=0;
        for(int i=0; i<buttons; i++) {
                if(i>0) buttonRect->right+=dlgXU*6;
                int x=buttonRect->right,
                    y=buttonRect->top,
                    w=dlgXU*40,
                    h=dlgYU*12;
                hwndButton[i]=CreateWindow("BUTTON",
                                          bt[i],
                                          WS_VISIBLE|WS_TABSTOP|WS_CHILD|(i==defButton?BS_DEFPUSHBUTTON:BS_PUSHBUTTON),
                                          x,y,
                                          w,h,
                                          hwnd,
                                          (HMENU)id[i],
                                          hInst,
                                          NULL
                                         );
                if(!hwndButton[i])
                        return FALSE;
                buttonRect->right+=w;
        }
        for(;i<3;i++) hwndButton[i]=NULL;
        return TRUE;
}

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

/***
Visibility: local to this module
Description:
        Size the messagebox to fit the contents (text, icon and button) exactly
        Move the messagebox to be centered over the parent window
Parameters:
        hwnd                    hwnd of the messagebox
        iconRect, hwndIcon      rectangle and hwnd of the icon (if any)
        textRect, hwndText      rectangle and hwnd of the text
        buttonRect, hwndButton  rectangle and hwnds of the buttons
***/
static void SizeAndMove(HWND hwnd,
                        const RECT *iconRect,HWND hwndIcon,
                        const RECT *textRect,HWND hwndText,
                        const RECT *buttonRect, HWND *hwndButton)
{
        //Make the window look like this:
        //      ttttttttttttttt
        //   I  ttttttttttttttt
        //      ttttttttttttttt
        //
        //   bbbbb bbbbb bbbbb

        const horzMargin = 8;
        const vertMargin = 8;
        const horzSpace = 6;
        const vertSpace = 8;


        if(hwndIcon) {
                //place icon
                int ydisp;
                ydisp = MAX(0, (textRect->bottom-iconRect->bottom)/2);

                SetWindowPos(hwndIcon,NULL,horzMargin,vertMargin+ydisp,0,0,SWP_NOSIZE|SWP_NOZORDER);
        }

        {
                //place text
                int xdisp,ydisp;
                if(hwndIcon)
                        xdisp=iconRect->right+horzSpace;
                else
                        xdisp=0;
                if(textRect->bottom<iconRect->bottom)
                        ydisp=(iconRect->bottom-textRect->bottom)/2;
                else
                        ydisp=0;

                SetWindowPos(hwndText,NULL,horzMargin+xdisp,vertMargin+ydisp,0,0,SWP_NOSIZE|SWP_NOZORDER);
        }

        int contentsHeight;
        int contentsWidth;
        {
                //place buttons
                int mainWidth,mainHeight;
                if(hwndIcon) {
                        mainWidth = iconRect->right+horzSpace+textRect->right;
                        mainHeight= MAX(iconRect->bottom,textRect->bottom);
                } else {
                        mainWidth = textRect->right;
                        mainHeight= textRect->bottom;
                }

                int ydisp=vertMargin+mainHeight+vertSpace;
                int xdisp=MAX(0,(mainWidth-buttonRect->right)/2);

                for(int i=0; i<3 && hwndButton[i]; i++) {
                        RECT br;
                        GetWindowRect(hwndButton[i],&br);
                        MapWindowPoints(HWND_DESKTOP,hwnd,(POINT*)&br,2);
                        SetWindowPos(hwndButton[i],NULL,horzMargin+xdisp+br.left,ydisp,0,0,SWP_NOSIZE|SWP_NOZORDER);
                }

                contentsHeight = vertMargin+mainHeight+vertMargin+buttonRect->bottom+vertMargin;
                contentsWidth = horzMargin+mainWidth+horzMargin;
        }

        //place&size window

        int x,y,w,h;

        w = contentsWidth+2*(GetSystemMetrics(SM_CXDLGFRAME));
        h = GetSystemMetrics(SM_CYCAPTION) + contentsHeight + 2*GetSystemMetrics(SM_CYDLGFRAME);

        RECT parentRect;
        GetWindowRect(GetParent(hwnd),&parentRect);
        x=parentRect.left+((parentRect.right-parentRect.left)-contentsWidth)/2;
        y=parentRect.top+((parentRect.bottom-parentRect.top)-contentsHeight)/2;
        SetWindowPos(hwnd,NULL,x,y,w,h,SWP_NOZORDER);
}



/***
Visibility: local to this module
Description:
        Remove unusable items from the system menu of the messagebox
***/
static void ModifySysMenu(HWND hwnd) {
        //remove various items not applicable to messageboxes
        HMENU hmenu=GetSystemMenu(hwnd,FALSE);
        DeleteMenu(hmenu,SC_RESTORE,    MF_BYCOMMAND);
        DeleteMenu(hmenu,SC_SIZE,       MF_BYCOMMAND);
        DeleteMenu(hmenu,SC_MINIMIZE,   MF_BYCOMMAND);
        DeleteMenu(hmenu,SC_MAXIMIZE,   MF_BYCOMMAND);
}


/***
Visibility: global (but should be local to this module)
Description:
        WndProc of the messagebox
***/
LRESULT CALLBACK __export BTMBWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch(msg) {
                case WM_CREATE: {
                        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
                        LPCSTR lpszText = (LPCSTR)lpcs->lpCreateParams;
                        UINT style=(UINT)lpcs->style;

                        //---icon setup
                        RECT iconRect;
                        HWND hwndIcon;
                        if(!IconSetup(hwnd,&iconRect,&hwndIcon,style,lpcs->hInstance))
                                return -1;

                        //---text setup
                        RECT textRect;
                        HWND hwndText;
                        if(!TextSetup(hwnd,&textRect,&hwndText,lpszText,lpcs->hInstance))
                                return -1;

                        //buttons setup
                        RECT buttonRect;
                        HWND hwndButton[3];
                        if(!ButtonSetup(hwnd,&buttonRect,hwndButton,style,lpcs->hInstance))
                                return -1;

                        SizeAndMove(hwnd,&iconRect,hwndIcon,&textRect,hwndText,&buttonRect,hwndButton);

                        //modify system menu
                        ModifySysMenu(hwnd);
                        return 0;
                }

                case WM_COMMAND:
                        PostMessage(hwnd,WM_MBFINISHED,wParam,0);
                        return 0;

                case WM_CLOSE:
                        PostMessage(hwnd,WM_MBFINISHED,IDCANCEL,0);
                        return 0;

                default:
                        return DefWindowProc(hwnd,msg,wParam,lParam);
        }
}


/***
Visibility: local to this module
Description:
        Register the messagebox window class
Returns:
        TRUE on success
***/
static BOOL RegisterMBClass(HINSTANCE hInst) {
        WNDCLASS cls;
        if(GetClassInfo(hInst,MB_CLASSNAME,&cls)!=0)
                return TRUE;    //already registered

        cls.style         = CS_SAVEBITS;
        cls.lpfnWndProc   = (WNDPROC)BTMBWndProc;
        cls.cbClsExtra    = 0;
        cls.cbWndExtra    = DLGWINDOWEXTRA;
        cls.hInstance     = hInst;
        cls.hIcon         = NULL;
        cls.hCursor       = LoadCursor(NULL,IDC_ARROW);
        cls.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        cls.lpszMenuName  = NULL;
        cls.lpszClassName = MB_CLASSNAME;

        if(!RegisterClass(&cls))
                return FALSE;
        else 
                return TRUE;
}


/***
Visibility: global
Description:
        Act like the normal MessageBox() function, but allow threads to run
        while displaying the messagebox
Parameters:
        As for Windows' MessageBox()
Returns:
        As for Windows' MessageBox()
***/
int BIFFUNCTION BTMessageBox(HWND hwndParent, LPCSTR lpszText, LPCSTR lpszTitle, UINT fuStyle) {
        FWindowThread *wt=FMainWindowThread::GetMWT();
        HINSTANCE hInst;

        //check for cases we can't handle
        if(wt==0 ||
           (fuStyle&MB_SYSTEMMODAL)!=0 ||
           (fuStyle&MB_TASKMODAL)!=0)
                return ::MessageBox(hwndParent,lpszText,lpszTitle,fuStyle);

        hInst = BIFTGet_hInstance();

        if(!RegisterMBClass(hInst))
                return ::MessageBox(hwndParent,lpszText,lpszTitle,fuStyle);

        HWND hwndBox = CreateWindowEx(WS_EX_DLGMODALFRAME,
                                      MB_CLASSNAME,
                                      lpszTitle?lpszTitle:"Error",
                                      fuStyle|WS_BORDER|WS_CAPTION|WS_POPUP|WS_SYSMENU|WS_VISIBLE,
                                      0,0,      //x,y
                                      200,200,  //w,h (is modified by the box itself)
                                      hwndParent?hwndParent:HWND_DESKTOP,
                                      NULL,     //hmenu
                                      hInst,
                                      (void far*)lpszText
                                     );

        if(hwndBox) {
                if(hwndParent) EnableWindow(hwndParent,FALSE);

                MSG msg;
                while(wt->GetMessage(&msg)) {
                        if(msg.hwnd==hwndBox && msg.message==WM_MBFINISHED) {
                                if(hwndParent) EnableWindow(hwndParent,TRUE);
                                DestroyWindow(hwndBox);
                                return msg.wParam;
                        }
                        if(!IsDialogMessage(hwndBox,&msg)) {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                        }
                }
                //WM_QUIT recieved - repost
                PostAppMessage(GetCurrentTask(),WM_QUIT,msg.wParam,msg.lParam);
                if(hwndParent) EnableWindow(hwndParent,TRUE);
                DestroyWindow(hwndBox);
                return -1;
        } else {
                //probably out-of-memory - try to let Windows show a messagebox
                return ::MessageBox(hwndParent,lpszText,lpszTitle,fuStyle);
        }
}

