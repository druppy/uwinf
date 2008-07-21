//advanced handlers

#ifdef BIFINCL_ALLADVANCEDHANDLERS
#  define BIFINCL_SCROLLHANDLER
#  define BIFINCL_CSIHANDLERS
#  define BIFINCL_SIZERESTRICTIONHANDLERS
#endif

//fan-out include
#if defined(BIFINCL_SCROLLHANDLER) && !defined(__SCROLL_H_INCLUDED)
#  include <window\wms\scroll.h>
#endif

#if defined(BIFINCL_CSIHANDLERS) && !defined(__BW_CSI_H_INCLUDED)
#  include <window\wms\csi.h>
#endif

#if defined(BIFINCL_SIZERESTRICTIONHANDLERS) && !defined(__SIZEREST_H_INCLUDED)
#  include <window\wms\sizerest.h>
#endif
