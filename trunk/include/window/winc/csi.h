#ifndef __CSI_H_INCLUDED
#define __CSI_H_INCLUDED

//CSI (coordinate-system independent
class BIFCLASS FCSIBaseHandler : public FHandler {
public:
	FCSIBaseHandler(FWnd *pwnd)
	  : FHandler(pwnd)
	  {}
protected:
	Bool IsSibling(FWnd *pWnd);
	void gwripc(FWnd *pWnd, FRect *pr);
};

class BIFCLASS FCSIMoveHandler : public FCSIBaseHandler {
public:
	FCSIMoveHandler(FWnd *pwnd)
	  : FCSIBaseHandler(pwnd)
	  {}

	//movement relative to parent
	Bool AlignTTParent();
	Bool AlignBBParent();
	Bool AlignLLParent();
	Bool AlignRRParent();

	//movement relative to siblings
	Bool AlignLLSibling(FWnd *pSibling);
	Bool AlignRRSibling(FWnd *pSibling);
	Bool AlignTTSibling(FWnd *pSibling);
	Bool AlignBBSibling(FWnd *pSibling);
	Bool AlignLRSibling(FWnd *pSibling);
	Bool AlignRLSibling(FWnd *pSibling);
	Bool AlignTBSibling(FWnd *pSibling);
	Bool AlignBTSibling(FWnd *pSibling);
private:
	void AlignTop(int newtop);
	void AlignBottom(int newbottom);
	void AlignLeft(int newleft);
	void AlignRight(int newright);
};


class BIFCLASS FCSISizeHandler : public FCSIBaseHandler {
public:
	FCSISizeHandler(FWnd *pwnd)
	  : FCSIBaseHandler(pwnd)
	  {}

	//size relative to parent
	Bool SizeTTParent();
	Bool SizeBBParent();
	Bool SizeLLParent();
	Bool SizeRRParent();

	//size relative to siblings
	Bool SizeLLSibling(FWnd *pSibling);
	Bool SizeRRSibling(FWnd *pSibling);
	Bool SizeTTSibling(FWnd *pSibling);
	Bool SizeBBSibling(FWnd *pSibling);
	Bool SizeLRSibling(FWnd *pSibling);
	Bool SizeRLSibling(FWnd *pSibling);
	Bool SizeTBSibling(FWnd *pSibling);
	Bool SizeBTSibling(FWnd *pSibling);
private:
	void SizeTop(int newtop);
	void SizeBottom(int newbottom);
	void SizeLeft(int newleft);
	void SizeRight(int newright);
};


#endif
