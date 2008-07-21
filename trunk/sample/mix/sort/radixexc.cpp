#include "sort.h"

//Radix Exchange Sort
//This algorithm requires that the keys can be bitwise sorted. Think of this
//algorithm as a quicksort where the partition is guaranteed to be "good".
//Advantages: _very_ fast
//Disadvantages: The partitioning could be better
//Complexities:
//Worst, average and best is the same
//   Time: O(Nb)
//   Space: O(b)
//   Bit tests: O(Nb)
//   Swaps: O(Nb)
// (N = number of elements in array
// (b = number of bits in each element

template<class T, class Bits, class Swapper, unsigned maxb>
void RadixExchangeSortClass<T,Bits,Swapper,maxb>::Sort(T *a, unsigned N, unsigned bitno) {
	if(N>1) {
		unsigned z=0,o=N;
		while(z<o) {
			while(z<o && Bits::bits(a[z],bitno,1)==0)
				z++;
			while(z<o && Bits::bits(a[o-1],bitno,1)==1)
				o--;
			if(z<o) {
				Swapper::swap(a[z],a[o-1]);
				z++;
				o--;
			}
		}
		if(bitno>0) {
			Sort(a,z,bitno-1);
			Sort(a+z,N-o,bitno-1);
		}
	}
}
