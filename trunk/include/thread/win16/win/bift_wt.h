#ifndef __BIFT_WT_H_INCLUDED
#define __BIFT_WT_H_INCLUDED

class BIFCLASS FWindowThread : public virtual FThread {
public:
	FWindowThread(unsigned stackNeeded=8192);
	~FWindowThread();
#if defined(__BORLANDC__) && __BORLANDC__<=0x410
        //BC 3.1 has a bug in its vtable generation. This avoids it.
        void Go() {}
#endif
protected:
	virtual void MessageLoop();
public:
	BOOL GetMessage(MSG *pmsg);
};

#endif
