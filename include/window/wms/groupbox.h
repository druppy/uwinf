#ifndef __GROUPBOX_H_INCLUDED
#define __GROUPBOX_H_INCLUDED

class BIFCLASS FGroupBox : public FControl, 
                           public FControlTextHandler 
{
public:
        FGroupBox() : FControl(), FControlTextHandler(this) {}

        Bool Create(FWnd *pParent, int resID)
          { return FControl::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect, const char *pszText=0);
protected:
        Bool Paint();
        Bool MatchMnemonic(char c);
        unsigned QueryDlgCode() { return DLGC_STATIC; }
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
};

#endif
