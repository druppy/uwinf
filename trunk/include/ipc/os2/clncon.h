#ifndef __CLNCON_H
#define __CLNCON_H
#include <ipc\os2\connection.h>

class FClientConnection : public FConnection {
protected:
        FClientConnection()
          : FConnection()
          {}
};

#ifdef BIFINCL_NPCLIENT
#  include <ipc\os2\npclient.h>
#endif
#ifdef BIFINCL_TCPCLIENT
#  include <ipc\os2\tcpclient.h>
#endif

#endif
