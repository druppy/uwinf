#ifndef __SLIDER_H__
#define __SLIDER_H__

//slider styles
#define SLS_HORIZONTAL       0x00000000L /* Orient slider horizontally*/
#define SLS_VERTICAL         0x00000001L /* Orient slider vertically  */
#define SLS_CENTER           0x00000000L /* Center shaft in window    */
#define SLS_BOTTOM           0x00000002L /* Offset shaft to bottom (H)*/
#define SLS_TOP              0x00000004L /* Offset shaft to top (H)   */
#define SLS_LEFT             0x00000002L /* Offset shaft to left (V)  */
#define SLS_RIGHT            0x00000004L /* Offset shaft to right (V) */
#define SLS_SNAPTOINCREMENT  0x00000008L /* Snap to nearest increment */
#define SLS_BUTTONSBOTTOM    0x00000010L /* Add buttons at shaft bot. */
#define SLS_BUTTONSTOP       0x00000020L /* Add buttons at shaft top  */
#define SLS_BUTTONSLEFT      0x00000010L /* Add buttons left of shaft */
#define SLS_BUTTONSRIGHT     0x00000020L /* Add buttons right of shaft*/
//#define SLS_OWNERDRAW        0x00000040L /* Owner draw some fields    */
#define SLS_READONLY         0x00000080L /* Provide a read only slider*/
#define SLS_RIBBONSTRIP      0x00000100L /* Provide a ribbon strip    */
#define SLS_HOMEBOTTOM       0x00000000L /* Set home position at bot. */
#define SLS_HOMETOP          0x00000200L /* Set home position at top  */
#define SLS_HOMELEFT         0x00000000L /* Set home position at left */
#define SLS_HOMERIGHT        0x00000200L /* Set home position at right*/
//#define SLS_PRIMARYSCALE1    0x00000000L /* Scale 1 is primary scale  */
//#define SLS_PRIMARYSCALE2    0x00000400L /* Scale 2 is primary scale  */
//slider messages
#define SLM_ADDDETENT          WM_USER+0    /* Add detent niche          */
#define SLM_QUERYDETENTPOS     WM_USER+1    /* Query position of detent  */
#define SLM_QUERYSCALETEXT     WM_USER+2    /* Query text at tick number */
#define SLM_QUERYSLIDERINFO    WM_USER+3    /* Query slider information  */
#define SLM_QUERYTICKPOS       WM_USER+4    /* Query position of tick    */
#define SLM_QUERYTICKSIZE      WM_USER+5    /* Query size of tick        */
#define SLM_REMOVEDETENT       WM_USER+6    /* Remove detent niche       */
#define SLM_SETSCALETEXT       WM_USER+7    /* Set text above tick       */
#define SLM_SETSLIDERINFO      WM_USER+8    /* Set slider parameters     */
#define SLM_SETTICKSIZE        WM_USER+9    /* Set size of tick          */
//message attributes:
#define SMA_SETALLTICKS          ((int)0xFFFF)
#define SMA_SLIDERARMPOSITION    0x0003
#define SMA_SHAFTRANGE           0x1001
#define SMA_TICKCOUNT            0x1002
//notifications
#define SLN_CHANGE             1         /* Slider position changed   */
#define SLN_SLIDERTRACK        2         /* Slider dragged by user    */
#define SLN_SETFOCUS           3         /* Slider gaining focus      */
#define SLN_KILLFOCUS          4         /* Slider losing focus       */

#define SLIDER_CLASS "BIFSlider"

BOOL RegisterSlider(HINSTANCE hinst);

#endif
