#include "menuitem.hpp"
#include <mem.h>

MenuItemList::MenuItemList() {
        items=0;
}

MenuItemList::~MenuItemList() {
        //nothing
}

void MenuItemList::insert(int index, const MenuItem& mi) {
        //menuitems are just structs, so memmov() 'em
        memmove(item+index+1,item+index,sizeof(MenuItem)*(items-index));
        item[index]=mi;
        items++;
}

void MenuItemList::remove(int index) {
        //menuitems are just structs, so memmov() 'em
        if(index<items-1)
                memmove(item+index,item+index+1,sizeof(MenuItem)*(items-index-1));
        items--;
}
