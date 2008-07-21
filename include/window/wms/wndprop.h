#ifndef __WNDPROP_H_INCLUDED
#define __WNDPROP_H_INCLUDED

class WindowProperty {
        char *text;
        int textlen;

        char *data;
        int datalen;

        struct ppentry {
                int id;
                char *buf;
                int buflen;
                ppentry *next;
        };
        ppentry *firstpp;
public:
        WindowProperty();
        ~WindowProperty();

        int  GetTextLength() const;
        int  GetText(char *buf, int maxbuf) const;
        int  SetText(const char *buf);

        int  GetDataLength() const;
        int  GetData(void *buf, int maxbuf) const;
        int  SetData(const void *buf, int buflen);

        int  GetPresParamLength(int id) const;
        int  GetPresParam(int id, void *buf, int maxbuf) const;
        int  SetPresParam(int id, const void *buf, int buflen);
        void RemovePresParam(int id);
};

#endif

