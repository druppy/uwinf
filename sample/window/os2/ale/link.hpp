#ifndef __LINK_H
#define __LINK_H

struct Link {
        int x,y;
        int cx,cy;

        enum referencetype {
                heading,
                footnote,
                launch,
                inform
        } reftype;
        char res[40];
        char refid[40];
        char database[40];
        char object[40];
        char data[40];
};

#endif

