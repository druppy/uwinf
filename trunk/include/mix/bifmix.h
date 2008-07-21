#if BIFOS_ == BIFOS_OS2_
#  include <mix\os2\bifmix.h>
#elif BIFOS_ == BIFOS_WIN16_
#  include <mix\win16\bifmix.h>
#elif BIFOS_ == BIFOS_WIN32_
#  include <mix\win32\bifmix.h>
#elif (BIFOS_ == BIFOS_DOS16_) || (BIFOS_ == BIFOS_DOS32_)
#  include <mix\dos1632\bifmix.h>
#else
#  error What?
#endif
