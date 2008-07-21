#ifndef __SYSQUEUE_HPP
#define __SYSQUEUE_HPP

#include "wms.hpp"

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

struct SystemMouseEvent {
        sint16 column;
        sint16 row;
        uint8 buttonState;
};

struct SystemKeyboardEvent {
        uint8 character;
        uint8 scancode;
        uint8 logicalkey;
        uint8 flags;
};

#define skef_char 0x40
#define skef_key  0x80

struct SystemEvent {
        enum se_type { mouse, keyboard } type;
        union {
                struct SystemMouseEvent sme;
                struct SystemKeyboardEvent ske;
        } u;
        uint8 keyboardShiftState; // kss_...
        TimeStamp timestamp;
};


class SystemEventQueue {
        FEventSemaphore notEmpty;
        FSemaphore notFull;
        FMutexSemaphore queueModify;
        enum {queueSize=20};
        SystemEvent q[queueSize];
        int msgs;
public:
        SystemEventQueue();
        ~SystemEventQueue();
        int get(SystemEvent *se, int timeout);
        int put(const SystemEvent *se, int timeout);
};

extern SystemEventQueue systemEventQueue_internal;

#endif

