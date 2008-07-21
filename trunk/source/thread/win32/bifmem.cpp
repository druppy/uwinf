#define INCL_DOSMEMMGR
#define INCL_DOSERRORS

#include <windows.h>
#include <thread\win32\bifmem.h>
#include "dbglog.h"



BIFMemoryPool::BIFMemoryPool(unsigned long poolSize) {
        TRACE_METHOD0("BIFMemoryPool::BIFMemoryPool");

        hHeap = HeapCreate(0, poolSize,poolSize);
        if(hHeap==NULL) {
                NOTE0("BIFMemoryPool::BIFMemoryPool: HeapCreate failed");
                return;
        }

}

BIFMemoryPool::~BIFMemoryPool() {
        TRACE_METHOD0("BIFMemoryPool::~BIFMemoryPool");

        if(hHeap && !HeapDestroy(hHeap)) {
                NOTE1("BIFMemoryPool::~BIFMemoryPool: HeapDestroy failed");
        }
}

int BIFMemoryPool::Fail() {
        TRACE_METHOD0("BIFMemoryPool::Fail");
        return hHeap==NULL;
}


void *BIFMemoryPool::Alloc(unsigned long bytes) {
        TRACE_METHOD0("BIFMemoryPool::Alloc");

        if(!hHeap) {
                LOG_TEXT("BIFMemoryPool::Alloc(): base==0");
                return 0;
        }


        LPVOID lpv;
        if((lpv=HeapAlloc(hHeap,0,bytes))==NULL) {
                NOTE0("BIFMemoryPool::Alloc: HeapAlloc failed");
                return 0;
        } else
                return lpv;
}

void BIFMemoryPool::Free(void *p) {
        TRACE_METHOD0("BIFMemoryPool::Free");

        if(!p)
                return; //this is not an error!
        if(!hHeap)
                return; //but I dont know if this should be an error

        if(!HeapFree(hHeap,0,p)) {
                NOTE0("BIFMemoryPool::Free: HeapFree failed");
                return;
        }
}
