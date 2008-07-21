#if BIFWS_ == BIFWS_PM_
#  include <mix\os2\pm\bifmix.h>
#elif BIFWS_ == BIFWS_WMS_
#  include <mix\os2\wms\bifmix.h>
#else
#  error What?
#endif
