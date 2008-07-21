#define BIFINCL_WINDOW
#define BIFINCL_RESOURCETEMPLATE
#include <bif.h>
#include <string.h>

//The global resource table
ResourceTable resource_table_;

//The resources table must have static trivial members because we cannot 
//guarantee that it is initialized before the resource registrations
//No thread synchronization is done because resources are registered at 
//startup when there are only one thread
ResourceTable::entry *ResourceTable::e[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

ResourceTable::ResourceTable() {
        //not initialization is required
}

ResourceTable::~ResourceTable() {
}

int ResourceTable::Register(unsigned type, unsigned id, const void *r) {
        unsigned i= (type+id)%13;
        for(entry *p=e[i]; p; p=p->next)
                if(p->type==type && p->id==id)
                        return -1; //already there
        p = new entry;
        if(!p) return -1;
        p->type=type;
        p->id=id;
        p->r=r;
        p->next=e[i];
        e[i]=p;
        return 0;
}

const void *ResourceTable::Find(unsigned type, unsigned id) {
        unsigned i= (type+id)%13;
        for(entry *p=e[i]; p; p=p->next)
                if(p->type==type && p->id==id)
                        return p->r;
        return 0;
}

//
static void split(char *s, char *prop[], int *props) {
        //split a dialog setup string into "prop[=value]" strings
        int ps=0;
        while(*s==' ') s++;
        while(*s) {
                char *p=s;
                int insideQuote=0;
                while(*p && (*p!=' ' || insideQuote)) {
                        if(*p=='"') insideQuote=!insideQuote;
                        p++;
                }
                prop[ps] = s;
                if(*p) {
                        *p ='\0';
                        p++;
                        while(*p==' ') p++;
                }
                s=p;
                ps++;
        }
        *props = ps;
}

static void vsplit(char *prop[], int ps, char *value[]) {
        //split array of "prop[=value]" into rpop+value strings
        while(ps) {
                char *p=*prop;
                while(*p && *p!='=') p++;
                if(*p) {
                        *p='\0';
                        p++;
                        if(*p=='"') {
                                *value=p+1;
                                while(*p) p++;
                                if(p[-1]=='"') p[-1]='\0';
                        } else
                                *value=p;
                } else
                        *value=0;

                prop++;
                value++;
                ps--;
        }
}

#define MAXITEMS 20
Bool DialogResource::SplitSetup(const char *setup, int *props, char ***name, char ***value) {
        *name = new char*[MAXITEMS];
        if(!*name) return False;
        *value = new char*[MAXITEMS];
        if(!*value) {
                delete[] *name;
                return False;
        }
        int slen = strlen(setup);
        char *workarea = new char[slen+1];
        if(!workarea) {
                delete[] *name;
                delete[] *value;
                return False;
        }
        strcpy(workarea,setup);
        split(workarea,*name,props);
        vsplit(*name,*props,*value);
        return True;
}

void DialogResource::DeallocateSetup(char **name, char **value) {
        delete[] name;
        delete[] value;
}

