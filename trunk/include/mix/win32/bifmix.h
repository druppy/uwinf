#if BIFWS_ == BIFWS_WIN_
#  include <mix\win32\win\bifmix.h>
#elif BIFWS_ == BIFWS_WMS_
#  include <mix\win32\wms\bifmix.h>
#else
#  error What?
#endif
