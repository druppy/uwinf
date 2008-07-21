#ifndef __SYSVALUE_HPP
#define __SYSVALUE_HPP

uint32  WmsQuerySysValue(WmsHWND desktop, int iSysValue);
Bool    WmsSetSysValue(WmsHWND desktop, int iSysValue, uint32 iValue);

#define SV_DBLCLKTIME              1
#define SV_CXDBLCLK                2
#define SV_CYDBLCLK                3
#define SV_CXSCREEN                20
#define SV_CYSCREEN                21

#endif
