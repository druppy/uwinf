#ifndef __BIFT_WT_H_INCLUDED
#define __BIFT_WT_H_INCLUDED

class BIFCLASS FWindowThread : public virtual FThread {
        int rememberedQS;
protected:
        int queryQS() const { return rememberedQS; }
        WmsHAB hab;
        WmsHMQ hmq;
public:
        FWindowThread(unsigned stackNeeded=0, int queueSize=0);
        ~FWindowThread();

        WmsHAB GetHAB() { return hab; }
        WmsHMQ GetHMQ() { return hmq; }

protected:
        virtual void MessageLoop();

        int PreGo();
        int _PreGo();
        void PostGo();
        void _PostGo();
};

#endif
