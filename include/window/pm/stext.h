#ifndef __STEXT_H_INCLUDED
#define __STEXT_H_INCLUDED

class BIFCLASS FStaticText : public FControl, public FControlTextHandler {
public:
        FStaticText() : FControl(), FControlTextHandler(this) {}

        enum {
                st_left        =0x0000,
                st_center      =0x0001,
                st_right       =0x0002,
                st_leftnowrap  =0x0003,
                st_noprefix    =0x1000
        };

        Bool Create(FWnd *pParent, int ID)
          { return FControl::Create(pParent,ID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect, long style=st_left, const char *pszText=0);

        Bool SetText(const char *text);
        Bool SetText_ul(unsigned long ul);
        Bool SetText_l(long l);
        Bool SetText_u(unsigned u);
        Bool SetText_i(int i);
};

#endif
