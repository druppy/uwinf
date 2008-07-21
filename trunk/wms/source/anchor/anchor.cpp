#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#include "wms.hpp"

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#include "habmap.hpp"
#include "../wmsmutex.hpp"
#include "anchor.hpp"

static AnchorBlock *first;      //first anchorblock in list
static HABMap habmap;           //WmsHAB->ab mapper
static WmsHAB HAB_sequencer;    //for generating WmsHABs
static WmsMutex HABMutex;       //module lock
class HABLock {
public:
        HABLock() { HABMutex.Request(); }
        ~HABLock() { HABMutex.Release(); }
};

WmsHAB WmsInitialize(uint32 /*options*/) {
        unsigned long tid=FThreadManager::GetCurrentThread()->GetTID();
        HABLock hb;
        for(AnchorBlock *ab=first; ab; ab=ab->next)
                if(ab->tid==tid)
                        return 0;       //already initialized

        ab=new AnchorBlock;
        if(ab) {
                HAB_sequencer = (WmsHAB)(((uint32)HAB_sequencer)+1);
                ab->hab = HAB_sequencer;
                ab->tid = tid;
                ab->hmq = 0;
                ab->next=first;
                first=ab;
                habmap.Insert(ab->hab,tid,ab);
                return ab->hab;
        } else
                return 0;
};

Bool WmsTerminate(WmsHAB hab) {
        AnchorBlock *ab;
        {
                HABLock hl;
                ab=habmap.Find(hab);
                if(!ab)
                        return False;
                if(ab->tid!=FThreadManager::GetCurrentThread()->GetTID())
                        return False;
                habmap.Remove(hab);
                for(AnchorBlock *p=first,*pp=0; p; pp=p,p=p->next) {
                        if(p==ab) {
                                if(pp) pp->next=p->next;
                                else first=p->next;
                                break;
                        }
                }
        }
        WmsDestroyMsgQueue(ab->hmq);
        delete ab;
        return True;
}


WmsHAB WmsQueryThreadAnchorBlock(unsigned long tid) {
        HABLock hl;
        AnchorBlock *ab=habmap.Find(tid);
        if(ab)
                return ab->hab;
        else
                return 0;
}

WmsHMQ WmsQueryAnchorBlockMsgQueue(WmsHAB hab) {
        HABLock hl;
        AnchorBlock *ab=habmap.Find(hab);
        if(ab)
                return ab->hmq;
        else
                return 0;
}

unsigned long WmsQueryAnchorBlockThread(WmsHAB hab) {
        HABLock hl;
        AnchorBlock *ab=habmap.Find(hab);
        if(ab)
                return ab->tid;
        else
                return 0;
}

AnchorBlock *WmsQueryAnchorBlockAnchorBlock_internal(WmsHAB hab) {
        HABLock hl;
        return habmap.Find(hab);
}

Bool WmsSetAnchorBlockMsgQueue_internal(WmsHAB hab, WmsHMQ hmq) {
        HABLock hl;
        AnchorBlock *ab=habmap.Find(hab);
        if(ab) {
                ab->hmq=hmq;
                return True;
        } else
                return False;
}

