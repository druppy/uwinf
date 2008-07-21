#include "linka.hpp"

void LinkArray::remove(int i) {
        for(int j=i; j<c-1; j++)
                la[j]=la[j+1];
        c--;
}


