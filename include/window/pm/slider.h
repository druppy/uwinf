#ifndef __SLIDER_H_INCLUDED
#define __SLIDER_H_INCLUDED

class BIFCLASS FSlider : public FControl {
public:
        enum {
                sls_horizontal      =0x0000,
                sls_vertical        =0x0001,
                sls_center          =0x0000,
                sls_left            =0x0002,
                sls_right           =0x0004,
                sls_top=sls_right, sls_bottom=sls_left,
                sls_snaptoincrement =0x0008,
              //sls_nobuttons
                sls_buttonsleft     =0x0010,
                sls_buttonsright    =0x0020,
                sls_buttonsbottom=sls_buttonsleft, sls_buttonstop=sls_buttonsright,
                sls_ownerdraw       =0x0040,
                sls_readonly        =0x0080,
                sls_ribbonstrip     =0x0100,
                sls_homeleft        =0x0000,
                sls_homeright       =0x0200,
                sls_homebottom=sls_homeleft, sls_hometop=sls_homeright,
                sls_primaryscale1   =0x0000,
                sls_primaryscale2   =0x0400
        };

        Bool Create(FWnd *pParent, int ID, int rangeMin, int rangeMax, int tickIncrement);
        Bool Create(FWnd *pParent, int ID, 
                    FRect *rect, ULONG style,
                    int rangeMin, int rangeMax, int tickIncrement);

        typedef ULONG DetentID;
        DetentID AddDetent(int pos);
        Bool RemoveDetent(DetentID did);
        int GetDetentPos(DetentID did, int *scale=0);

        Bool SetTickText(int pos, const char *pszText);
        Bool GetTickText(int pos, char *buf, int buflen);

        int GetTickSize(int pos);
        Bool SetTickSize(int pos, int pels);

        int GetArmPosition();
        Bool SetArmPosition(int pos);

        //the rest is strictly OS/2 only
private:
        int rangeMin, rangeMax, tickIncrement, range;

protected:
        Bool ResetTranslation(int rangeMin, int rangeMax, int tickIncrement);
public:
        int pel2LV(int pel);
        int LV2pel(int lv);
        int pos2ticknum(int pos);
        int ticknum2pos(int ticknum);
};

#endif

