#include "sort.h"

//HeapSort
//Advantages: Guaranteed O(N lg N)
//Disadvantages: Twice as slow as QuickSort on the average
//Complexities:
//         Worst      Average    Best
//   Time: O(N lg N)  O(N lg N)  O(N lg N)
//   Space O(1)       O(1)       O(1)
template<class T, class Comp, class Swapper>
void HeapSortClass<T,Comp,Swapper>::Sort(T *a, unsigned N) {
        unsigned j;

        //build heap
        for(j=0; j<N; j++) {
                unsigned i=j;

                while(i>0 && Comp::compare(a[parent(i)],a[i])<0) {
                        Swapper::swap(a[parent(i)],a[i]);

                        i=parent(i);
                }
        }


        //unbuild the heap
        for(j=N-1; j>0; j--) {
                Swapper::swap(a[0],a[j]);

                unsigned p=0;
                while(p<j) {
                        unsigned l=lson(p);
                        unsigned r=rson(p);
                        if(l>=j) break;
                        if(r>=j) {
                                if(Comp::compare(a[l],a[p])>0) {
                                        Swapper::swap(a[l],a[p]);
                                        p=l;
                                } else
                                        break;
                        } else {
                                if(Comp::compare(a[l],a[r])>0) {
                                        if(Comp::compare(a[l],a[p])>0) {
                                                Swapper::swap(a[l],a[p]);
                                                p=l;
                                        } else
                                                break;
                                } else {
                                        if(Comp::compare(a[r],a[p])>0) {
                                                Swapper::swap(a[r],a[p]);
                                                p=r;
                                        } else
                                                break;
                                }
                        }
                }
        }
}
