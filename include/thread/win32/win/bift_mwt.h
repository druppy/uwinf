#ifndef __BIFT_MWT_H_INCLUDED
#define __BIFT_MWT_H_INCLUDED

#ifdef _MSC_VER
   // We know that Go() and Run are inherited via dominance
#  pragma warning(disable:4250)
#endif

class BIFCLASS FMainWindowThread : public virtual FThread,
                                   public FMainThread,
                                   public FWindowThread 
{
public:
      FMainWindowThread(int argc, char *argv[]);
      ~FMainWindowThread();
};

#endif
