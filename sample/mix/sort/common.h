#ifndef __COMMON_H
#define __COMMON_H

class MonitoredInt;

extern MonitoredInt e[];
extern const unsigned N;
extern const int MaxValue;

extern int displayDisabled;

//this function has to be provided elsewhere
extern void showArray(const MonitoredInt *a, int n);

#endif
