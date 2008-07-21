#ifndef __LNOTE_HPP
#define __LNOTE_HPP

#include "link.hpp"

class LinkChangeNotifier {
public:
        virtual void ChangeLink(const Link&) =0;
};

#endif

