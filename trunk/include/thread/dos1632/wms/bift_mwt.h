#ifndef __BIFT_MWT_H_INCLUDED
#define __BIFT_MWT_H_INCLUDED

class BIFCLASS FMainWindowThread : public virtual FThread,
                                   public FMainThread,
                                   public FWindowThread 
{
public:
      FMainWindowThread(int argc, char *argv[], int queueSize=0);
      ~FMainWindowThread();
};

#endif
