#ifndef __BIFDETOS_H
#define __BIFDETOS_H
// Detect which target operating system we are compiling for

#define BIFOS_OS2_   1
#define BIFOS_WIN16_ 2
#define BIFOS_WIN32_ 3
#define BIFOS_DOS16_ 4
#define BIFOS_DOS32_ 5

#if defined(_Windows) || defined(__WINDOWS__) || defined(_WINDOWS) || defined(__NT__) || defined(_WIN32)
// Some sort of windows. Currently Watcoms win386 windows extender is
// not supported.
#  if defined(__WIN32__) || defined(__WINNT__) || defined(__FLAT__) || defined(M_I386) || defined(__NT__) || defined(_WIN32)
//   Windows NT or Win32s - 32-bit
#    define BIFOS_ BIFOS_WIN32_
#  else
//   Windows 3.x 16-bit
#    define BIFOS_ BIFOS_WIN16_
#  endif
#elif defined(__OS2__) || defined(OS2)
#  define BIFOS_ BIFOS_OS2_
#elif defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
#  if defined(__FLAT__) || defined(__DPMI32__)
//   DOS 32-bit extender
#    define BIFOS_ BIFOS_DOS32_
#  elif defined(__DPMI16__)
#    error 16-bit DPMI not supported
#  else
//   DOS 16-bit real mode
#    define BIFOS_ BIFOS_DOS16_
#  endif
#else
#  error Could not detect target operating system
#endif

#endif /*__BIFDETOS_H*/
