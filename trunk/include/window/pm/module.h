#ifndef __MODULE_H_INCLUDED
#define __MODULE_H_INCLUDED

class BIFCLASS FModule {
        FModule(const FModule&);                //don't copy
        FModule& operator=(const FModule&);     //don't copy
public:
        FModule(HMODULE h);
        FModule(const char *name);
        virtual ~FModule();

        int Fail() const {
                return hmod==NULL;
        }

        HMODULE GetMODULE() const {
                return hmod;
        }

        
        HBITMAP LoadBitmap(HPS hps, int resID, int lWidth, int lHeight);
        HBITMAP LoadBitmap(HPS hps, int resID)
          { return LoadBitmap(hps,resID,0,0); }

        HPOINTER LoadIcon(int resID) { 
                return WinLoadPointer(HWND_DESKTOP,hmod,resID); 
        }

        HPOINTER LoadPointer(int resID) {
                return WinLoadPointer(HWND_DESKTOP,hmod,resID); 
        }
        
        HWND LoadMenu(HWND hwndFrame, int resID) {
                return WinLoadMenu(hwndFrame,hmod,resID);
        }


        int LoadMessage(HAB hab, int resID, char *buf, int maxbuf) {
                return WinLoadMessage(hab,hmod,resID,maxbuf,(PSZ)buf);
        }

        int LoadString(HAB hab, int resID, char *buf, int maxbuf) {
                return WinLoadString(hab,hmod,resID,maxbuf,(PSZ)buf);
        }

        static FModule *GetDefaultResourceModule();
        static void SetDefaultResourceModule(FModule *mod);
private:
        HMODULE hmod;
        Bool zapModule;
        static char failName[256];
        static FModule *defResModule;
public: //OS/2 only:
        static char *GetFailName();
};

#endif /*__BW_MODULE_H_INCLUDED*/

