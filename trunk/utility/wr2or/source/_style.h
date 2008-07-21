/* Static Control Styles */
#define winSS_LEFT             0x00000000L
#define winSS_CENTER           0x00000001L
#define winSS_RIGHT            0x00000002L
#define winSS_ICON             0x00000003L
#define winSS_BLACKRECT        0x00000004L
#define winSS_GRAYRECT         0x00000005L
#define winSS_WHITERECT        0x00000006L
#define winSS_BLACKFRAME       0x00000007L
#define winSS_GRAYFRAME        0x00000008L
#define winSS_WHITEFRAME       0x00000009L
#define winSS_SIMPLE           0x0000000BL
#define winSS_LEFTNOWORDWRAP   0x0000000CL
#define winSS_NOPREFIX         0x00000080L

/* Button Control Styles */
#define winBS_PUSHBUTTON       0x00000000L
#define winBS_DEFPUSHBUTTON    0x00000001L
#define winBS_CHECKBOX         0x00000002L
#define winBS_AUTOCHECKBOX     0x00000003L
#define winBS_RADIOBUTTON      0x00000004L
#define winBS_3STATE           0x00000005L
#define winBS_AUTO3STATE       0x00000006L
#define winBS_GROUPBOX         0x00000007L
#define winBS_USERBUTTON       0x00000008L
#define winBS_AUTORADIOBUTTON  0x00000009L
#define winBS_OWNERDRAW        0x0000000BL
#define winBS_LEFTTEXT         0x00000020L

/* Edit styles */
#define winES_LEFT             0x00000000L
#define winES_CENTER           0x00000001L
#define winES_RIGHT            0x00000002L
#define winES_MULTILINE        0x00000004L
#define winES_UPPERCASE        0x00000008L
#define winES_LOWERCASE        0x00000010L
#define winES_PASSWORD         0x00000020L
#define winES_AUTOVSCROLL      0x00000040L
#define winES_AUTOHSCROLL      0x00000080L
#define winES_NOHIDESEL        0x00000100L
#define winES_OEMCONVERT       0x00000400L
#define winES_READONLY         0x00000800L
#define winES_WANTRETURN       0x00001000L

/* Scroll bar styles */
#define winSBS_HORZ                    0x0000L
#define winSBS_VERT                    0x0001L
#define winSBS_TOPALIGN                0x0002L
#define winSBS_LEFTALIGN               0x0002L
#define winSBS_BOTTOMALIGN             0x0004L
#define winSBS_RIGHTALIGN              0x0004L
#define winSBS_SIZEBOXTOPLEFTALIGN     0x0002L
#define winSBS_SIZEBOXBOTTOMRIGHTALIGN 0x0004L
#define winSBS_SIZEBOX                 0x0008L

/* Listbox styles */
#define winLBS_NOTIFY            0x0001L
#define winLBS_SORT              0x0002L
#define winLBS_NOREDRAW          0x0004L
#define winLBS_MULTIPLESEL       0x0008L
#define winLBS_OWNERDRAWFIXED    0x0010L
#define winLBS_OWNERDRAWVARIABLE 0x0020L
#define winLBS_HASSTRINGS        0x0040L
#define winLBS_USETABSTOPS       0x0080L
#define winLBS_NOINTEGRALHEIGHT  0x0100L
#define winLBS_MULTICOLUMN       0x0200L
#define winLBS_WANTKEYBOARDINPUT 0x0400L
#define winLBS_EXTENDEDSEL       0x0800L
#define winLBS_DISABLENOSCROLL   0x1000L
#define winLBS_STANDARD          (winLBS_NOTIFY | winLBS_SORT | winWS_VSCROLL | winWS_BORDER)

/* Combobox styles */
#define winCBS_SIMPLE            0x0001L
#define winCBS_DROPDOWN          0x0002L
#define winCBS_DROPDOWNLIST      0x0003L
#define winCBS_OWNERDRAWFIXED    0x0010L
#define winCBS_OWNERDRAWVARIABLE 0x0020L
#define winCBS_AUTOHSCROLL       0x0040L
#define winCBS_OEMCONVERT        0x0080L
#define winCBS_SORT              0x0100L
#define winCBS_HASSTRINGS        0x0200L
#define winCBS_NOINTEGRALHEIGHT  0x0400L
#define winCBS_DISABLENOSCROLL   0x0800L

unsigned long translateStyleString(const char *_s, unsigned long defstyle);

