#ifndef __BIFT_WAITTHING_H_INCLUDED
#define __BIFT_WAITTHING_H_INCLUDED

class BIFCLASS FThread;
class BIFCLASS FThreadManager;

class BIFCLASS FWaitThing {
        //a thing to wait for
protected:
        FWaitThing();
        virtual ~FWaitThing();

private:
        void AddWaiter(FThread *tp);
protected:
        void RemoveWaiter(FThread *tp);

        int GenericWait(long timeout=-1);

        virtual FThread *WakeOneWaiter(int retcode);
        virtual void WakeAllWaiters(int retcode);

        int MoreWaiters() const;

        //friend void FThread::terminateThread();
        //Recursive headers would be required to do the line above, so we
        //have to use the line below
        friend class FThread;
private:
        FThread *firstWaiter;
};


class BIFCLASS FExternalWaitThing : protected FWaitThing {
protected:
        FExternalWaitThing();
        ~FExternalWaitThing();

        virtual void Check() =0;        //check to if external event has happened
        virtual void Wait() =0; //wait until external event happens
        virtual void PollLoop(long pollEndTime) =0;     //poll until timeout or event happens
private:
        int AnyWaiters() const;
        friend class FThreadManager;
};


#endif  /* __BIFT_WAITTHING_H_INCLUDED */
