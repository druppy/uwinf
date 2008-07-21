#ifndef __MENUITEM_HPP
#define __MENUITEM_HPP

#define MAXITEMTEXT        128
#define MAXITEMID        80
struct MenuItem {
        enum {
                et_item,
                et_submenu,
                et_endsubmenu
        } EditType;

        char ItemText[MAXITEMTEXT];
        char ItemID[MAXITEMID];
        enum {
                it_text,
                it_bitmap,
                it_separator,
                it_ownerdraw
        } ItemType;
        int Static;
        int Help;
        int SysCommand;
        enum {
                ib_nobreak,
                ib_break,
                ib_breakseparator,
                ib_buttonseparator
        } ItemBreak;
        int Checked;
        int Disabled;
        int Framed;
        int NoDismiss;
};


#define MAXITEM        100

class MenuItemList {
public:
        MenuItemList();
        ~MenuItemList();

        void insert(int index, const MenuItem& mi);
        MenuItem& operator[](int index) { return item[index]; }
        const MenuItem& Get(int index) const { return item[index]; }
        void remove(int index);
        int count() const { return items; }
        void clear() { items=0; }
private:
        MenuItem item[MAXITEM];
        int items;
};

#endif

