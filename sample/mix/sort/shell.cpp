#include "sort.h"

//ShellSort
//Advantages: Only a minor variation of selectionsort
//Disadvantages:
//Complexities: (using the sequence 1, 4, 13, 40, 121, ...)
//         Worst         Average    Best
//   Time: O(N sqr N)    ?          ?
//   Space O(1)          O(1)       O(1)
template<class T, class Comp>
void ShellSortClass<T,Comp>::Sort(T *a, unsigned N)
{
        unsigned h;
        for(h=1; h<N/9; h = h*3+1);        // sequence: 1,4,13,40,121,...

        for( ; h>0; h/=3) {
                for(unsigned i=h; i<N; i+=1) {
                        unsigned j=i;
                        T v=a[i];
                        while(j>=h && Comp::compare(a[j-h],v)>0) {
                                a[j]=a[j-h];
                                if(j!=0) j-=h; else break;
                        }
                        a[j]=v;
                }
        }
}
