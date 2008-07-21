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
#define BIFINCL_MESSAGEBOX
#include <bif.h>

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

int displayDisabled=0;  //if nonzero changes should not be painted


// Set up constants that defines the number of items in the sorts and the range
// of the elements
// Assumes 600x400 display
const unsigned N=300;
const MaxValue = 400;

//Global array for sorting
MonitoredInt e[N];                        // array being sorted


#define BKCOLOR RGB(0,0,0)
#define FGCOLOR RGB(255,0,0)


void showArray(HDC hdc, const MonitoredInt *a, int n) {
        if((!isArrayElement(a)) || displayDisabled) return;

        int low = int(a-e);
        int high = low+n;
        int x;

        HPEN fgpen=CreatePen(PS_SOLID,0,FGCOLOR);
        HGDIOBJ oldPen=SelectObject(hdc,fgpen);


        for(x=low; x<high; x++) {
                if(e[x]!=0) {
                        POINT pt[2];
                        pt[0].x = x*2+1;
                        pt[0].y = MaxValue-1;
                        pt[1].x = x*2+1;
                        pt[1].y = MaxValue-e[x];
                        Polyline(hdc,pt,2);
                }
        }

        HPEN bkpen=CreatePen(PS_SOLID,0,BKCOLOR);
        SelectObject(hdc,bkpen);
        for(x=low; x<high; x++) {
                POINT pt[2];
                pt[0].x = x*2+1;
                pt[0].y = 0;
                pt[1].x = x*2+1;
                pt[1].y = MaxValue-e[x]+1;
                Polyline(hdc,pt,2);
                        
        }
        SelectObject(hdc,oldPen);
        DeleteObject(fgpen);
        DeleteObject(bkpen);
}

HWND hwndSortClient;

void eraseBackground(HDC hdc, const RECT *pr) {
        HBRUSH hbr = CreateSolidBrush(BKCOLOR);
        if(hbr) {
                FillRect(hdc,pr,hbr);
                DeleteObject(hbr);
        }
}

//FixMe
class SortThread : public FWWindowThread {
        FWnd *notifyWindow;
        Bool *sorting;
        int alg;
public:
        SortThread(FWnd *wndNotify, Bool *isSorting, int algorithm)
          : FWWindowThread(),
            notifyWindow(wndNotify),
            sorting(isSorting),
            alg(algorithm)
          {}

        void Go();
};

void showArray(const MonitoredInt *a, int n) {
        if(displayDisabled) return;
        HDC hdc=GetDC(hwndSortClient);
        if(hdc) {
                showArray(hdc,a,n);
                ReleaseDC(hwndSortClient,hdc);
        }
        FThreadManager::Yield();
}

void SortThread::Go() {
        //notify client that we have started
        PostMessage(notifyWindow->GetHwnd(), WM_USER, 1, 0);

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
        *sorting=False;
        //notify client that we have finished
        PostMessage(notifyWindow->GetHwnd(), WM_USER, 0, 0);
}



void setCaption(FFrameWindow *pwnd, int alg) {
        //set window caption based on the sorting algorithm
        switch(alg) {
                case IDM_QUICKSORT:
                        pwnd->SetCaption(FResourceString(IDS_QUICKSORT));
                        break;
                case IDM_BATCHERSORT:
                        pwnd->SetCaption(FResourceString(IDS_BATCHERSORT));
                        break;
                case IDM_HEAPSORT:
                        pwnd->SetCaption(FResourceString(IDS_HEAPSORT));
                        break;
                case IDM_SHELLSORT:
                        pwnd->SetCaption(FResourceString(IDS_SHELLSORT));
                        break;
                case IDM_COMBSORT:
                        pwnd->SetCaption(FResourceString(IDS_COMBSORT));
                        break;
                case IDM_RADIXEXCHANGE:
                        pwnd->SetCaption(FResourceString(IDS_RADIXEXCHANGE));
                        break;
                default:
                        pwnd->SetCaption("Unknown");    //what?
        }
}


void FillDataSet(int dataSetType) {
        displayDisabled++;
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
        displayDisabled--;
}

class FSortStatusHandler : public FHandler {
public: FSortStatusHandler(FWnd *pwnd)
          : FHandler(pwnd)
        { SetDispatch(dispatchFunc_t(&FSortStatusHandler::Dispatch)); }
        Bool Dispatch(FEvent &ev) {
                if(ev.GetID()==WM_USER) {
                        if(ev.wParam)
                                SortingStarted();
                        else
                                SortingFinished();
                        return True;
                } else
                        return False;
        }
protected:
        virtual void SortingStarted() =0;
        virtual void SortingFinished() =0;
};

class SortClientWindow : public FClientWindow,
                         public FMouseHandler,
                         public FPaintHandler,
                         public FCommandHandler,
                         public FCloseHandler,
                         public FSizeRestrictionClientHandler,
                         public FSortStatusHandler
{
public:
        SortClientWindow(FSizeRestrictionFrameHandler *pframehandler)
          : FClientWindow(), 
            FMouseHandler(this),
            FPaintHandler(this),
            FCommandHandler(this),
            FCloseHandler(this),
            FSizeRestrictionClientHandler(this,pframehandler),
            FSortStatusHandler(this)
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
        void SortingStarted();
        void SortingFinished();
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
        PAINTSTRUCT ps;
        if(BeginPaint(GetHwnd(),&ps)) {
                eraseBackground(ps.hdc,&ps.rcPaint);
                int low=ps.rcPaint.left/2-1;
                if(low<0) low=0;
                int high=ps.rcPaint.right/2+1;
                if(high>=N) high=N-1;
                showArray(ps.hdc,e+low,(high-low+1));
                EndPaint(GetHwnd(),&ps);
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
        Invalidate(0);
}


void SortClientWindow::StartSort() {
        isSorting=True;
        if(sortThread) delete sortThread;
        sortThread = new SortThread(this,&isSorting,sortingAlgorithm);
        if(!sortThread)
                MessageBox(this,"Not enough memory to create thread",0,mb_ok);
        else
                sortThread->Start();
        //sortThread->SetPriority(-10);    //
}

void SortClientWindow::SortingStarted() {
        GetAppWindow()->SetSubtitle("Sorting");
}
void SortClientWindow::SortingFinished() {
        GetAppWindow()->SetSubtitle(0);
}


//FixMe
class SortFrame : public FAppWindow,
//                  public FSizeHandler,
                  public FSizeRestrictionFrameHandler
{
        SortClientWindow client;
public:
        SortFrame()
          : FAppWindow(),
//            FSizeHandler(this),
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


//FixMe
class FSortApplication : public FMTApplication {
        SortFrame frame;
public:
        Bool StartUp(int argc, char *argv[]);
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

