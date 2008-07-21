#define BIFINCL_WINDOW
#include <bif.h>
#include <string.h>

WindowProperty::WindowProperty()
  : text(0),
    data(0),
    datalen(0),
    firstpp(0)
{}

WindowProperty::~WindowProperty() {
        delete[] text;
        delete[] data;
        ppentry *p=firstpp;
        while(p) {
                ppentry *n=p->next;
                delete[] p->buf;
                delete p;
                p=n;
        }
}

int  WindowProperty::GetTextLength() const {
        return textlen;
}

int  WindowProperty::GetText(char *buf, int maxbuf) const {
        if(text) {
                if(textlen<maxbuf) {
                        memcpy(buf,text,textlen);
                        buf[textlen]='\0';
                        return textlen;
                } else {
                        memcpy(buf,text,maxbuf-1);
                        buf[maxbuf-1]='\0';
                        return maxbuf-1;
                }
        } else {
                *buf='\0';
                return 1;
        }
}

int  WindowProperty::SetText(const char *buf) {
        if(!buf) return -1;
        int blen=strlen(buf);
        char *p=new char[blen];
        if(p) {
                delete[] text;
                text=p;
                textlen=blen;
                memcpy(text,buf,blen);
                return 0;
        } else
                return -1;
}

int  WindowProperty::GetDataLength() const {
        return datalen;
}

int  WindowProperty::GetData(void *buf, int maxbuf) const {
        if(datalen<=maxbuf) {
                memcpy(buf,data,datalen);
                return datalen;
        } else
                return 0;
}

int  WindowProperty::SetData(const void *buf, int buflen) {
        if(buf) {
                char *p=new char[buflen];
                if(p) {
                        delete data;
                        data=p;
                        datalen=buflen;
                        return 0;
                } else
                        return -1;
        } else {
                delete[] data;
                data=0;
                datalen=0;
                return 0;
        }
}

int WindowProperty::GetPresParamLength(int id) const {
        for(ppentry *p=firstpp; p; p=p->next) {
                if(p->id==id)
                        return p->buflen;
        }
        return -1;
}

int WindowProperty::GetPresParam(int id, void *buf, int maxbuf) const {
        for(ppentry *p=firstpp; p; p=p->next) {
                if(p->id==id) {
                        if(p->buflen<=maxbuf) {
                                memcpy(buf,p->buf,p->buflen);
                                return 0;
                        }
                }
        }
        return -1;
}

int WindowProperty::SetPresParam(int id, const void *buf, int buflen) {
        ppentry *p=new ppentry;
        if(!p) return -1;
        p->buf=new char[buflen];
        if(!p->buf) {
                delete p;
                return -1;
        }
        p->id=id;
        p->buflen=buflen;
        memcpy(p->buf,buf,buflen);
        RemovePresParam(id);
        p->next=firstpp;
        firstpp=p;

        return 0;
}

void WindowProperty::RemovePresParam(int id) {
        for(ppentry *p=firstpp,*prev=0; p; prev=p,p=p->next) {
                if(p->id==id) {
                        if(prev) prev->next=p->next;
                        else firstpp=p->next;
                        delete[] p->buf;
                        delete p;
                        return;
                }
        }
}
