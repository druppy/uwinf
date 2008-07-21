/* Basic window types */
#define winWS_OVERLAPPED       0x00000000L
#define winWS_POPUP            0x80000000L
#define winWS_CHILD            0x40000000L

/* Clipping styles */
#define winWS_CLIPSIBLINGS     0x04000000L
#define winWS_CLIPCHILDREN     0x02000000L

/* Generic window states */
#define winWS_VISIBLE          0x10000000L
#define winWS_DISABLED         0x08000000L

/* Main window states */
#define winWS_MINIMIZE         0x20000000L
#define winWS_MAXIMIZE         0x01000000L

/* Main window styles */
#define winWS_CAPTION          0x00C00000L     /* winWS_BORDER | winWS_DLGFRAME  */
#define winWS_BORDER           0x00800000L
#define winWS_DLGFRAME         0x00400000L
#define winWS_VSCROLL          0x00200000L
#define winWS_HSCROLL          0x00100000L
#define winWS_SYSMENU          0x00080000L
#define winWS_THICKFRAME       0x00040000L
#define winWS_MINIMIZEBOX      0x00020000L
#define winWS_MAXIMIZEBOX      0x00010000L

/* Control window styles */
#define winWS_GROUP            0x00020000L
#define winWS_TABSTOP          0x00010000L

/* Common Window Styles */
#define winWS_OVERLAPPEDWINDOW (winWS_OVERLAPPED | winWS_CAPTION | winWS_SYSMENU | winWS_THICKFRAME | winWS_MINIMIZEBOX | winWS_MAXIMIZEBOX)
#define winWS_POPUPWINDOW      (winWS_POPUP | winWS_BORDER | winWS_SYSMENU)
#define winWS_CHILDWINDOW      (winWS_CHILD)

/* Dialog styles */
#define winDS_ABSALIGN         0x01L
#define winDS_SYSMODAL         0x02L
#define winDS_LOCALEDIT        0x20L
#define winDS_SETFONT          0x40L
#define winDS_MODALFRAME       0x80L
#define winDS_NOIDLEMSG        0x100L
