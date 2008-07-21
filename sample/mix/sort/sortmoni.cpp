#include "sortmoni.h"

//--------------------------------------------------------------
// Globals for counting swaps, comparisons and assignments
unsigned long comps;
unsigned long swaps;
unsigned long assigns;


MonitoredInt& MonitoredInt::operator=(const MonitoredInt& m) {
        v=m.v;
        if(isArrayElement(this)) showArray(this,1);
        assigns++;
        return *this;
}
