#ifndef __CLNCON_H
#define __CLNCON_H
//The same as os/2 bt included files is not
#include <ipc\win32\connection.h>

class FClientConnection : public FConnection {
protected:
        FClientConnection()
          : FConnection()
          {}
};

#ifdef BIFINCL_NPCLIENT
#  include <ipc\win32\npclient.h>
#endif
#ifdef BIFINCL_TCPCLIENT
#  include <ipc\win32\tcpclient.h>
#endif

#endif
