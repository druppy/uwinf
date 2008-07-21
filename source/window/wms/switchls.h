#ifndef __SWITCHLS_H_INCLUDED
#define __SWITCHLS_H_INCLUDED

class BIFCLASS FSwitchList {
public:
        struct SwitchEntry {
                FWnd *pwnd;
                Bool visible;
                Bool jumpable;
                char title[160];
        };

        Bool AddEntry(const SwitchEntry *entry);
        Bool ChangeEntry(const SwitchEntry *entry);
        Bool QueryEntry(FWnd *pwnd, SwitchEntry *entry);
        Bool DeleteEntry(FWnd *pwnd);
};

FSwitchList * BIFFUNCTION GetSwitchList();

#endif

