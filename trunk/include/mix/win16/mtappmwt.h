#ifndef __MTAPPMWT_H_INCLUDED
#define __MTAPPMWT_H_INCLUDED

class BIFCLASS FMTApplication : public FApplication {
        FMainWindowThread *mwt;
public:
        FMTApplication()
          : FApplication()
          {}

        Bool GetEvent(FEvent &);
        int DoMessageBox(HWND hwnd, const char *pszText, const char *pszTitle, UINT fuStyle);
};


class BIFCLASS FWMainWindowThread : public FMainWindowThread {
public: //Watcom has problems handling friend functions modified classes
        FApplication *(*pfn_makeappinst)();
public:
        FWMainWindowThread(int argc, char **argv)
          : FMainWindowThread(argc,argv)
	  {}

	int Main(int argc, char **argv);
};


#define DEFBIFMIXMAIN(appClass,mainThreadClass)                                                \
FApplication *MakeApplicationInstance() {                                                      \
	FMTApplication *app=new appClass;                                                      \
	return app;                                                                            \
}                                                                                              \
FMainThread *MakeMainThread(int argc, char *argv[]) {                                          \
	FWMainWindowThread *t=new mainThreadClass(argc,argv);                                  \
	if(t)                                                                                  \
		t->pfn_makeappinst=MakeApplicationInstance;                                    \
	return t;                                                                              \
}                                                                                              \
int PASCAL WinMain(HINSTANCE hinst, HINSTANCE hprevInst, LPSTR lpCmdLine, int nCmdShow) {      \
	return _BIFThreadMain(hinst,hprevInst,lpCmdLine,nCmdShow,MakeMainThread);              \
}

#endif

