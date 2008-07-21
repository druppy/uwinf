#define WMSINCL_BW
#include "paintq.hpp"

void PaintQueue::Insert(WmsHWND hwnd) {
        if(please_terminate) return;
        if((head+1)%MAXHWNDSINQUEUE==tail)
                ;//more than MAXHWNDSINQUEUE windows needs repainting - ignore
        else {
                int i=tail-1;
                do {
                        i=(i+1)%MAXHWNDSINQUEUE;
                        if(e[i]==hwnd) return;
                } while(i!=head);
                e[head] = hwnd;
                head = (head+1)%MAXHWNDSINQUEUE;
                notEmpty.Post();
        }
}

void PaintQueue::Remove(WmsHWND hwnd) {
        for(int i=0; i<MAXHWNDSINQUEUE; i++)
                if(e[i]==hwnd) e[i]=0;
}

WmsHWND PaintQueue::Get() {
        while(!please_terminate) {
                if(tail==head) {
                        notEmpty.Wait();
                        notEmpty.Reset();
                }
                if(please_terminate) return 0;
                while(tail!=head && !please_terminate) {
                        WmsHWND hwnd=e[tail];
                        tail = (tail+1)%MAXHWNDSINQUEUE;
                        if(hwnd!=0)
                                return hwnd;
                }
        }
        return 0;
}

void PaintQueue::PleaseTerminate() {
        please_terminate=True;
        notEmpty.Post();
}



