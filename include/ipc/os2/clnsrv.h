#ifdef BIFINCL_CLIENT
#  include <ipc\os2\clncon.h>
#endif

#ifdef BIFINCL_SERVER
#  define BIFINCL_THREAD
#  define BIFINCL_SEMAPHORES
#  include <thread\bifthread.h>
#  include <ipc\os2\connection.h>
#  include <ipc\os2\server.h>
#  ifdef BIFINCL_NPSERVER
#    include <ipc\os2\npserver.h>
#  endif
#  ifdef BIFINCL_TCPSERVER
#    include <ipc\os2\tcpserv.h>
#  endif
#endif
