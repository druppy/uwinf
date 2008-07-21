/* BIF top-level include file */

#ifndef __BIF_WAS_HERE__
#  define __BIF_WAS_HERE__
   //detect OS and window system and include OS- and WS-specific headerfile(s)
#  include <bifdetos.h>
#  include <bifdetws.h>
#  if BIFOS_ == BIFOS_OS2_
#    include <bifimex.os2>
#    ifdef BIFINCL_THREAD
#      define INCL_DOSSEMAPHORES
#      define INCL_DOSPROCESS
#    endif
#    ifdef BIFINCL_SERVER
#      define INCL_DOSSEMAPHORES
#    endif
#    ifdef BIFINCL_WINDOW
#      if BIFWS_ == BIFWS_PM_
#        define INCL_DOSMODULEMGR
#        define INCL_WIN
#      else
#        define INCL_NOPMAPI
#      endif
#    endif
#    if BIFWS_ == BIFWS_WMS_
#      include <wms.hpp>
#    endif
#    include <os2.h>
#  elif BIFOS_ == BIFOS_WIN16_
#    include <bifimex.w16>
#    include <windows.h>
#  elif BIFOS_ == BIFOS_WIN32_
#    include <bifimex.w32>
#    include <windows.h>
#  elif (BIFOS_ == BIFOS_DOS16_) || (BIFOS_ == BIFOS_DOS32_)
#    include <bifimex.dos>
#  else
#    error What?
#  endif
#endif /*__BIF_WAS_HERE__*/


#ifdef BIFINCL_THREAD
#  include <thread\bifthrea.h>
#endif

#ifdef BIFINCL_WINDOW
#  include <window\bifwnd.h>
#endif

#if defined(BIFINCL_THREAD) && defined(BIFINCL_WINDOW)
#  include <mix\bifmix.h>
#endif

#if defined(BIFINCL_IPC)
#  include <ipc\bifipc.h>
#endif
