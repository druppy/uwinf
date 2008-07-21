#ifndef __PUSHBUT_H_INCLUDED
#define __PUSHBUT_H_INCLUDED

class BIFCLASS FPushButton : public FControl, public FControlTextHandler {
public:
	FPushButton() : FControl(), FControlTextHandler(this) {}

	Bool Create(FWnd *pParent, int ID)
	  { return FControl::Create(pParent,ID); }
	Bool Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, Bool isDefault=False);

	void Push();
};

#endif
