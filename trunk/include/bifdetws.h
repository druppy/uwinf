#ifndef __BIFDETWS_H
#define __BIFDETWS_H
//Detect which window system the user wants
//bifdetos.h must have been included ahead of the file

#define BIFWS_PM_       1
#define BIFWS_WMS_      2
#define BIFWS_WIN_      3

#if defined(BIFWINDOWSYSTEM_PM)
#  define BIFWS_ BIFWS_PM_
#elif defined(BIFWINDOWSYSTEM_WMS)
#  define BIFWS_ BIFWS_WMS_
#elif defined(BIFWINDOWSYSTEM_WINDOWS)
#  define BIFWS_ BIFWS_WIN_
#else
#  if BIFOS_ == BIFOS_OS2_
//   default is Presentation manager
#    define BIFWS_ BIFWS_PM_
#  elif BIFOS_ == BIFOS_WIN16_
//   Default (and only) is Windows' window system
#    define BIFWS_ BIFWS_WIN_
#  elif BIFOS_ == BIFOS_WIN32_
//   Default is Windows's window system
#    define BIFWS_ BIFWS_WIN_
#  elif BIFOS_ == BIFOS_DOS16_
//   default is WMS
#    define BIFWS_ BIFWS_WMS_
#  elif BIFOS_ == BIFOS_DOS32_
//   default is WMS
#    define BIFWS_ BIFWS_WMS_
#  else
#    error Could not guess window system
#  endif
#endif

//now check for invalid combinations
#if BIFOS_ == BIFOS_WIN16_
#  if BIFWS_ != BIFWS_WIN_
#    error Only Windows window system can be used under Windows 3.x
#  endif
#elif BIFOS_ == BIFOS_WIN32_
#  if (BIFWS_ != BIFWS_WIN_) && (BIFWS_ != BIFWS_WMS_)
#    error Only Windows window system and WMS can be used under Win32
#  endif
#elif BIFOS_ == BIFOS_OS2_
#  if (BIFWS_ != BIFWS_PM_) && (BIFWS_ != BIFWS_WMS_)
#    error Only Presentation Manager and WMS can be used under OS/2
#  endif
#elif BIFOS_ == BIFOS_DOS16_
#  if BIFWS_ != BIFWS_WMS_
#    error Only WMS can by used under 16-bit DOS
#  endif
#elif BIFOS_ == BIFOS_DOS32_
#  if BIFWS_ != BIFWS_WMS_
#    error Only WMS can by used under 32-bit DOS
#  endif
#else
#  error What?
#endif

#endif /*__BIFDETWS_H*/
