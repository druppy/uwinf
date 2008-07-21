#ifdef BIFINCL_CLIENT
#  include <ipc\win32\clncon.h>
#endif

#ifdef BIFINCL_SERVER
#  define BIFINCL_THREAD
#  define BIFINCL_SEMAPHORES
#  include <thread\bifthread.h>
#  include <ipc\win32\connection.h>
#  include <ipc\win32\server.h>
#  ifdef BIFINCL_NPSERVER
#    include <ipc\win32\npserver.h>
#  endif
#  ifdef BIFINCL_TCPSERVER
#    include <ipc\win32\tcpserv.h>
#  endif
#endif
