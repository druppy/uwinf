/***
Filename: ownerdrw.cpp
Description:
  Implementation of FModule (not a lot :-)
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 97-02-01 Implemented GetDefaultResourceModule()
**/
#define INCL_GPIBITMAPS
#define BIFINCL_WINDOW
#define BIFINCL_MODULE
#define BIFINCL_APPLICATION
#include <bif.h>

static char LoadModule_FailName[256];
FModule *FModule::defResModule=0;


FModule::FModule(HMODULE h) {
        hmod=h;
        zapModule=False;
}
            
FModule::FModule(const char *name) {
        if(DosLoadModule((PSZ)LoadModule_FailName,256,name,&hmod)==0) {
                zapModule=True;
        } else {
                zapModule=False;
                hmod=NULL;
        }
}

FModule::~FModule() {
        if(zapModule) DosFreeModule(hmod);
}


HBITMAP FModule::LoadBitmap(HPS hps, int resID, int lWidth, int lHeight) {
        return GpiLoadBitmap(hps, hmod, resID, lWidth, lHeight);
}

FModule *FModule::GetDefaultResourceModule() {
        return defResModule?defResModule:GetCurrentApp();
}

void FModule::SetDefaultResourceModule(FModule *mod) {
        defResModule = mod;
}

char *FModule::GetFailName() {
        return LoadModule_FailName;
}
