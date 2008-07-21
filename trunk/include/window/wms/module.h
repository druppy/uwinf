#ifndef __MODULE_H_INCLUDED
#define __MODULE_H_INCLUDED

class ResourceTable;
class DialogResource;
class BIFCLASS FModule {
        FModule(const FModule&);                //don't copy
        FModule& operator=(const FModule&);     //don't copy
        ResourceTable *restbl;
public:
        FModule();
        virtual ~FModule();

        int Fail() const {
                return 0;
        }

        const void *LoadIcon(int resID);
        const void *LoadPointer(int resID);
        const void *LoadMenu(int resID);
        const void *LoadAccelTable(int resID);
        const DialogResource *FindDialogResource(int resID);

        int LoadString(int resID, char *buf, int maxbuf);
};

#endif /*__BW_MODULE_H_INCLUDED*/

