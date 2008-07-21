/***
Filename: module.cpp
Description:
  Implementation of FModule (not a lot :-) (yet! :-)))
Host:
  Watcom 10.0a-10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-04-21 Adapted to WMS
**/
#define BIFINCL_WINDOW
#define BIFINCL_MODULE
#define BIFINCL_RESOURCETEMPLATE
#include <bif.h>
#include <string.h>

FModule::FModule() {
        restbl = &resource_table_;
}

FModule::~FModule() {
}


const void *FModule::LoadIcon(int resID) {
        return restbl->Find(RESTYPE_ICON,resID);
}

const void *FModule::LoadPointer(int resID) {
        return restbl->Find(RESTYPE_POINTER,resID);
}

const void *FModule::LoadMenu(int resID) {
        return restbl->Find(RESTYPE_MENU,resID);
}

const void *FModule::LoadAccelTable(int resID) {
        return restbl->Find(RESTYPE_ACCELTABLE,resID);
}

int FModule::LoadString(int resID, char *buf, int maxbuf) {
        const void *p=restbl->Find(RESTYPE_STRING,resID);
        if(p) {
                strncpy(buf,(char*)p,maxbuf);
                buf[maxbuf-1]='\0';
                return strlen(buf);
        } else
                return 0;
}

const DialogResource *FModule::FindDialogResource(int resID) {
        const void *p=restbl->Find(RESTYPE_DIALOG,resID);
        if(p)
                return (const DialogResource*)p;
        else
                return 0;
}


