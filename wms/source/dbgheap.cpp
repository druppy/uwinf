#define INCL_DOSMEMMGR
#include <os2.h>

#include <stdlib.h>
#include <string.h>

class DebugHeap {
public:
        static void *alloc(unsigned size);
        static void free(void *p);
};

struct chunk_header {
        unsigned chunk_size;            //# of comitted bytes
        unsigned block_size;            //heap block size (requested size)
        unsigned block_offset;          //offset from start of block to (user-)pointer
};

void *DebugHeap::alloc(unsigned size) {
        //round up to nearest 4KB block size
        unsigned chunk_size=((size+sizeof(chunk_header)+0x0FFF)&~0x0FFF);
        unsigned alloc_size=chunk_size+4096;

        APIRET rc;

        //allocate the memory
        char *pchunk;
        rc=DosAllocMem((PPVOID)&pchunk, alloc_size, PAG_COMMIT|PAG_READ|PAG_WRITE);
        if(rc!=0) return 0;

        //decommit the last page
        rc=DosSetMem(pchunk+chunk_size, 4096, PAG_DECOMMIT);
        if(rc!=0) {
                DosFreeMem(pchunk);
                return 0;
        }

        //set initial bytes of block to something
        memset(pchunk,0xff,alloc_size-4096);

        //setup the chunk housekeeping structure
        chunk_header *chp=(chunk_header *)pchunk;
        chp->chunk_size   = chunk_size;
        chp->block_size   = size;
        chp->block_offset = chp->chunk_size-chp->block_size;

        return pchunk+chp->block_offset;
}

void DebugHeap::free(void *p) {
        char *pchunk=(char*)(((unsigned long)p)&0xFFFFE000);
        chunk_header *chp=(chunk_header *)pchunk;

        //check for overwrite of the chunk header structure
        if(chp->block_offset != chp->chunk_size-chp->block_size) {
                exit(0);
        }

        //check for correct pointer
        if(p != pchunk+chp->block_offset) {
                exit(0);
        }

        DosFreeMem(pchunk);
}

void *operator new(size_t bytes) {
        return DebugHeap::alloc(bytes);
}

void operator delete(void *p) {
        if(p) DebugHeap::free(p);
}

void *operator new[](size_t bytes) {
        return DebugHeap::alloc(bytes);
}

void operator delete[](void *p) {
        if(p) DebugHeap::free(p);
}

#if 0
int main() {
        char *p=(char*)DebugHeap::alloc(14);
        strcpy(p,"Hello world");                  //ok
        strcpy(p,"Hello my friend. What's your name?"); //error
        DebugHeap::free(p);
        return 0;
}
#endif
