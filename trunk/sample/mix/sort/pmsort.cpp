#define INCL_GPI

#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#define BIFINCL_WINDOWTHREAD
#define BIFINCL_WINDOW
#define BIFINCL_APPWND
#define BIFINCL_MOUSEHANDLERS
#define BIFINCL_CLIENTHANDLERS
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_APPLICATION
#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_SIZERESTRICTIONHANDLERS
#include <bif.h>
#include <dbglog.h>

#include "pmsort.rh"
#include "sort.h"
#include "quick.cpp"
#include "batcher.cpp"
#include "shell.cpp"
#include "comb.cpp"
#include "heap.cpp"
#include "radixexc.cpp"

#include "sortmoni.h"
#include "sortutil.h"

#include <stdlib.h>




// Set up constants that defines the number of items in the sorts and the range
// of the elements
// Assumes 600x400 display
const unsigned N=300;
const MaxValue = 400;

//Global array for sorting
MonitoredInt e[N];                        // array being sorted


int displayDisabled=0;  //if nonzero changes should not be painted

#define BKCOLOR CLR_BLACK
#define FGCOLOR CLR_RED


void showArray(HPS hps, const MonitoredInt *a, int n) {
        if((!isArrayElement(a)) || displayDisabled) return;

        int low = int(a-e);
        int high = low+n;
        int x;

        POINTL p[2];

        GpiSetColor(hps, FGCOLOR);
        p[0].y=0;
        for(x=low; x<high; x++) {
                p[0].x = p[1].x = x*2;
                p[1].y = (LONG)e[x];
                GpiPolyLineDisjoint(hps, 2, p);
        }

        GpiSetColor(hps, BKCOLOR);
        p[1].y=MaxValue;
        for(x=low; x<high; x++) {
                p[0].x = p[1].x=x*2;
                p[0].y = (LONG)e[x];
                GpiPolyLineDisjoint(hps, 2, p);
        }
}

HWND hwndSortClient;
void showArray(const MonitoredInt *a, int n) {
        if((!isArrayElement(a)) || displayDisabled) return;

        HPS hps=WinGetPS(hwndSortClient);
        if(hps) {
                showArray(hps,a,n);
                WinReleasePS(hps);
        }
}


void eraseBackground(HPS hps, const FRect *pr) {
        WinFillRect(hps, (PRECTL)pr, BKCOLOR);
}



class SortThread : public FWWindowThread {
        FFrameWindow *frame;
        Bool *sorting;
        int alg;
public:
        SortThread(FFrameWindow *f, Bool *isSorting, int algorithm)
          : FWWindowThread(),
            frame(f),
            sorting(isSorting),
            alg(algorithm)
          {}

        void Go();
};

void SortThread::Go() {
        frame->SetSubtitle("Sorting");

        assigns=comps=swaps=0;

        switch(alg) {
                case IDM_QUICKSORT: {
                        QuickSortClass<MonitoredInt,SortComperator<MonitoredInt>,SortSwapper<MonitoredInt> > qsorter;
                        qsorter.Sort(e,N);
                        break;
                }
                case IDM_BATCHERSORT: {
                        BatcherSortClass<MonitoredInt,SortComperator<MonitoredInt>,SortSwapper<MonitoredInt> > bsorter;
                        bsorter.Sort(e,N);
                        break;
                }
                case IDM_HEAPSORT: {
                        HeapSortClass<MonitoredInt,SortComperator<MonitoredInt>,SortSwapper<MonitoredInt> > hsorter;
                        hsorter.Sort(e,N);
                        break;
                }
                case IDM_SHELLSORT: {
                        ShellSortClass<MonitoredInt,SortComperator<MonitoredInt> > ssorter;
                        ssorter.Sort(e,N);
                        break;
                }
                case IDM_COMBSORT: {
                        CombSortClass<MonitoredInt,SortComperator<MonitoredInt>,SortSwapper<MonitoredInt> > csorter;
                        csorter.Sort(e,N);
                        break;
                }
                case IDM_RADIXEXCHANGE: {
                        RadixExchangeSortClass<MonitoredInt,SortBits<MonitoredInt>,SortSwapper<MonitoredInt>,16 > resorter;
                        resorter.Sort(e,N);
                        break;
                }
        }
        frame->SetSubtitle(0);
        *sorting=False;
}



void setCaption(FFrameWindow *pwnd, int alg) {
        //set window caption based on the sorting algorithm
        switch(alg) {
                case IDM_QUICKSORT:
                        pwnd->SetCaption("QuickSort");
                        break;
                case IDM_BATCHERSORT:
                        pwnd->SetCaption("BatcherSort");
                        break;
                case IDM_HEAPSORT:
                        pwnd->SetCaption("HeapSort");
                        break;
                case IDM_SHELLSORT:
                        pwnd->SetCaption("ShellSort");
                        break;
                case IDM_COMBSORT:
                        pwnd->SetCaption("CombSort");
                        break;
                case IDM_RADIXEXCHANGE:
                        pwnd->SetCaption("Radix Exchange");
                        break;
                default:
                        pwnd->SetCaption("Unknown");    //what?
        }
}


void FillDataSet(int dataSetType) {
        switch(dataSetType) {
                case IDM_RANDOM:
                        fillDataSet(dst_random,e,N);
                        break;
                case IDM_MOSTLYSORTED:
                        fillDataSet(dst_mostlysorted,e,N);
                        break;
                case IDM_ALMOSTSORTED:
                        fillDataSet(dst_almostsorted,e,N);
                        break;
                case IDM_SORTED:
                        fillDataSet(dst_sorted,e,N);
                        break;
                case IDM_REVERSESORTED:
                        fillDataSet(dst_reverse,e,N);
                        break;
                case IDM_SMALLVALUERANGE:
                        fillDataSet(dst_smallrange,e,N);
                        break;
        }
}



class SortClientWindow : public FClientWindow,
                         public FMouseHandler,
                         public FPaintHandler,
                         public FCommandHandler,
                         public FCloseHandler,
                         public FSizeRestrictionClientHandler
{
public:
        SortClientWindow(FSizeRestrictionFrameHandler *pframehandler)
          : FClientWindow(), 
            FMouseHandler(this),
            FPaintHandler(this),
            FCommandHandler(this),
            FCloseHandler(this),
            FSizeRestrictionClientHandler(this,pframehandler)
          { isSorting=False; sortingAlgorithm=0; dataSetType=0; sortThread=0; }
        ~SortClientWindow() { delete sortThread; }
                
        Bool Create(FAppWindow *pwnd);
protected:
        Bool ControlPointer(FControlPointerEvent& );
        Bool Paint(FPaintEvent&);
        Bool Command(FCommandEvent&);
        Bool Close(FEvent&);
        Bool GetMinimumWidth(int &w) {w=N*2+1; return True;}
        Bool GetMinimumHeight(int &h) {h=MaxValue+1; return True;}
        Bool GetMaximumWidth(int &w) {w=N*2+1; return True;}
        Bool GetMaximumHeight(int &h) {h=MaxValue+1; return True;}
private:
        void StartSort();
        Bool isSorting;
        int sortingAlgorithm;         // IDM_xxxx
        int dataSetType;              // IDM_xxxx
        void setAlgorithm(int newAlg);
        void setDataSet(int newSet);
        FThread *sortThread;
};

Bool SortClientWindow::Create(FAppWindow *pwnd)
{
        if(!FClientWindow::Create(pwnd))
                return False;
        //set global variable
        hwndSortClient=GetHwnd();       
        //initialize menu
        setAlgorithm(IDM_QUICKSORT);
        setDataSet(IDM_RANDOM);
        //restrict the size
        ApplySizeRestrictions();
        return True;
}

Bool SortClientWindow::ControlPointer(FControlPointerEvent& ev) {
        if(isSorting)
                ev.SetPointer(*GetSystemWaitPointer());
        else
                ev.SetPointer(*GetSystemArrowPointer());
        return True;
}

Bool SortClientWindow::Paint(FPaintEvent& ev) {
        FRect rc;
        HPS hps = WinBeginPaint(GetHwnd(),0,&rc);
        if(hps) {
                eraseBackground(hps,&rc);
                showArray(hps,e,N);
                WinEndPaint(hps);
        }
        ev.SetResult(0);
        return True;
}

Bool SortClientWindow::Close(FEvent&) {
        if(isSorting)
                return True;    //reject
        else 
                return False;   //we don't mind closing the window
}

Bool SortClientWindow::Command(FCommandEvent& ev) {
        if(isSorting)
                return True;    //reject all commands while sorting
        switch(ev.GetItem()) {
                case IDM_QUICKSORT:
                case IDM_HEAPSORT:
                case IDM_SHELLSORT:
                case IDM_COMBSORT:
                case IDM_BATCHERSORT:
                case IDM_RADIXEXCHANGE: {
                        setAlgorithm(ev.GetItem());
                        return True;
                }
                case IDM_RANDOM:
                case IDM_MOSTLYSORTED:
                case IDM_ALMOSTSORTED:
                case IDM_SORTED:
                case IDM_REVERSESORTED:
                case IDM_SMALLVALUERANGE: {
                        setDataSet(ev.GetItem());
                        return True;
                }
                case IDM_NEXT: {
                        if(dataSetType<IDM_SMALLVALUERANGE)
                                //new dataset
                                setDataSet(dataSetType+1);
                        else {
                                //new algorithm and dataset
                                setDataSet(IDM_RANDOM);
                                if(sortingAlgorithm<IDM_RADIXEXCHANGE)
                                        setAlgorithm(sortingAlgorithm+1);
                                else
                                        setAlgorithm(IDM_QUICKSORT);
                        }
                        return True;
                }
                case IDM_SORT: {
                        StartSort();
                        return True;
                }
        }
        return False;
}

void SortClientWindow::setAlgorithm(int newAlg) {
        FMenu *m=GetAppWindow()->GetMenu();
        m->CheckItem(sortingAlgorithm,False);
        sortingAlgorithm=newAlg;
        m->CheckItem(sortingAlgorithm,True);
        setCaption(GetAppWindow(),sortingAlgorithm);
}

void SortClientWindow::setDataSet(int newSet) {
        FMenu *m=GetAppWindow()->GetMenu();
        m->CheckItem(dataSetType,False);
        dataSetType=newSet;
        m->CheckItem(dataSetType,True);
        FillDataSet(dataSetType);
}


void SortClientWindow::StartSort() {
        isSorting=True;
        if(sortThread) delete sortThread;
        sortThread = new SortThread(GetAppWindow(),&isSorting,sortingAlgorithm);
        sortThread->Start();
        sortThread->SetPriority(-10);    //
}




class SortFrame : public FAppWindow, public FSizeHandler, public FSizeRestrictionFrameHandler {
        SortClientWindow client;
public:
        SortFrame() 
          : FAppWindow(), 
            FSizeHandler(this), 
            FSizeRestrictionFrameHandler(this), 
            client(this) 
          {}

        Bool Create(FWnd *pOwner);
};

Bool SortFrame::Create(FWnd *pOwner) {
        //create ourselves
        if(!FAppWindow::Create(pOwner,
                               IDSORT,
                               FAppWindow::fcf_border      |
                               FAppWindow::fcf_sysmenu     |
                               FAppWindow::fcf_titlebar    |
                               FAppWindow::fcf_minbutton   |
                               FAppWindow::fcf_menu,
                               FAppWindow::alf_icon        |
                               FAppWindow::alf_menu        |
                               FAppWindow::alf_accelerator
                              ))
                return False;
        //create the client
        if(!client.Create(this))
                return False;

        return True;
}


class FSortApplication : public FMTApplication {
        SortFrame frame;
public:
        Bool StartUp(int,char**);
        int ShutDown();
};


Bool FSortApplication::StartUp(int , char **) {
        if(frame.Create(GetDesktop())) {
                frame.Show();
                frame.BringToFront();
                return True;
        } else
                return False;
}

int FSortApplication::ShutDown() {
        frame.Destroy();
        return 0;
}


DEFBIFMIXMAIN(FSortApplication,FWMainWindowThread);

