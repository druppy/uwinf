#include "tdelthrd.hpp"

ThreadDeleteThread::ThreadDeleteThread()
  : FThread(),
    first(0),
    list_mutex(0),
    threadsMarked(0)
{ }

void ThreadDeleteThread::Go() {
        for(;;) {
                threadsMarked.Wait();   //wait for work to do
                list_mutex.Request();   //lock list
                if(first==0) {
                        // We have been woken by PleaseTerminate,
                        // OK we will do so
                        list_mutex.Release();
                        break;
                } else {
                        entry *e=first;
                        first=first->next;
                        list_mutex.Release();
                        e->thread->Wait();  //wait for thread to really terminate
                        delete e->thread;
                        delete e;
                }
        }
}

void ThreadDeleteThread::MarkThreadForDelete(FThread *t) {
        entry *e = new entry;
        e->thread = t;
        //insert element in list
        list_mutex.Request();
          e->next = first;
          first = e;
        list_mutex.Release();
        threadsMarked.Signal();
}

void ThreadDeleteThread::PleaseTerminate() {
        //The thread is terminated by incrementing threadsMarked without
        //inserting a thread in the list
        threadsMarked.Signal();
}

