#ifndef __BIFT_WT_H_INCLUDED
#define __BIFT_WT_H_INCLUDED

class BIFCLASS FWindowThread : public virtual FThread {
        int rememberedQS;
protected:
        int queryQS() const { return rememberedQS; }
        HAB hab;
        HMQ hmq;
public:
        FWindowThread(unsigned stackNeeded=0, int queueSize=0);
        ~FWindowThread();

        HAB GetHAB() { return hab; }
        HMQ GetHMQ() { return hmq; }

protected:
        virtual void MessageLoop();

        int PreGo();
        int _PreGo();
        void PostGo();
        void _PostGo();
};

#endif
