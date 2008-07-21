#include "joblist.hpp"

JobList jobList;

JobList::JobList()
  : first(0), current(0)
{ }

JobList::~JobList() {
        mutex.Request();
          while(first) {
                  entry *n=first->next;
                  delete first;
                  first=n;
          }
          current=0;
        mutex.Release();
}
        
void JobList::AddJob(Job *j) {
        entry *e=new entry;
        mutex.Request();
          e->next=first;
          e->prev=0;
          e->j=j;
          if(first) first->prev=e;
          first=e;
        mutex.Release();
}

void JobList::RemoveJob(Job *j) {
        mutex.Request();
          entry *e=first;
          while(e && e->j!=j)
                e=e->next;
          if(e) {
                  if(e->prev) e->prev=e->next;
                  if(e->next) e->next->prev=e->prev;
                  if(e==first) first=e->next;
                  if(e==current) current=e->next;
                  delete e;
          }
        mutex.Release();
}


void JobList::StartTraversal() {
        mutex.Request();
        current=first;
}

Job *JobList::GetNext() {
        if(current) {
                Job *j=current->j;
                current=current->next;
                return j;
        } else
                return 0;
}

void JobList::EndTraversal() {
        mutex.Release();
}

