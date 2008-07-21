#ifndef __NPCLIENT_H
#define __NPCLIENT_H

class FNPClientConnection {
#if BIFOS_ == BIFOS_OS2_
        HFILE hf;
#elif BIFOS_ == BIFOS_WIN32_
        HANDLE hf;
#else
#error should not happen
#endif
public:
        FNPClientConnection(const char *name);
        virtual ~FNPClientConnection() { Close(); }
        virtual int  Read(void *buf, int maxbytes);
        virtual int  Write(const void *buf, int bytes);
        virtual void Close();
        virtual int Fail() const;

#if BIFOS_ == BIFOS_OS2_
        HFILE GetHFILE() { return hf; }
#elif BIFOS_ == BIFOS_WIN32_
        HANDLE GetHANDLE() { return hf; }
#endif
};

#endif
