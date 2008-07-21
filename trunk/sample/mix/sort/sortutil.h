#ifndef __SORTUTIL_H
#define __SORTUTIL_H

#include "common.h"

enum DataSetType {
	dst_random,
	dst_mostlysorted,
	dst_almostsorted,
	dst_sorted,
	dst_reverse,
	dst_smallrange,
	dst_min=dst_random,
	dst_max=dst_smallrange
};

const char *fillDataSet(DataSetType method, MonitoredInt a[], unsigned N);

#endif
