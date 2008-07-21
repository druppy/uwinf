#include "sort.h"


//BatcherSort.
//Advantages: Guaranteed O(N lg N). Perfect for multi-programming
//Disadvantages: Hard to understand
//Complexities:
//         Worst     Average      Best
//   Time: O(N lg N)   O(N lg N)  O(N lg N)
//   Space O(lg N)     O(lg N)    O(lg N)

template<class T, class Comp, class Swapper>
void BatcherSortClass<T,Comp,Swapper>::_sort(T *a, unsigned N, unsigned skip, int oddEvenSort)
{
        if(skip>=N)
                return;        //arrays containing less than two elements are always sorted

        if(skip*2>=N) {
                //only two elements - a simple compare-swap
                if(Comp::compare(a[0],a[skip])>0) {
                        Swapper::swap(a[0],a[skip]);
                }
                return;
        }

        if(!oddEvenSort) {
                //split the array
                int pot2;
                //find a power of 2 that is not less than N
                for(pot2=1; pot2<(int)N; pot2*=2);

                int pot2halv = pot2/2;


                //the two halves
                _sort(a,          pot2halv,   skip, False);        //first half
                _sort(a+pot2halv, N-pot2halv, skip, False);        //second half
        }

        //sort the "even" elements
        _sort(a,      N,      skip*2, True);
        //sort the "odd" elements
        _sort(a+skip, N-skip, skip*2, True);

        //compare-swap the elements 2/3, 4/5, 6/7, ....
        for(int i=skip; i+skip<N; i+=skip*2) {
                if(Comp::compare(a[i],a[i+skip])>0) {
                        Swapper::swap(a[i],a[i+skip]);
                }
        }
}


template<class T, class Comp, class Swapper>
void BatcherSortClass<T,Comp,Swapper>::Sort(T *a, unsigned N) {
        _sort(a, N, 1, False);
}

