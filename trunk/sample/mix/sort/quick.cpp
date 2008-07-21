#include "sort.h"

//The classical QuickSort
//Advantages: very small inner loop
//Disadvantages: Degrades to a O(N*N) in several cases
//Complexities:
//         Worst    Average    Best
//   Time: O(N*N)   O(N lg N)  O(N lg N)
//   Space O(N)     O(lg N)    O(lg N)


template<class T, class Comp, class Swapper>
unsigned QuickSortClass<T,Comp,Swapper>::partition(T *a, unsigned N)
{
        //choose a pivot element
        {
                int c=N/2;
                if(Comp::compare(a[c],a[0])>0)   Swapper::swap(a[c],a[0]);
                if(Comp::compare(a[0],a[N-1])>0) Swapper::swap(a[0],a[N-1]);
                if(Comp::compare(a[c],a[0])>0)   Swapper::swap(a[c],a[0]);
        }
        unsigned left=1, right=N-1;

        while(left<=right) {
                if(Comp::compare(a[left],a[0])<=0) {
                        left++;
                } else {
                        Swapper::swap(a[left],a[right]);
                        right--;
                }
        }
        Swapper::swap(a[0],a[left-1]);
        return left-1;
}



template<class T, class Comp, class Swapper>
void QuickSortClass<T,Comp,Swapper>::Sort(T *a, unsigned N)
{
        if(N<2) return;
        if(N==2) {
                if(Comp::compare(a[0],a[1])>0) {
                        Swapper::swap(a[0],a[1]);
                }
                return;
        }


        unsigned center=partition(a,N);

        Sort(a,center);
        Sort(a+center+1,N-center-1);
}

