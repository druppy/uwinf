/*0x0000..0x001f Standard messages*/
#define WM_NULL                 0x0000
#define WM_CREATE               0x0001
#define WM_DESTROY              0x0002
#define WM_SHOW                 0x0003
#define WM_ENABLE               0x0004
#define WM_ADJUSTWINDOWPOS      0x0010
#define WM_WINDOWPOSCHANGED     0x0011
#define WM_SIZE                 0x0012
#define WM_MOVE                 0x0013
#define WM_QUIT                 0x0014
/*0x0020..0x002f Property messages*/
#define WM_QUERYTEXTLENGTH      0x0020
#define WM_QUERYTEXT            0x0021
#define WM_SETTEXT              0x0022
#define WM_SETWINDOWPARAM       0x0023
#define WM_QUERYWINDOWPARAM     0x0024
#define WM_SETPRESPARAM         0x0025
#define WM_QUERYPRESPARAM       0x0026
#define WM_PRESPARAMCHANGED     0x0027
#define WM_STYLECHANGED         0x0028
/*0x0030..0x003f Input - keyboard*/
#define WM_FOCUSCHANGING        0x0030
#define WM_SETFOCUS             0x0031
#define WM_KEYBOARD             0x0032
#define WM_KEYBOARDFIRST 0x0030
#define WM_KEYBOARDLAST 0x0032
/*0x0040..0x005f Input - mouse*/
#define WM_MOUSEACTIVITY        0x0040
#define WM_HITTEST              0x0041
#define WM_MOUSEMOVE            0x0042
#define WM_SETPOINTER           0x0043
#define WM_BUTTON1DOWN          0x0044
#define WM_BUTTON1UP            0x0045
#define WM_BUTTON1DBLCLK        0x0046
#define WM_BUTTON2DOWN          0x0047
#define WM_BUTTON2UP            0x0048
#define WM_BUTTON2DBLCLK        0x0049
#define WM_BUTTON3DOWN          0x004a
#define WM_BUTTON3UP            0x004b
#define WM_BUTTON3DBLCLK        0x004c
#define WM_MOUSEFIRST 0x0040
#define WM_MOUSELAST 0x004c
/*0x0060..0x006f Timer services*/
#define WM_TIMER                0x0060
/*0x0070..0x007f Paint&redraw*/
#define WM_PAINT                0x0070
/*0x0080.0x008f Help manager*/
#define WM_HELP                 0x0080
/*0x0090..0x009f Controls etc.*/
#define WM_COMMAND              0x0090
#define WM_SYSCOMMAND           0x0091
#define WM_CONTROL              0x0092
#define WM_DRAWITEM             0x0093
#define WM_MEASUREITEM          0x0094
/*frames*/
#define WM_CLOSE                0x0100
#define WM_SAVEAPPLICATION      0x0101
#define WM_TRACKFRAME           0x0150
#define WM_QUERYTRACKINFO       0x0151
#define WM_QUERYWINDOWSTATE     0x0152
#define WM_SETWINDOWSTATE       0x0153
#define WM_QUERYACTIVE          0x0154
#define WM_SETACTIVE            0x0155
#define WM_CALCFRAMERECT        0x0156
#define WM_UPDATEFRAME          0x0157
#define WM_QUERYFRAMECTLCOUNT   0x0158
#define WM_FORMATFRAME          0x0159
#define WM_ACTIVATE             0x015a
#define WM_OWNERPOSCHANGING     0x015b
#define WM_OWNERSTATECHANGING   0x015c
#define WM_QUERYFRAMEINFO       0x015d

/*dialogs*/
#define WM_DISMISSDLG           0x01a0
#define WM_QUERYDLGCODE         0x01a1
#define WM_MATCHMNEMONIC        0x01a2
#define WM_QUERYRESOURCEITEMPTR 0x01a3

/*control-specific messages*/
#define WM_CUT                  0x0200
#define WM_COPY                 0x0201
#define WM_CLEAR                0x0202
#define WM_PASTE                0x0203

#define BM_QUERYCHECK           0x0280
#define BM_QUERYCHECKINDEX      0x0281
#define BM_SETCHECK             0x0282
#define BM_CLICK                0x0283
#define BM_QUERYDEFAULT         0x0284
#define BM_SETDEFAULT           0x0285

#define EM_SETTEXTLIMIT         0x0300
#define EM_QUERYFIRSTCHAR       0x0301
#define EM_SETFIRSTCHAR         0x0302
#define EM_QUERYREADONLY        0x0303
#define EM_SETREADONLY          0x0304
#define EM_QUERYSELECTION       0x0305
#define EM_SETSELECTION         0x0306

#define SBM_QUERYPOS            0x0380
#define SBM_SETPOS              0x0381
#define SBM_QUERYRANGESTART     0x0382
#define SBM_QUERYRANGEEND       0x0383
#define SBM_SETRANGE            0x0384
#define SBM_SETTHUMBSIZE        0x0386

#define LM_DELETEALL            0x0400
#define LM_DELETEITEM           0x0401
#define LM_INSERTITEM           0x0402
#define LM_QUERYITEMCOUNT       0x0403
#define LM_QUERYITEMDATA        0x0404
#define LM_QUERYITEMTEXT        0x0405
#define LM_QUERYITEMTEXTLEN     0x0406
#define LM_QUERYSELECTION       0x0407
#define LM_QUERYTOPINDEX        0x0408
#define LM_SEARCHSTRING         0x0409
#define LM_SELECTITEM           0x040a
#define LM_SETITEMDATA          0x040b
#define LM_SETITEMTEXT          0x040c
#define LM_SETTOPINDEX          0x040d

#define CBM_SHOWLIST            0x0480
#define CBM_ISLISTSHOWING       0x0481

//menus
#define MM_DELETEITEM           0x0500
#define MM_ENDMENUMODE          0x0501
#define MM_INSERTITEM           0x0502
#define MM_ISITEMVALID          0x0503
#define MM_ITEMIDFROMPOSITION   0x0504
#define MM_ITEMPOSITIONFROMID   0x0505
#define MM_QUERYITEM            0x0506
#define MM_QUERYITEMATTR        0x0507
#define MM_QUERYITEMCOUNT       0x0508
#define MM_QUERYITEMRECT        0x0509
#define MM_QUERYITEMTEXT        0x050a
#define MM_QUERYITEMTEXTLENGTH  0x050b
#define MM_QUERYSELITEMID       0x050c
#define MM_REMOVEITEM           0x050d
#define MM_SELECTITEM           0x050e
#define MM_SETITEM              0x050f
#define MM_SETITEMATTR          0x0510
#define MM_SETITEMHANDLE        0x0511
#define MM_SETITEMTEXT          0x0512
#define MM_STARTMENUMODE        0x0513
#define WM_INITMENU             0x0520
#define WM_MENUEND              0x0521
#define WM_MENUSELECT           0x0522
#define WM_NEXTMENU             0x0523

/*switchlist/desktop*/
#define WM_ADDSWITCHENTRY       0x0600
#define WM_CHANGESWITCHENTRY    0x0601
#define WM_QUERYSWITCHENTRY     0x0602
#define WM_DELETESWITCHENTRY    0x0603
#define WM_SHOWSWITCHLIST       0x0604


/*0x0100..0x1000 reserved*/
#define WM_USER                 0x1000

/*0xf000..0xffff reserved for registered messages*/
