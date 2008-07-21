#ifndef __LINKA_HPP
#define __LINKA_HPP

#include "link.hpp"

class LinkArray {
        Link la[50];
        int c;
public:
        int count() const { return c; }
        Link &operator[](int i) { return la[i]; }
        const Link &operator[](int i) const { return la[i]; }
        int add(const Link &l) {
                la[c]=l;
                return c++;
        }
        void remove(int i);
        void clear() { c=0; }
};

#endif

