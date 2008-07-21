#ifndef __SORTMONITOR_H
#define __SORTMONITOR_H

#include "sort.h"
#include "common.h"

//class to monitor assignments to the elements being sorted
class MonitoredInt {
	int v;
public:
	MonitoredInt() {}
	MonitoredInt(int m) { v=m; }
	operator int() const { return v; }
	MonitoredInt& operator=(const MonitoredInt& m);
};


inline int isArrayElement(const MonitoredInt *a) {
	/* test wether or not a points to an elements in e[] */
	if(a<e || a>=e+N) return 0;	//in range?
	/* The above statements doesn't catch all cases in segmented memory
	 * where a compiler might just test the offset within the segments.
	 * According to the ANSI standard if a does not point to an element
	 * within e[] the comparison is undefined
	 */

	int n=int(a-e);		// compute index in e[]
	if(n<0 || (unsigned)n>=N) return 0;
	const MonitoredInt *p=e+n;	// if a points into e[] then it should be p
	if(a!=p) return 0;	// This statement should force the compiler to
				// do a full segment:offset comparison
	return 1;
}


extern unsigned long comps;
extern unsigned long swaps;
extern unsigned long assigns;


//Because we want to count the number of comparisons we create our own comperator
class SortComperator<MonitoredInt> {
public:
	static int compare(const MonitoredInt& e1, const MonitoredInt& e2) {
		comps++;
		return e1<e2 ? -1: e2<e1;
	}
};

//Because we want to count the number of swaps and show the progress to the user
//we create our own swapper
class SortSwapper<MonitoredInt> {
public:
	static void swap(MonitoredInt& e1, MonitoredInt& e2) {
		swaps++;

		MonitoredInt tmp=e1;
		e1=e2;
		e2=tmp;
	}
};


#endif
