#ifndef __PRESPARAM_H_INCLUDED
#define __PRESPARAM_H_INCLUDED
Bool WmsSetPresParam(WmsHWND hwnd, int id, const void *buf, unsigned buflen);
Bool WmsRemovePresParam(WmsHWND hwnd, int id);
Bool WmsQueryPresParam(WmsHWND hwnd, int id1, int id2, int *idFound, void *buf, int buflen, Bool inherit=True);


//desktop
#define PP_DESKTOPFOREGROUNDCOLOR                101    //desktop color
#define PP_DESKTOPBACKGROUNDCOLOR                102    //desktop color
#define PP_DESKTOPFILLCHAR                       103    //desktop fill character

//frame stuff
#define PP_ACTIVEBORDERCOLOR                     201    //border and titlebar color when active
#define PP_INACTIVEBORDERCOLOR                   202    //border and titlebar color when active
#define PP_ACTIVETEXTFOREGROUNDCOLOR             203    //titlebar text color when active
#define PP_ACTIVETEXTBACKGROUNDCOLOR             204    //-
#define PP_INACTIVETEXTFOREGROUNDCOLOR           205    //titlebar text color when inactive
#define PP_INACTIVETEXTBACKGROUNDCOLOR           206    //-
#define PP_TITLEBUTTONFOREGROUNDCOLOR            207    //titlebar buttons color
#define PP_TITLEBUTTONBACKGROUNDCOLOR            208    //-
#define PP_THINBORDERCHARS                       209    //6 chars for thin border (/-\|\/)
#define PP_SIZEBORDERCHARS                       210    //6 chars for size border (/=\|\/)
#define PP_MODALBORDERCHARS                      211    //6 chars for modal border (/=\|\/)
#define PP_TITLEBUTTONMAXCHARS                   212    //3 chars
#define PP_TITLEBUTTONMINCHARS                   213    //-
#define PP_TITLEBUTTONHIDECHARS                  214    //-
#define PP_TITLEBUTTONRESTORECHARS               215    //-
#define PP_TITLEBUTTONSYSMENUCHARS               216

#define PP_DIALOGBACKGROUNDCOLOR                 300    //dialog "background"
//controls etc.
#define PP_DEFAULTFOREGROUNDCOLOR                301    //eg static text, labels etc
#define PP_DEFAULTBACKGROUNDCOLOR                302    //-
#define PP_DISABLEDFOREGROUNDCOLOR               303    //"grayed"
#define PP_DISABLEDBACKGROUNDCOLOR               304    //-
#define PP_FOCUSFOREGROUNDCOLOR                  305    //focused color
#define PP_FOCUSBACKGROUNDCOLOR                  306    //-
#define PP_HILITEFOREGROUNDCOLOR                 307    //down color
#define PP_HILITEBACKGROUNDCOLOR                 308    //-
#define PP_MNEMONICFOREGROUNDCOLOR               309    //hotkey
#define PP_MNEMONICBACKGROUNDCOLOR               310    //-
//control specific:
//button
#define PP_BUTTONMIDDLEFOREGROUNDCOLOR           401    //normal button color
#define PP_BUTTONMIDDLEBACKGROUNDCOLOR           402    //
#define PP_BUTTONSHADOWFOREGROUNDCOLOR           403    //button shadow
#define PP_BUTTONSHADOWBACKGROUNDCOLOR           404    //
#define PP_BUTTONSHADOWCHARS                     405    //right, bottom, bottom-right
#define PP_BUTTONDEFAULTFOREGROUNDCOLOR          406    //default indicators on button
#define PP_BUTTONDEFAULTBACKGROUNDCOLOR          407    //-
#define PP_BUTTONDEFAULTLEFTCHAR                 408    //left character
#define PP_BUTTONDEFAULTRIGHTCHAR                409    //right character
#define PP_BUTTONMNEMONICFOREGROUNDCOLOR         410    //hotkey (accelerator-character)
#define PP_BUTTONMNEMONICBACKGROUNDCOLOR         411    //-
//checkboxes
#define PP_CBUNCHECKED                           501    //unchecked
#define PP_CBCHECKED                             502    //checked
#define PP_CBUNDETERMINED                        503    //3rd state
//radiobuttons
#define PP_RBUNSELECTED                          601    //unselected
#define PP_RBSELECTED                            602    //selected
//listboxes:
#define PP_LBNORMALFOREGROUNDCOLOR               701
#define PP_LBNORMALBACKGROUNDCOLOR               702
#define PP_LBSELECTEDFOREGROUNDCOLOR             703
#define PP_LBSELECTEDBACKGROUNDCOLOR             704
#define PP_LBFOCUSFOREGROUNDCOLOR                705
#define PP_LBFOCUSBACKGROUNDCOLOR                706
#define PP_LBSELECTEDFOCUSFOREGROUNDCOLOR        707
#define PP_LBSELECTEDFOCUSBACKGROUNDCOLOR        708
//groupboxes:
#define PP_GBBORDERCHARS                         800     //frame characters
//SLE/MLE:
#define PP_LENORMALFOREGROUNDCOLOR               900    //"background" in entryfields and MLEs
#define PP_LENORMALBACKGROUNDCOLOR               901
#define PP_LESELECTIONFOREGROUNDCOLOR            902
#define PP_LESELECTIONBACKGROUNDCOLOR            903
#define PP_LEFOCUSFOREGROUNDCOLOR                904
#define PP_LEFOCUSBACKGROUNDCOLOR                905
#define PP_LEDISABLEDFOREGROUNDCOLOR             906
#define PP_LEDISABLEDBACKGROUNDCOLOR             907
#define PP_SLESCROLLFOREGROUNDCOLOR              908
#define PP_SLESCROLLBACKGROUNDCOLOR              909
#define PP_SLESCROLLCHARS                        910
//scrollbars:
#define PP_SBCHARS                              1000    //left,right,up,down,track,verticalthumb,horizontalthumb
#define PP_SBSCROLLFOREGROUNDCOLOR              1001
#define PP_SBSCROLLBACKGROUNDCOLOR              1002
#define PP_SBTRACKFOREGROUNDCOLOR               1003
#define PP_SBTRACKBACKGROUNDCOLOR               1004
#define PP_THUMBFOREGROUNDCOLOR                 1005
#define PP_THUMBBACKGROUNDCOLOR                 1006
#define PP_SBDISABLEDSCROLLFOREGROUNDCOLOR      1007
#define PP_SBDISABLEDSCROLLBACKGROUNDCOLOR      1008
#define PP_SBDISABLEDTRACKFOREGROUNDCOLOR       1009
#define PP_SBDISABLEDTRACKBACKGROUNDCOLOR       1010
//comboxes:
#define PP_CBBUTTONFOREGROUNDCOLOR              1100
#define PP_CBBUTTONBACKGROUNDCOLOR              1101
#define PP_CBDROPDOWNCHARS                      1102
#define PP_CBROLLUPCHARS                        1103

//menus:
#define PP_MBORDERCHARS                         1200
#define PP_MSUBMENUCHAR                         1201
#define PP_MCONDITIONALCASCADECHAR              1202
#define PP_MCHECKCHAR                           1203
#define PP_MSEPARATORCHAR                       1204
#define PP_MFOREGROUNDCOLOR                     1205
#define PP_MBACKGROUNDCOLOR                     1206
#define PP_MBORDERFOREGROUNDCOLOR               1207
#define PP_MBORDERBACKGROUNDCOLOR               1208
#define PP_MNORMALFOREGROUNDCOLOR               1209
#define PP_MNORMALBACKGROUNDCOLOR               1210
#define PP_MHOTKEYFOREGROUNDCOLOR               1211
#define PP_MHOTKEYBACKGROUNDCOLOR               1212
#define PP_MDISABLEDFOREGROUNDCOLOR             1213
#define PP_MDISABLEDBACKGROUNDCOLOR             1214
#define PP_MSELECTEDFOREGROUNDCOLOR             1215
#define PP_MSELECTEDBACKGROUNDCOLOR             1216
#define PP_MSTATICFOREGROUNDCOLOR               1217
#define PP_MSTATICBACKGROUNDCOLOR               1218

//more to come

#endif
