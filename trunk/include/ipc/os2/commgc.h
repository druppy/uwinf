#ifndef __COMMGC_H
#define __COMMGC_H

class CommGCThread : public FThread {
        FServer *server;
        FEventSemaphore workToDo;
        int pleaseTerminate;
public:
        CommGCThread(FServer *s)
          : FThread(), server(s), pleaseTerminate(0)
          {}
        void Go();
        void WakeUp() {
                workToDo.Post();
        }
        void PleaseTerminate() {
                pleaseTerminate=1;
                workToDo.Post();
        }
};

#endif
