#include "sortutil.h"
#include "sortmoni.h"
#include <stdlib.h>

#ifdef __BORLANDC__
//tell BC++ to ignore 'condition is always true/false'
#pragma warn -ccc
#elif defined(__WATCOMC__)
//tell Watcom the same
#pragma warning 354 9
#endif

const char *fillDataSet(DataSetType method, MonitoredInt a[], unsigned N) {
        unsigned i;
        const char *name;

        displayDisabled++;
        switch(method) {
                case dst_random:
                        // random
                        srand(1);
                        for(i=0; i<N; i++)
                                a[i]=rand()%MaxValue;
                        name="Random            ";
                        break;

                case dst_mostlysorted:
                        // mostly sorted  (sorted with major shuffle)
                        fillDataSet(dst_sorted,a,N);
                        for(i=0; i<N/2; i++) {
                                unsigned j1=rand()%N;
                                unsigned j2=rand()%N;
                                MonitoredInt tmp=a[j1];
                                a[j1]=e[j2];
                                a[j2]=tmp;
                        }
                        name="Mostly sorted     ";
                        break;

                case dst_almostsorted:
                        // mostly sorted 2 (sorted with localized shuffle)
                        fillDataSet(dst_sorted,a,N);

                        for(i=0; i<N; i++) {
                                unsigned j1=rand()%N;
                                unsigned j2=rand()%(N/10)-5;
                                if(/*j1+j2>=0 &&*/ j1+j2<N) {
                                        MonitoredInt tmp=a[j1];
                                        a[j1]=e[j1+j2];
                                        a[j1+j2]=tmp;
                                }
                        }
                        name="Almost sorted     ";
                        break;

                case dst_sorted:
                        //already sorted
                        for(i=0; i<N; i++)
                                a[i] = (int)((double)(MaxValue)/N)*i;
                        name="Sorted            ";
                        break;

                case dst_reverse:
                        //reverse sorted
                        for(i=0; i<N; i++)
                                a[i] = (int)((double)(MaxValue)/N)*(N-i);
                        name="Reverse sorted    ";
                        break;

                case dst_smallrange:
                        //small value range
                        int mv = (MaxValue/2)>(int)(N/4)?(N/4):(MaxValue/2);

                        for(i=0; i<N; i++)
                                a[i] = rand()%mv;
                        name="Small value range ";
                        break;
        }
        displayDisabled--;
        showArray(a,N);

        return name;
}

