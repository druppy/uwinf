#ifndef __BWWINTH_H_INCLUDED
#define __BWWINTH_H_INCLUDED

class BIFCLASS FWWindowThread : public FWindowThread {
public:
        FWWindowThread(unsigned stackNeeded=0)
          : FWindowThread(stackNeeded)
          {}
protected:
        int PreGo();
        int _PreGo();
        void PostGo();
        void _PostGo();
};

#endif

