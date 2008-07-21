#define BIFINCL_WINDOW
#define BIFINCL_DYNMSG
#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#include <string.h>

struct DynamicMessage {
        char name[128];
        WmsMSG msg;
        DynamicMessage *next;
};

static DynamicMessage *first=0;
static WmsMSG nextMsg=0xf000;
static FMutexSemaphore dynMutex;

WmsMSG RegisterDynamicMessage(const char *s) {
        dynMutex.Request();
        for(DynamicMessage *p=first; p; p=p->next) {
                if(stricmp(p->name,s)==0) {
                        dynMutex.Release();
                        return p->msg;
                }
        }
        p=new DynamicMessage;
        if(!p) {
                dynMutex.Release();
                return 0;
        }
        strcpy(p->name,s);
        p->msg=nextMsg++;
        p->next=first;
        first=p;
        dynMutex.Release();
        return p->msg;
}

