#include "conlist.hpp"

ConnectionList connectionList;

ConnectionList::ConnectionList()
  : first(0), current(0)
{ }

ConnectionList::~ConnectionList() {
        mutex.Request();
          while(first) {
                  entry *n=first->next;
                  delete first;
                 first=n;
          }
          current=0;
        mutex.Release();
}
        
void ConnectionList::AddConnection(Connection *c) {
        entry *e=new entry;
        mutex.Request();
          e->next=first;
          e->prev=0;
          e->c=c;
          if(first) first->prev=e;
          first=e;
        mutex.Release();
}

void ConnectionList::RemoveConnection(Connection *c) {
        mutex.Request();
          entry *e=first;
          while(e && e->c!=c)
                e=e->next;
          if(e) {
                  if(e->prev) e->prev->next=e->next;
                  if(e->next) e->next->prev=e->prev;
                  if(e==first) first=e->next;
                  if(e==current) current=e->next;
                  delete e;
          }
        mutex.Release();
}


void ConnectionList::StartTraversal() {
        mutex.Request();
        current=first;
}

Connection *ConnectionList::GetNext() {
        if(current) {
                Connection *c=current->c;
                current=current->next;
                return c;
        } else
                return 0;
}

void ConnectionList::Endtraversal() {
        mutex.Release();
}


