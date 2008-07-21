#include <bifdetos.h>

#if BIFOS_ == BIFOS_OS2_
#  include <thread\os2\bifthrea.h>
#elif BIFOS_ == BIFOS_WIN16_
#  include <thread\win16\bifthrea.h>
#elif BIFOS_ == BIFOS_WIN32_
#  include <thread\win32\bifthrea.h>
#elif (BIFOS_ == BIFOS_DOS16_) || (BIFOS_ == BIFOS_DOS32_)
#  include <thread\dos1632\bifthrea.h>
#else
#  error What?
#endif

