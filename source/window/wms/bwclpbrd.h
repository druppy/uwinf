#ifndef __BWCLPBRD_H_INCLUDED
#define __BWCLPBRD_H_INCLUDED

class WMSClipBoard {
public:
        WMSClipBoard();
        ~WMSClipBoard();

        int SetText(const char *buf, unsigned bytes);
        int GetText(char *buf, unsigned maxbuf, unsigned *bytes);
private:
        int defSetText(const char *buf, unsigned bytes);
        int defGetText(char *buf, unsigned maxbuf, unsigned *bytes);

        int useInternalClipboard();
};

extern WMSClipBoard WmsClipboard;
#endif
