#ifdef __OS2__
#define INCL_KBD
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSMISC
#endif

#define WMSINCL_TIMERSTAMP
#include "wms.hpp"

#include "hwkeybrd.hpp"
#include "kbdstate.hpp"
#include "sysqueue.hpp"
#include "logkey.hpp"

#include <string.h>

#if defined(__DOS__) || defined(__OS2__)
//keyboard mapper is the same for dos and os2

struct fullmapping {
       uint16 charscan;
       logical_key lk;
       uint8 flags;
};

static fullmapping fm[] = {
//esc: (shift and ctrl is ignored)
{0x1b01, lk_esc        ,skef_key },                                                     {0x0001, lk_esc        ,skef_key },
//F1..F12
{0x003b, lk_f1         ,skef_key }, {0x0054, lk_f1         ,skef_key }, {0x005e, lk_f1         ,skef_key }, {0x0068, lk_f1         ,skef_key },
{0x003c, lk_f2         ,skef_key }, {0x0055, lk_f2         ,skef_key }, {0x005f, lk_f2         ,skef_key }, {0x0069, lk_f2         ,skef_key },
{0x003d, lk_f3         ,skef_key }, {0x0056, lk_f3         ,skef_key }, {0x0060, lk_f3         ,skef_key }, {0x006a, lk_f3         ,skef_key },
{0x003e, lk_f4         ,skef_key }, {0x0057, lk_f4         ,skef_key }, {0x0061, lk_f4         ,skef_key }, {0x006b, lk_f4         ,skef_key },
{0x003f, lk_f5         ,skef_key }, {0x0058, lk_f5         ,skef_key }, {0x0062, lk_f5         ,skef_key }, {0x006c, lk_f5         ,skef_key },
{0x0040, lk_f6         ,skef_key }, {0x0059, lk_f6         ,skef_key }, {0x0063, lk_f6         ,skef_key }, {0x006d, lk_f6         ,skef_key },
{0x0041, lk_f7         ,skef_key }, {0x005a, lk_f7         ,skef_key }, {0x0064, lk_f7         ,skef_key }, {0x006e, lk_f7         ,skef_key },
{0x0042, lk_f8         ,skef_key }, {0x005b, lk_f8         ,skef_key }, {0x0065, lk_f8         ,skef_key }, {0x006f, lk_f8         ,skef_key },
{0x0043, lk_f9         ,skef_key }, {0x005c, lk_f9         ,skef_key }, {0x0066, lk_f9         ,skef_key }, {0x0070, lk_f9         ,skef_key },
{0x0044, lk_f10        ,skef_key }, {0x005d, lk_f10        ,skef_key }, {0x0067, lk_f10        ,skef_key }, {0x0071, lk_f10        ,skef_key },
{0x0085, lk_f11        ,skef_key }, {0x0087, lk_f11        ,skef_key }, {0x0089, lk_f11        ,skef_key }, {0x008b, lk_f11        ,skef_key },
{0x0086, lk_f12        ,skef_key }, {0x0088, lk_f12        ,skef_key }, {0x008a, lk_f12        ,skef_key }, {0x008c, lk_f12        ,skef_key },
//ALT-1-0 (if other modifiers are used the logical key is unknown)
{0x0078, lk_1          ,skef_key }, {0x0079, lk_2          ,skef_key }, {0x007a, lk_3          ,skef_key }, {0x007b, lk_4          ,skef_key },
{0x007c, lk_5          ,skef_key }, {0x007d, lk_6          ,skef_key }, {0x007e, lk_7          ,skef_key }, {0x007f, lk_8          ,skef_key },
{0x0080, lk_9          ,skef_key }, {0x0081, lk_0          ,skef_key },
//ALT-a-z is not in this table because it would not work well with different
//keyboard layouts. Even the QWERTY layout has variations (eg: Y and Z is
//swapped on the German version).
{0x080e, lk_backspace  ,skef_key },                                     {0x7f0e, lk_backspace  ,skef_key }, {0x000e, lk_backspace  ,skef_key },
//tab
{0x090f, lk_tab        ,skef_key }, {0x000f, lk_tab        ,skef_key }, {0x0094, lk_tab        ,skef_key }, {0x00a5, lk_tab        ,skef_key },
//enter
{0x0d1c, lk_enter      ,skef_key },                                     {0x0a1c, lk_enter      ,skef_key }, {0x001c, lk_enter      ,skef_key },
//insert
{0xe052, lk_insert     ,skef_key },                                     {0xe092, lk_insert     ,skef_key }, {0x00a2, lk_insert     ,skef_key },
//home
{0xe047, lk_home       ,skef_key },                                     {0xe077, lk_home       ,skef_key }, {0x0097, lk_home       ,skef_key },
//pageup
{0xe049, lk_pageup     ,skef_key },                                     {0xe084, lk_pageup     ,skef_key }, {0x0099, lk_pageup     ,skef_key },
//del
{0xe053, lk_delete     ,skef_key },                                     {0xe093, lk_delete     ,skef_key }, {0x00a3, lk_delete     ,skef_key },
//end
{0xe04f, lk_end        ,skef_key },                                     {0xe075, lk_end        ,skef_key }, {0x009f, lk_end        ,skef_key },
//pagedown
{0xe051, lk_pagedown   ,skef_key },                                     {0xe076, lk_pagedown   ,skef_key }, {0x00a1, lk_pagedown   ,skef_key },
//arrows
//up (shift is ignored)
{0xe048, lk_up         ,skef_key },                                     {0xe08d, lk_up         ,skef_key }, {0x0098, lk_up         ,skef_key },
//left (shift is ignored)
{0xe04b, lk_left       ,skef_key },                                     {0xe073, lk_left       ,skef_key }, {0x009b, lk_left       ,skef_key },
//down (shift is ignored)
{0xe050, lk_down       ,skef_key },                                     {0xe091, lk_down       ,skef_key }, {0x00a0, lk_down       ,skef_key },
//right (shift is ignored)
{0xe04d, lk_right      ,skef_key },                                     {0xe074, lk_right      ,skef_key }, {0x009d, lk_right      ,skef_key },
//keypad with numlock on
//0..9 (no alt due to alt-nnn feature)
{0x3052, lk_num0       ,skef_char}, {0x0052, lk_insert     ,skef_key }, {0x0092, lk_insert     ,skef_key },
{0x314f, lk_num1       ,skef_char}, {0x004f, lk_end        ,skef_key }, {0x0075, lk_end        ,skef_key },
{0x3250, lk_num2       ,skef_char}, {0x0050, lk_down       ,skef_key }, {0x0091, lk_down       ,skef_key },
{0x3351, lk_num3       ,skef_char}, {0x0051, lk_pagedown   ,skef_key }, {0x0076, lk_pagedown   ,skef_key },
{0x344b, lk_num4       ,skef_char}, {0x004b, lk_left       ,skef_key }, {0x0073, lk_left       ,skef_key },
{0x354c, lk_num5       ,skef_char}, {0x004c, lk_num5       ,skef_key }, {0x008f, lk_num5       ,skef_key },
{0x364d, lk_num6       ,skef_char}, {0x004d, lk_right      ,skef_key }, {0x0074, lk_right      ,skef_key },
{0x3747, lk_num7       ,skef_char}, {0x0047, lk_home       ,skef_key }, {0x0077, lk_home       ,skef_key },
{0x3848, lk_num8       ,skef_char}, {0x0048, lk_up         ,skef_key }, {0x008d, lk_up         ,skef_key },
{0x3949, lk_num9       ,skef_char}, {0x0049, lk_pageup     ,skef_key }, {0x0084, lk_pageup     ,skef_key },
//slash (shift is ignored)
{0x2fe0, lk_numslash   ,skef_char},                                     {0x0095, lk_numslash   ,skef_key }, {0x00a4, lk_numslash   ,skef_key },
//asterisk (shift is ignored) (and dead under os/2)
{0x2a37, lk_numasterisk,skef_char},                                     {0x0096, lk_numasterisk,skef_key }, {0x0037, lk_numasterisk,skef_key },
//minus (shift is ignored)
{0x2d4a, lk_numminus   ,skef_char},                                     {0x008e, lk_numminus   ,skef_key }, {0x004a, lk_numminus   ,skef_key },
//plus (shift is ignored)
{0x2b4e, lk_numplus    ,skef_char},                                     {0x0090, lk_numplus    ,skef_key }, {0x004e, lk_numplus    ,skef_key },
//enter (shift is ignored)
{0x0de0, lk_numenter   ,skef_key },                                     {0x0ae0, lk_numenter   ,skef_key }, {0x00a6, lk_numenter   ,skef_key },
};

/* note:
 * Under OS/2 shift-asterisk is a dead key and the session manager grabs
 * some of the keycodes (such as ctrl-esc, alt-home, ...)
 * and the 102th key on DK keyboard (half and paragraph sign) is a dead key ?!?
 */

static void TranslateKeystroke(uint8 c, uint8 s, logical_key *lk, uint8 *flags) {
        uint16 charscan = (uint16)((((uint16)c)<<8)|s);
        for(unsigned i=0; i<sizeof(fm)/sizeof(fm[0]); i++) {
                if(fm[i].charscan==charscan) {
                        *lk=fm[i].lk;
                        *flags=fm[i].flags;
                        return;
                }
        }
        //no table match on "special" keys
        //Try algorithmic match
        if(s==0) {
                //if the key is unknown is probably an alt-nnn or combination (such as u-umlaut)
                *lk = lk_unknown;
                *flags = skef_char;
                return;
        }
        //standard maping for 0..9 a..z A..Z ^@..^Z
        if(c>='0' && c<='9') {
                switch(c) {
                        case '0': *lk=lk_0; break;
                        case '1': *lk=lk_1; break;
                        case '2': *lk=lk_2; break;
                        case '3': *lk=lk_3; break;
                        case '4': *lk=lk_4; break;
                        case '5': *lk=lk_5; break;
                        case '6': *lk=lk_6; break;
                        case '7': *lk=lk_7; break;
                        case '8': *lk=lk_8; break;
                        case '9': *lk=lk_9; break;
                }
                *flags = skef_char;
                return;
        }
        if(c>='a' && c<='z') {
                *lk = logical_key(c-'a'+lk_a);
                *flags = skef_char;
                return;
        }
        if(c>='A' && c<='Z') {
                *lk = logical_key(c-'A'+lk_a);
                *flags = skef_char;
                return;
        }
        if(c>=1 && c<=26) {
                *lk = logical_key(c-1+lk_a);
                *flags = skef_char;
                return;
        }
        //if the character is 0x00 or 0xe0 it is most likely a key
        *lk = lk_unknown;
        if(c==0x00 || c==0xe0) {
                if(c==0x00 && s==0x03) {
                        *flags = skef_char;     //ctrl-2 is a special case
                } else if(c==0x00) {
                        *flags = skef_key;      //most likely ALT-x
                } else if(s>=0x02 && s<=0x35) {
                        //main block key
                        //probably a char (must be using Greek keyboard)
                        *flags = skef_char;
                } else
                        *flags = skef_key;
        } else
                *flags = skef_char;
}

#endif /*__DOS__ or __OS2__ */


//---DOS-----------------------------------------------------------------------
#if (BIFOS_ == BIFOS_DOS16_) || (BIFOS_ == BIFOS_DOS32_)
#include <bios.h>
#define WMSINCL_TIMESTAMP
#include <wms.hpp>
#include "../dosrmode.hpp"
class KBDReaderThread : public FThread {
public:
        volatile int please_terminate;
        KBDReaderThread() : FThread(), please_terminate(0) {}

        void Go();
};

struct ScanToLK_map {
        uint8 scan;
        logical_key lk;
};

static ScanToLK_map kbd_english[] = {
        {0x10,lk_q}, {0x11,lk_w}, {0x12,lk_e}, {0x13,lk_r}, {0x14,lk_t},
        {0x15,lk_y}, {0x16,lk_u}, {0x17,lk_i}, {0x18,lk_o}, {0x19,lk_p},
        {0x1e,lk_a}, {0x1f,lk_s}, {0x20,lk_d}, {0x21,lk_f}, {0x22,lk_g},
        {0x23,lk_h}, {0x24,lk_j}, {0x25,lk_k}, {0x26,lk_l},
        {0x2c,lk_z}, {0x2d,lk_x}, {0x2e,lk_c}, {0x2f,lk_v}, {0x30,lk_b},
        {0x31,lk_n}, {0x32,lk_m}
};
static ScanToLK_map kbd_german[] = {
        {0x10,lk_q}, {0x11,lk_w}, {0x12,lk_e}, {0x13,lk_r}, {0x14,lk_t},
        {0x15,lk_z}, {0x16,lk_u}, {0x17,lk_i}, {0x18,lk_o}, {0x19,lk_p},
        {0x1e,lk_a}, {0x1f,lk_s}, {0x20,lk_d}, {0x21,lk_f}, {0x22,lk_g},
        {0x23,lk_h}, {0x24,lk_j}, {0x25,lk_k}, {0x26,lk_l},
        {0x2c,lk_y}, {0x2d,lk_x}, {0x2e,lk_c}, {0x2f,lk_v}, {0x30,lk_b},
        {0x31,lk_n}, {0x32,lk_m}
};
static ScanToLK_map kbd_french[] = {
        {0x10,lk_a}, {0x11,lk_z}, {0x12,lk_e}, {0x13,lk_r}, {0x14,lk_t},
        {0x15,lk_y}, {0x16,lk_u}, {0x17,lk_i}, {0x18,lk_o}, {0x19,lk_p},
        {0x1e,lk_q}, {0x1f,lk_s}, {0x20,lk_d}, {0x21,lk_f}, {0x22,lk_g},
        {0x23,lk_h}, {0x24,lk_j}, {0x25,lk_k}, {0x26,lk_l},
        {0x2c,lk_w}, {0x2d,lk_x}, {0x2e,lk_c}, {0x2f,lk_v}, {0x30,lk_b},
        {0x31,lk_n}, {0x32,lk_m}
};

//The default keyboard is english. Since we only map a-z this works for
//several keyboard layouts suchs as US, Brazilian, Canadian-French, Danish, etc.
#if BIFOS_==BIFOS_DOS16_
static void mapScancodeToLK(uint8 scancode, logical_key *lk) {
        static ScanToLK_map *map=0;
        if(map==0) {
                REGS regs;
                SREGS sregs;
                uint8 buf[39];
                regs.h.ah = 0x65; //get extended country info
                regs.h.al = 0x01; //get general info
                regs.x.bx = 0xffff; //global codepage
                regs.x.dx = 0xffff; //current country
                sregs.es = FP_SEG(buf);
                regs.x.di = FP_OFF(buf);
                regs.x.cx = 39;
                intdosx(&regs,&regs,&sregs);
                if(!regs.x.cflag) {
                        uint16 country = *((uint16*)(buf+3));
                        if(country==49 ||
                           country==36 ||
                           country==41 ||
                           country==48)
                                map = kbd_german;
                        else if(country==33 || country==32)
                                map = kbd_french;
                        else
                                map = kbd_english;
                } else
                        map=kbd_english;
        }
        for(int i=0; i<26; i++) {
                if(map[i].scan == scancode) {
                        *lk = map[i].lk;
                        return;
                }
        }
}
#else
//todo: realmode memory must be allocated
static void mapScancodeToLK(uint8, logical_key *) {
}
#endif

void KBDReaderThread::Go() {
        //The ideal solution instead of polling constantly would be to hook
        //interrupt 9 (called when something happens on the keyboard) and then
        //wake this thread. But that could cause problems under OS's that does
        //not simulate int9, or "keyboard stuffes" which inserts keystrokes into
        //the keyboard buffer and thus not simulating an int9
        while(!please_terminate) {
                short k=_bios_keybrd(_NKEYBRD_READY);
                if(k) {
                        k=_bios_keybrd(_NKEYBRD_READ);
                        SystemEvent se;
                        se.type = SystemEvent::keyboard;
                        se.u.ske.character = (char)(k);
                        se.u.ske.scancode = (char)(k>>8);
                        logical_key lk;
                        TranslateKeystroke((char)(k), (char)(k>>8),
                                           &lk,
                                           &se.u.ske.flags);
                        se.u.ske.logicalkey=lk;
                        se.keyboardShiftState = hwkeyboard_internal.QueryCurrentShiftState();
                        se.timestamp = WmsQueryTimeStamp();
                        if(lk==lk_unknown) {
                                //try to map a-z
                                mapScancodeToLK(se.u.ske.scancode,&lk);
                                se.u.ske.logicalkey=lk;
                        }
                        systemEventQueue_internal.put(&se, 100);
                } else {
                        //No keystroke available
                        //Since the fastest repeat rate is 30 chars/sec, we
                        //we can safely sleep for at least 33ms, but not too
                        //long to ensure responsiveness
                        Sleep(100);
                }
        }
}

static KBDReaderThread *readerThread=0;

HWKeyboard::HWKeyboard() {
}

HWKeyboard::~HWKeyboard() {
}

int HWKeyboard::Initialize() {
        //start reader thread
        readerThread = new KBDReaderThread;
        if(!readerThread) return -1;
        if(readerThread->Start() != 0) {
                delete readerThread;
                readerThread = 0;
                return -2;
        }
        return 0;
}

void HWKeyboard::Terminate() {
        //stop reader thread
        if(readerThread) {
                readerThread->please_terminate=1;
                readerThread->Wait();
                delete readerThread;
                readerThread=0;
        }
}

uint8 HWKeyboard::QueryCurrentShiftState() {
        unsigned char status1 = *(Ptr_0040_Type(volatile unsigned char))Ptr_0040(0x0017);
        //unsigned char status2 = *(Ptr_0040_Type(volatile unsigned char))Ptr_0040(0x0018);
        uint8 r=0;
        if(status1&0x03) r |= kss_shift;
        if(status1&0x04) r |= kss_ctrl;
        if(status1&0x08) r |= kss_alt;
        if(status1&0x40) r |= kss_capslock;
        if(status1&0x20) r |= kss_numlock;
        if(status1&0x10) r |= kss_scrolllock;
        return r;
}

//---OS/2----------------------------------------------------------------------
#elif (BIFOS_ == BIFOS_OS2_)

static uint8 translateShiftState(USHORT fsState) {
        uint8 r=0;
        if(fsState&0x03) r |= kss_shift;
        if(fsState&0x04) r |= kss_ctrl;
        if(fsState&0x08) r |= kss_alt;
        if(fsState&0x40) r |= kss_capslock;
        if(fsState&0x20) r |= kss_numlock;
        if(fsState&0x10) r |= kss_scrolllock;
        return r;
}

class KBDReaderThread : public FThread {
public:
        volatile int please_terminate;
        FEventSemaphore terminating;
        KBDReaderThread() : FThread(), please_terminate(0), terminating() {}

        void Go();
};

void KBDReaderThread::Go() {
        while(!please_terminate) {
                KBDKEYINFO kki;
                APIRET rc = KbdCharIn(&kki, 0, 0);
                if(please_terminate) break;
                if(rc==0) {
                        SystemEvent se;
                        se.type = SystemEvent::keyboard;
                        se.u.ske.character = kki.chChar;
                        se.u.ske.scancode = kki.chScan;
                        logical_key lk;
                        TranslateKeystroke(kki.chChar, kki.chScan,
                                           &lk,
                                           &se.u.ske.flags);
                        se.u.ske.logicalkey=lk;
                        se.keyboardShiftState = translateShiftState(kki.fsState);
                        se.timestamp = kki.time;
                        if(lk==lk_unknown) {
                                //try to map ALT-A-Z
                                KBDTRANS kt;
                                memset(&kt,0,sizeof(kt));
                                *((KBDKEYINFO*)(&kt)) = kki;
                                kt.fsState = 0; //unmodified key
                                rc = KbdXlate(&kt, 0);
                                if(rc==0) {
                                        if(kt.chChar>='A' && kt.chChar<='Z')
                                                se.u.ske.logicalkey = (logical_key)(lk_a+kt.chChar-'A');
                                        if(kt.chChar>='a' && kt.chChar<='z')
                                                se.u.ske.logicalkey = (logical_key)(lk_a+kt.chChar-'a');
                                        if(kt.chChar>='1' && kt.chChar<='9')
                                                se.u.ske.logicalkey = (logical_key)(lk_1+kt.chChar-'1');
                                        if(kt.chChar=='0')
                                                se.u.ske.logicalkey = lk_0;
                                }
                        }
                        systemEventQueue_internal.put(&se, 100);
                }
        }
        terminating.Post();
}

static KBDReaderThread *readerThread=0;
static HFILE hfKeyboard=0;      //handle used for querying shift state

HWKeyboard::HWKeyboard() {
}

HWKeyboard::~HWKeyboard() {
}

int HWKeyboard::Initialize() {
        //open keyboard handle for shift state query
        ULONG action;
        APIRET rc=DosOpen("KBD$",
                          &hfKeyboard,
                          &action,
                          0,
                          0,
                          OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
                          OPEN_SHARE_DENYNONE|OPEN_ACCESS_READONLY,
                          0
                         );
        //errors are ignored
        if(rc)
                hfKeyboard=0;

        //start reader thread
        readerThread = new KBDReaderThread;
        if(!readerThread) return -1;
        if(readerThread->Start() != 0) {
                delete readerThread;
                readerThread = 0;
                return -2;
        }
        return 0;
}

void HWKeyboard::Terminate() {
        //stop reader thread
        if(readerThread) {
                readerThread->please_terminate=1;
                // The Kbd subsystem is buggy under OS/2 2.0-2.11, so the
                // kbdreader thread is not terminated unless we are running 
                // Warp or later
                ULONG u[2];
                DosQuerySysInfo(QSV_VERSION_MAJOR,QSV_VERSION_MINOR,u,sizeof(u[0])*2);
                if(u[0]>20 || (u[0]==20 && u[1]>30)) {
                        if(readerThread->terminating.Wait(200)!=0) {
                                readerThread->Kill();
                        }
                        readerThread->Wait();
                        delete readerThread;
                }
                readerThread=0;
        }
        if(hfKeyboard) {
                HFILE hf=hfKeyboard;
                hfKeyboard=0;
                DosClose(hf);
        }
}

uint8 HWKeyboard::QueryCurrentShiftState() {
/* This code doesn't work, because other threads blocks on it:
        KBDINFO kbdinfo;
        kbdinfo.cb = sizeof(kbdinfo);
        KbdGetStatus(&kbdinfo, 0);
        return translateShiftState(kbdinfo.fsState);
*/
        if(hfKeyboard==0) return 0;     //shift state unknown

        ULONG paramLen=0;
        SHIFTSTATE st;
        ULONG datalen=sizeof(st);
        APIRET rc = DosDevIOCtl(hfKeyboard,
                                IOCTL_KEYBOARD, KBD_GETSHIFTSTATE,
                                0, 0, &paramLen,
                                &st, sizeof(st), &datalen
                               );
        if(rc)
                return 0;       //shift state unknown

        return translateShiftState(st.fsState);
}

#else
#error unknow OS
#endif

