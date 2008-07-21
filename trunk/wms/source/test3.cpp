#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#define WMSINCL_MSGS
#define WMSINCL_CPI
#define WMSINCL_DESKTOP
#define WMSINCL_BW
#define WMSINCL_SYSVALUE
#include <wms.hpp>

#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>

#include <stdlib.h>
#include <stdio.h>
//#include "source/wsamp/wsamp.hpp"
#include <time.h>

#include <malloc.h>

WmsHWND desktop,
        frame,
        closeButton,
        titlebar,
        minButton,
        maxButton,
        vertScrollBar,
        client;
#define wm_clientpainted (WM_USER+1)

Bool makeWindows() {
        desktop =
          WmsCreateWindow(0,                        //no parent
            0,0,
            (int)WmsQuerySysValue(0,SV_CXSCREEN),(int)WmsQuerySysValue(0,SV_CYSCREEN),
            HWND_TOP,
            0,                        //no owner
            WS_CLIPCHILDREN|WS_VISIBLE,
            0,
            0
          );
        if(!desktop) return False;
        WmsSetDesktop(desktop);


        frame =
          WmsCreateWindow(desktop,
            5,5,
            (int)WmsQuerySysValue(desktop,SV_CXSCREEN)*2/3,(int)WmsQuerySysValue(desktop,SV_CYSCREEN)/3,
            HWND_TOP,
            0,                        //no owner
            WS_VISIBLE|WS_CLIPCHILDREN,
            0,
            0
          );
        if(!frame) return False;

        FRect frameRect;
        WmsQueryWindowRect(frame,&frameRect);

        closeButton =
          WmsCreateWindow(frame,
            2,0,
            3,1,
            HWND_BOTTOM,
            frame,
            WS_VISIBLE,
            0,
            0
          );
        if(!closeButton) return False;

        titlebar =
          WmsCreateWindow(frame,
            6,0,
            6,1,
            HWND_BOTTOM,
            frame,
            WS_VISIBLE,
            0,
            0
          );
        if(!titlebar) return False;

        minButton =
          WmsCreateWindow(frame,
            frameRect.xRight-9,0,
            3,1,
            HWND_BOTTOM,
            frame,
            WS_VISIBLE,
            0,
            0
          );
          if(!minButton) return False;

        maxButton =
          WmsCreateWindow(frame,
            frameRect.xRight-5,0,
            3,1,
            HWND_BOTTOM,
            frame,
            WS_VISIBLE,
            0,
            0
          );
        if(!maxButton) return False;

        vertScrollBar =
          WmsCreateWindow(frame,
            frameRect.xRight-1,1,
            1,frameRect.GetHeight()-2,
            HWND_BOTTOM,
            frame,
            WS_VISIBLE,
            0,
            0
          );
        if(!vertScrollBar) return False;

        client =
          WmsCreateWindow(frame,
            1,1,
            frameRect.GetWidth()-2,frameRect.GetHeight()-2,
            HWND_BOTTOM,
            frame,
            WS_VISIBLE,
            0,
            0
          );
        if(!client) return False;

        return True;
}

void paintDesktop() {
        WmsHDC hdc=WmsBeginPaint(desktop);
        if(!hdc) return;

        FRect r;
        WmsQueryWindowRect(desktop,&r);
        CpiFillRect(hdc,r,clr_lightgray<<4|clr_darkgray,'Û');

        WmsEndPaint(hdc);
}

void paintFrame() {
        WmsHDC hdc=WmsBeginPaint(frame);
        if(!hdc) return;

        FRect r;
        WmsQueryWindowRect(frame,&r);

        const int frameColor=clr_blue<<4|clr_white;
        CpiEraseRect(hdc,r,frameColor);

        CpiSetCellData(hdc,FPoint(0,0),                                                frameColor<<8|'É');
        CpiFillRect(hdc,1,             0,              r.GetRight()-1,1,               frameColor,'Í');
        CpiSetCellData(hdc,FPoint(r.GetRight()-1,0),                                   frameColor<<8|'»');
        CpiFillRect(hdc,0,             1,              1,             r.GetHeight()-1, frameColor,'º');
        CpiFillRect(hdc,r.GetWidth()-1,1,              r.GetWidth(),  r.GetHeight()-1, frameColor,'º');
        CpiSetCellData(hdc,FPoint(0,r.GetHeight()-1),                                  frameColor<<8|'È');
        CpiFillRect(hdc,1,             r.GetHeight()-1,r.GetWidth()-1,r.GetHeight(),   frameColor,'Í');
        CpiSetCellData(hdc,FPoint(r.GetRight()-1,r.GetHeight()-1),                     frameColor<<8|'¼');

        WmsEndPaint(hdc);
}


void paintCloseButton() {
        WmsHDC hdc=WmsBeginPaint(closeButton);
        if(!hdc) return;

        FRect r;
        WmsQueryWindowRect(closeButton,&r);
        CpiEraseRect(hdc,r,clr_blue<<4);

        CpiSetBkMode(hdc,bm_transparent);

        FPoint p(0,0);
        CpiSetTextColor(hdc,clr_white);
        p=CpiOuttext(hdc,p,"[");
        CpiSetTextColor(hdc,clr_lightgreen);
        p=CpiOuttext(hdc,p,"þ");
        CpiSetTextColor(hdc,clr_white);
        CpiOuttext(hdc,p,"]");

        WmsEndPaint(hdc);
}


void paintTitlebar() {
        WmsHDC hdc=WmsBeginPaint(titlebar);
        if(!hdc) return;

        color fg=clr_white;
        color bg=clr_blue;

        FRect r;
        WmsQueryWindowRect(titlebar,&r);
        CpiFillRect(hdc,r,bg<<4|fg,' ');

        CpiSetTextColor(hdc,fg);
        CpiSetBkMode(hdc,bm_transparent);
        CpiOuttext(hdc,1,0,"Test");

        WmsEndPaint(hdc);
}


void paintMinButton() {
        WmsHDC hdc=WmsBeginPaint(minButton);
        if(!hdc) return;

        FRect r;
        WmsQueryWindowRect(minButton,&r);
        CpiEraseRect(hdc,r,clr_blue<<4);

        CpiSetBkMode(hdc,bm_transparent);

        FPoint p(0,0);
        CpiSetTextColor(hdc,clr_white);
        p=CpiOuttext(hdc,p,"[");
        CpiSetTextColor(hdc,clr_lightgreen);
        p=CpiOuttext(hdc,p,"\031");
        CpiSetTextColor(hdc,clr_white);
        CpiOuttext(hdc,p,"]");

        WmsEndPaint(hdc);
}

void paintMaxButton() {
        WmsHDC hdc=WmsBeginPaint(maxButton);
        if(!hdc) return;

        FRect r;
        WmsQueryWindowRect(maxButton,&r);
        CpiEraseRect(hdc,r,clr_blue<<4);

        CpiSetBkMode(hdc,bm_transparent);

        FPoint p(0,0);
        CpiSetTextColor(hdc,clr_white);
        p=CpiOuttext(hdc,p,"[");
        CpiSetTextColor(hdc,clr_lightgreen);
        p=CpiOuttext(hdc,p,"\030");
        CpiSetTextColor(hdc,clr_white);
        CpiOuttext(hdc,p,"]");

        WmsEndPaint(hdc);
}

void paintVertScrollBar() {
        WmsHDC hdc=WmsBeginPaint(vertScrollBar);
        if(!hdc) return;

        FRect r;
        WmsQueryWindowRect(vertScrollBar,&r);
        CpiFillRect(hdc,r,clr_blue<<4|clr_cyan,'±');
        CpiFillRect(hdc,FRect(0,0,1,1),clr_cyan<<4|clr_blue,'\036');
        CpiFillRect(hdc,FRect(0,r.yBottom-1,1,r.yBottom),clr_cyan<<4|clr_blue,'\037');

        WmsEndPaint(hdc);
}

void paintClient() {
        WmsHDC hdc=WmsBeginPaint(client);
        if(!hdc) return;

        FRect r;
        WmsQueryWindowRect(client,&r);

        CpiEraseRect(hdc,r,clr_blue<<4|clr_white);
        CpiSetBkMode(hdc,bm_transparent);

        FPoint p;

        CpiSetTextColor(hdc,clr_lightgray);
        CpiOuttext(hdc,0,0,"// sample hello world program");

        CpiSetTextColor(hdc,clr_yellow);
        CpiOuttext(hdc,0,2,"#include <stdio.h>");

        p.Set(0,4);
        CpiSetTextColor(hdc,clr_white);
        p=CpiOuttext(hdc,p,"int");
        CpiSetTextColor(hdc,clr_yellow);
        p=CpiOuttext(hdc,p," main(");
        CpiSetTextColor(hdc,clr_white);
        p=CpiOuttext(hdc,p,"void");
        CpiSetTextColor(hdc,clr_yellow);
        p=CpiOuttext(hdc,p,") {");

        CpiSetTextColor(hdc,clr_yellow);
        CpiOuttext(hdc,0,5,"        printf(\"Hello world!\\n\");");

        p.Set(0,6);
        CpiSetTextColor(hdc,clr_yellow);
        p=CpiOuttext(hdc,p,"        ");
        CpiSetTextColor(hdc,clr_white);
        p=CpiOuttext(hdc,p,"return");
        CpiSetTextColor(hdc,clr_yellow);
        p=CpiOuttext(hdc,p," 0;");


        CpiSetTextColor(hdc,clr_yellow);
        CpiOuttext(hdc,0,7,"}");
/*
        CpiDrawText(hdc,
                    "// Sample program - the classic hello world program\n"
                    "\n"
                    "#include <stdio.h>\n"
                    "\n"
                    "int main(void) {\n"
                    "        printf(\"Hello world!\\n\");\n"
                    "        return 0;\n"
                    "}\n",
                    -1,
                    &r,
                    clr_yellow,clr_blue,
                    DT_ERASERECT
                   );
*/
        WmsEndPaint(hdc);
        WmsPostMsg(client,wm_clientpainted,0,0);
}

WmsMRESULT MyWndProc(const WmsQMSG *qmsg) {
        switch(qmsg->msg) {
                case WM_PAINT:
                        if(qmsg->hwnd==desktop)
                                paintDesktop();
                        else if(qmsg->hwnd==frame)
                                paintFrame();
                        else if(qmsg->hwnd==closeButton)
                                paintCloseButton();
                        else if(qmsg->hwnd==titlebar)
                                paintTitlebar();
                        else if(qmsg->hwnd==minButton)
                                paintMinButton();
                        else if(qmsg->hwnd==maxButton)
                                paintMaxButton();
                        else if(qmsg->hwnd==vertScrollBar)
                                paintVertScrollBar();
                        else if(qmsg->hwnd==client)
                                paintClient();
                        break;
        }
        return 0; //no WmsDefWindowProc() yet
}

class MyMain:public FMainThread {
public: MyMain(int argc, char **argv) : FMainThread(argc,argv) {}
        int Main(int,char**);
};


int MyMain::Main(int,char**) {
        InitializeWMS();

        WmsHAB hab=WmsInitialize(0);
        if(!hab) return -1;

        WmsHMQ hmq=WmsCreateMsgQueue(hab,0,MyWndProc);
        if(!hmq) {
                WmsTerminate(hab);
                return -2;
        }

        if(!makeWindows()) return -3;

        FRect r;
        WmsQueryWindowRect(frame,&r);

        int maxx=(int)WmsQuerySysValue(desktop,SV_CXSCREEN) - r.GetWidth();
        int maxy=(int)WmsQuerySysValue(desktop,SV_CYSCREEN) - r.GetHeight();

clock_t starttime=clock();


        unsigned i=0;
        WmsQMSG qmsg;
        while(WmsGetMsg(hmq,&qmsg)) {
                //printf("got msg: %d  hwnd:%08x\n",qmsg.msg,qmsg.hwnd);
                if(qmsg.msg==wm_clientpainted) {
                        i++;
                        if(i==1000) {
                                WmsPostQueueMsg(hmq,WM_QUIT,0,0);
                        } else {
                                WmsSetWindowPos(frame,
                                                0,
                                                rand()%maxx,rand()%maxy,
                                                0,0,
                                                SWP_MOVE
                                               );
                                //invalidate because WmsSetWindowPos() might have optimized out of SWP_MOVE
                                WmsInvalidateRect(frame,0,True);
                        }
                } else {
                        WmsDispatchMsg(&qmsg);
                }
        }

/*
//message-less test:
        for(int i=0; i<10; i++) {
                WmsSetWindowPos(frame,
                                0,
                                rand()%maxx,rand()%maxy,
                                0,0,
                                SWP_MOVE
                               );
                paintDesktop();
                paintFrame();
                paintCloseButton();
                paintTitlebar();
                paintMinButton();
                paintMaxButton();
                paintVertScrollBar();
                paintClient();
        }
*/
clock_t endtime=clock();

        WmsDestroyMsgQueue(hmq);
        WmsTerminate(hab);

//        dumpSampling("wms.smp");

        TerminateWMS();

printf("\nRunning time: %f seconds\n\n",((double)(endtime-starttime))/CLK_TCK);
        return 0;
}

DEFBIFTHREADMAIN(MyMain);

/*
° 176
± 177
² 178
Û 219

º 186
» 187
¼ 188
È 200
É 201
Í 205
ú 250
þ 254

*/
