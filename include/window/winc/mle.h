#ifndef __MLE_H_INCLUDED
#define __MLE_H_INCLUDED

class BIFCLASS FMLE : public FControl {
public:
	FMLE() : FControl() {}


	enum {
		mle_hscroll  =0x0001,
		mle_vscroll  =0x0002,
		mle_readonly =0x0004,
		mle_border   =0x0008
	};

	Bool Create(FWnd *pParent, int ID)
	  { return FControl::Create(pParent,ID); }
	Bool Create(FWnd *pParent, int ID, FRect *rect, long style=mle_border, const char *pszText=0);


	//clipboard operations:
	Bool Cut();
	Bool Copy();
	Bool Paste();
	Bool Clear();
	//selection operations:
	Bool ReplaceSelection(const char *text);

	//modify flag
	Bool HasBeenModified();
	Bool SetModify(Bool modified);

	//text
	int GetTextLen();
	Bool GetText(char *buffer, int maxbuf);
	Bool SetText(const char *text);
	Bool LimitText(int limit);

	//readonly
	Bool IsReadonly();
	Bool SetReadonly(Bool f);

	//more to come...
};

#endif
