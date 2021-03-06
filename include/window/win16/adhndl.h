//advanced handlers

#ifdef BIFINCL_ALLADVANCEDHANDLERS
#  define BIFINCL_SCROLLHANDLER
#  define BIFINCL_CSIHANDLERS
#endif

//fan-out include
#if defined(BIFINCL_SCROLLHANDLER) && !defined(__SCROLL_H_INCLUDED)
#  include <window\win16\scroll.h>
#endif

#if defined(BIFINCL_CSIHANDLERS) && !defined(__CSI_H_INCLUDED)
#  include <window\win16\csi.h>
#endif

#if defined(BIFINCL_SIZERESTRICTIONHANDLERS) && !defined(__SIZEREST_H_INCLUDED)
#  include <window\win16\sizerest.h>
#endif

