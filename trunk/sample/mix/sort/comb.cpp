#include "sort.h"

//CombSort
//Advantages: In some cases it can beat QuickSort. It is very fast sorting an
//            already reverse-sorted array.
//Disadvantages: A bad sequence could turn up for some N. But this is unlikely.
//Complexities:
//         Worst    Average    Best
//   Time: O(N*N)   *          O(N)
//   Space O(1)     O(1)       O(1)
//
//* The average time complexity has not yet been analyzed, but it seems that
//  it is around O(N lg N) and O(N sqr N)


template<class T, class Comp, class Swapper>
const double CombSortClass<T,Comp,Swapper>::SHRINKFACTOR=1.3;

template<class T, class Comp, class Swapper>
void CombSortClass<T,Comp,Swapper>::Sort(T *a, unsigned N)
{
        int finished;
        unsigned i,top,gap;

        gap=N;
        do {
                gap = (unsigned)(gap/SHRINKFACTOR);
                switch(gap) {
                        case 0:
                                gap=1;
                                break;
                        case 9:
                        case 10:
                                gap=11;        //this makes it a CombSort
                                break;
                        default:
                                break;
                }

                finished=1;

                top=N-gap;
                for(i=0; i<top; i++) {
                        unsigned j=i+gap;
                        if(Comp::compare(a[i],a[j])>0) {
                                Swapper::swap(a[i],a[j]);
                                finished=0;
                        }
                }
        } while(gap>1 || !finished);
}
