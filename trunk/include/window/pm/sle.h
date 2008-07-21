#ifndef __SLE_H_INCLUDED
#define __SLE_H_INCLUDED

class BIFCLASS FSLE : public FControl, 
                      public FControlTextHandler 
{
public:
        FSLE() : FControl(), FControlTextHandler(this) {}

        enum {
                sle_password    = 0x0001,
                sle_autohscroll = 0x0002,
                sle_readonly    = 0x0004,
                sle_border      = 0x0008
        };

        Bool Create(FWnd *pParent, int ID)
          { return FControl::Create(pParent,ID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect, long style=sle_border, const char *pszText=0);


        //clipboard operations
        Bool Cut();
        Bool Copy();
        Bool Clear();
        Bool Paste();

        //text operations
        Bool LimitText(int limit);

        //readonly
        Bool IsReadonly();
        Bool SetReadonly(Bool f);

        //selection
        Bool GetSelection(int *start, int *end);
        Bool SetSelection(int start, int end);
};

#endif
