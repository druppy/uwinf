#ifndef __FRAME_H_INCLUDED
#define __FRAME_H_INCLUDED


class FFrameWindow;

class BIFCLASS FFrameMenuDestroyHandler : public FHandler {
public:
        FFrameMenuDestroyHandler(FFrameWindow *pff);
protected:
        Bool Dispatch(FEvent &ev);
};

class BIFCLASS FFrameWindow : public FWnd,
                              private FFrameMenuDestroyHandler
{
public:
        FFrameWindow();
        ~FFrameWindow();

public:
        // Creation
        Bool Create(FWnd *pParent, int Id, long fcf, long alf, FRect *pRect = NULL, FModule *module=0 );
        enum {  //Auto-load flags
                alf_none         =0x0000,
                alf_icon         =0x0001,
                alf_menu         =0x0002,
                alf_accelerator  =0x0004,
                alf_caption      =0x0008
        };
        enum {  //frame creation flags
                fcf_none          =0x0000,
                fcf_sysmenu       =0x0001,      //system menu
                fcf_titlebar      =0x0002,      //title bar
                fcf_minbutton     =0x0004,      //minimize button
                fcf_maxbutton     =0x0008,      //maximize/restore button
                fcf_hidebutton    =0x0010,      //hide button
                fcf_minmaxbuttons =0x000c,      //minimize&maximize buttons
                fcf_bothbuttons   =0x0018,      //both buttons (os/2: hide+max)
                fcf_menu          =0x0020,      //menu
                fcf_horzscroll    =0x0040,      //horizontal scrollbar
                fcf_vertscroll    =0x0080,      //vertical scrollbar
                fcf_border        =0x0100,      //thin border
                fcf_sizeborder    =0x0200,      //size border
                fcf_dialogborder  =0x0300       //dialog border
        };

        // inter-frame relationship
        Bool BringToFront();
        Bool BringToBack();

        // state
        Bool Minimize();
        Bool Maximize();
        Bool Restore();

        //menu
        FMenu *GetMenu();
        void SetMenu( FMenu *m );
        FMenu *GetSystemMenu();
        void RedrawMenu();

        //caption
        Bool SetCaption(const char *s);
        int GetCaption(char *buf, int buflen);
        Bool SetSubtitle(const char *s);
        int GetSubtitle(char *buf, int buflen);

private:
        FIcon *icon;
        char *caption;
        char *subtitle;
        FMenu *pMenu,
              *pAutoMenu;
        FMenu *pSysMenu;

        void UpdateTitle();
protected:
        //icon
        FIcon *GetIcon();
        void SetIcon(FIcon *pIcon);
}; // End of FFrameWindow

#endif

