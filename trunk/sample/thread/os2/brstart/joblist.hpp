#ifndef __JOBLIST_HPP
#define __JOBLIST_HPP

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

struct Job;

class JobList {
public:
        JobList();
        ~JobList();

        void AddJob(Job *j);
        void RemoveJob(Job *j);

        void StartTraversal();
        Job *GetNext();
        void EndTraversal();

private:
        FMutexSemaphore mutex;                  //mutex to protect the list
        struct entry {
                entry *prev,*next;
                Job *j;
        } *first;                               //double-linked list
        entry *current;
};

extern JobList jobList;

#endif

