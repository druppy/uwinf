#ifndef __RADIOBUT_H_INCLUDED
#define __RADIOBUT_H_INCLUDED

class BIFCLASS FRadioButton : public FControl, public FControlTextHandler {
public:
	FRadioButton() : FControl(), FControlTextHandler(this) {}

	Bool Create(FWnd *pParent, int ID)
	  { return FControl::Create(pParent,ID); }
	Bool Create(FWnd *pParent, int ID, FRect *rect, const char *pszText);

	Bool IsChecked();
	Bool Check();
	Bool UnCheck();

	int GetCheckIndex();
	Bool CheckIndex(int i);
};

#endif
