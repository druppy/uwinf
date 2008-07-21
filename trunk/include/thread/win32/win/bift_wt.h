#ifndef __BIFT_WT_H_INCLUDED
#define __BIFT_WT_H_INCLUDED

class BIFCLASS FWindowThread : public virtual FThread {
protected:
public:
        FWindowThread(unsigned stackNeeded=0);
        ~FWindowThread();

protected:
        virtual void MessageLoop();
};

#endif
