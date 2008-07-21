#ifndef __SORT_H
#define __SORT_H


template<class T>
class SortComperator {
public:
        static int compare(const T& e1, const T& e2) {
                return e1<e2 ? -1: e2<e1;
        }
};



template<class T>
class SortSwapper {
public:
        static void swap(T& e1, T& e2) {
                T tmp=e1;
                e1=e2;
                e2=tmp;
        }
};

template<class T>
class SortBits {
public:
        static unsigned bits(T& e, unsigned l, unsigned b) {
                return (((unsigned)(e))>>(l-b+1)) & ~(~0<<b);
        }
};

template<class T, class Comp, class Swapper>
class QuickSortClass {
        static unsigned partition(T *a, unsigned N);
public:
        static void Sort(T *a, unsigned N);
};


template<class T, class Comp, class Swapper>
class BatcherSortClass {
private:
        static void _sort(T *a, unsigned N, unsigned skip, int oddEvenSort);
public:
        static void Sort(T *a, unsigned N);
};


template<class T, class Comp>
class ShellSortClass {
public:
        static void Sort(T *a, unsigned N);
};


template<class T, class Comp, class Swapper>
class CombSortClass {
        static const double SHRINKFACTOR;
public:
        static void Sort(T *a, unsigned N);
};


template<class T, class Comp, class Swapper>
class HeapSortClass {
        static unsigned parent(unsigned son) {
                //return (son+1)/2       -1;
                return ((son+1)>>1)-1;
        }

        static unsigned lson(unsigned parent) {
                //return (parent+1)*2    -1;
                return (parent<<1)+1;
        }

        static unsigned rson(unsigned parent) {
                //return (parent+1)*2+1  -1;
                return (parent<<1)+2;
        }
public:
        static void Sort(T *a, unsigned N);
};


template<class T, class Bits, class Swapper, unsigned maxb>
class RadixExchangeSortClass {
public:
        static void Sort(T e[], unsigned N, unsigned bitno=maxb-1);
};

#endif
