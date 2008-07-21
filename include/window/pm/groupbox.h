#ifndef __GROUPBOX_H_INCLUDED
#define __GROUPBOX_H_INCLUDED

class BIFCLASS FGroupBox : public FControl, 
                           public FControlTextHandler 
{
public:
        FGroupBox() : FControl(), FControlTextHandler(this) {}

        Bool Create(FWnd *pParent, int ID)
          { return FControl::Create(pParent,ID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect, const char *pszText=0);
};

#endif
