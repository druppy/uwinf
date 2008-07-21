#ifndef __RESTEMPL_H_INCLUDED
#define __RESTEMPL_H_INCLUDED

class ResourceTable {
        struct entry {
                unsigned type;
                unsigned id;
                const void *r;
                entry *next;
        };
static  entry *e[13];
public:
        ResourceTable();
        ~ResourceTable();

        int Register(unsigned type, unsigned id, const void *r);
        const void *Find(unsigned type, unsigned id);
};

extern ResourceTable resource_table_;

class ResourceRegistration {
public:
        ResourceRegistration(unsigned type, unsigned id, const void *r)
          { resource_table_.Register(type,id,r); }
};

//predefined resource types
#define RESTYPE_STRING          0x0001
#define RESTYPE_DIALOG          0x0002
#define RESTYPE_POINTER         0x0003
#define RESTYPE_ICON            0x0004
#define RESTYPE_MENU            0x0005
#define RESTYPE_ACCELTABLE      0x0006

//string resource
#define STRING(id,s) ResourceRegistration stringresource_##id(RESTYPE_STRING,id,s);


//dialog resource
#define DIS_ITEM 0
#define DIS_END 1
struct DialogItemTemplate {
        int dis;
        int ID;
        const char *setup;
};

class DialogResource : public ResourceRegistration {
        const char *setup;
        const DialogItemTemplate *dlgitems;
public: DialogResource(int resId, const char *s, const DialogItemTemplate *t)
          : ResourceRegistration(RESTYPE_DIALOG,resId,this), setup(s), dlgitems(t)
          {}
        const char *QuerySetup() const { return setup; }
        const DialogItemTemplate *QueryDlgItems() const { return dlgitems; }
static Bool SplitSetup(const char *setup, int *props, char ***name, char ***value);
static void DeallocateSetup(char **name, char **value);
};

#define DIALOG(id,setup)                                                \
extern DialogItemTemplate dialogtemplate_##id[];                        \
DialogResource dialogresource_##id(id,setup,dialogtemplate_##id);\
DialogItemTemplate dialogtemplate_##id[] = {

#define DLGITEM(id,setup)                                               \
{DIS_ITEM,id,setup},

#define ENDDIALOG                                                       \
{DIS_END,0,0}                                                           \
};



//pointer resource (textmode only)
struct PointerTemplate {
        char char_xor;
        char char_and;
        char attr_xor;
        char attr_and;
};
class PointerResource : public ResourceRegistration {
public: PointerResource(int id, const PointerTemplate *t)
          : ResourceRegistration(RESTYPE_POINTER,id,t)
          {}
};

#define POINTER(id)                                                     \
extern PointerTemplate pointertemplate_##id;                            \
PointerResource pointerresource_##id(id,&pointertemplate_##id);         \
PointerTemplate pointertemplate_##id = {

#define POINTERSHAPE(cx,ca,ax,aa)                                       \
cx,ca,ax,aa

#define ENDPOINTER                                                      \
};


//Menu template
#define _MIS_ITEM 1
#define _MIS_SUBMENU 2
#define _MIS_END 3
struct MenuItemTemplate {
        int mis;
        int ID;
        const char *setup;
};
class MenuResource : public ResourceRegistration {
public: MenuResource(int id, const MenuItemTemplate *t)
          : ResourceRegistration(RESTYPE_MENU,id,t)
          {}
};
#define MENU(id)                                                        \
extern MenuItemTemplate menutemplate_##id[];                            \
MenuResource menuresource_##id(id,menutemplate_##id);                   \
MenuItemTemplate menutemplate_##id[] = {

#define MENUITEM(id,setup)                                              \
{_MIS_ITEM,id,setup},

#define SUBMENU(id,setup)                                               \
{_MIS_SUBMENU,id,setup},

#define ENDMENU                                                         \
{_MIS_END,0,0}                                                          \
};



//accelerator table
#define UPDOWN 0
#define UP     1
#define DOWN   2
struct AcceleratorEntry {
        int ID;
        WmsMSG msg;
        Bool isKey;
        char theChar;
        uint8 lk;
        unsigned shiftFlag;
        unsigned ctrlFlag;
        unsigned altFlag;
};
class AccelResource : public ResourceRegistration {
public: AccelResource(int id, const AcceleratorEntry r[])
          : ResourceRegistration(RESTYPE_ACCELTABLE,id,r)
          {}
};
#define ACCELTABLE(id)                                                  \
extern AcceleratorEntry acceltable_##id[];                              \
AccelResource accelinstance_##id(id,acceltable_##id);                   \
AcceleratorEntry acceltable_##id[] = {

#define ACCELCHAR(id,msg,c,shift,ctrl,alt)                              \
{id,WM_##msg,False,c,0,shift,ctrl,alt},

#define ACCELKEY(id,msg,k,shift,ctrl,alt)                               \
{id,WM_##msg,True,' ',(uint8)k,shift,ctrl,alt},

#define ENDACCELTABLE                                                   \
{0,0,False,0,0,0,0,0}                                                   \
};

#endif
