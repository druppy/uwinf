#ifndef __BIFMEM_H
#define __BIFMEM_H

class BIFMemoryPool {
        HANDLE hHeap;
public:
        BIFMemoryPool(unsigned long poolSize);
        ~BIFMemoryPool();

        int Fail();

        void *Alloc(unsigned long bytes);
        void Free(void *p);
};

#endif
