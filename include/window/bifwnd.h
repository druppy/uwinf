#if BIFWS_ == BIFWS_PM_
#  include <window\pm\bifwnd.h>
#elif BIFWS_ == BIFWS_WIN_
#  if BIFOS_ == BIFOS_WIN16_
#    include <window\win16\bifwnd.h>
#  else
#    include <window\win32\bifwnd.h>
#  endif
#elif BIFWS_ == BIFWS_WMS_
#  include <window\wms\bifwnd.h>
#else
#  error What?
#endif

