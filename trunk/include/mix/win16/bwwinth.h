#ifndef __BWWINTH_H_INCLUDED
#define __BWWINTH_H_INCLUDED

class BIFCLASS FWWindowThread : public FWindowThread {
public:
        FWWindowThread(unsigned stackNeeded=8192)
          : FWindowThread(stackNeeded)
          {}
};

#endif

