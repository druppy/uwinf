#include <bifdetos.h>

#if BIFOS_ == BIFOS_OS2_
#  include <ipc\os2\bifipc.h>
#elif BIFOS_ == BIFOS_WIN16_
#  include <ipc\win16\bifipc.h>
#elif BIFOS_ == BIFOS_WIN32_
#  include <ipc\win32\bifipc.h>
#elif (BIFOS_ == BIFOS_DOS16_) || (BIFOS_ == BIFOS_DOS32_)
#  include <ipc\dos1632\bifipc.h>
#else
#  error What?
#endif

