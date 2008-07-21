#ifndef __FREETHRD_HPP
#define __FREETHRD_HPP

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

/* Because some thread are allocated on the heap and only the threads
 * themselves know when they are going to terminate, delete'ing them
 * can be a problem. ThreadDeleteThread solves the problem. Before a
 * thread allocated on the heap termiantes it calls
 * ThreadDeleteThread::MarkThreadForDelete(this), the ThreadDeleteThread
 * will then wait for the thread to terminate and then delete it.
 */
class ThreadDeleteThread : public FThread {
        struct entry {
                FThread *thread;
                entry *next;
        } *first;
        FMutexSemaphore list_mutex;
        FSemaphore threadsMarked;
public:
        ThreadDeleteThread();

        void Go();

        void MarkThreadForDelete(FThread *t);
        void PleaseTerminate();
};

#endif
