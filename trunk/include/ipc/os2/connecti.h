#ifndef __CONNECTION_H
#define __CONNECTION_H

class FConnection {
protected:
        FConnection() {};
public:
        virtual ~FConnection() {}
        virtual int  Read(void *buf, int maxbytes) =0;
        virtual int  Write(const void *buf, int bytes) =0;
        virtual void Close() =0;
        virtual int Fail() const =0;
};

#endif
