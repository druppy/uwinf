/***
Filename: mle.cpp
Description:
  Implementation of FMLE (Multi-Line-Eddit)
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_MLE
#include <bif.h>
#include <string.h>

Bool FMLE::Create(FWnd *pParent, int ID, FRect *rect, long style, const char *pszText) {
        ULONG flStyle=0;
        if(style&mle_hscroll)
                flStyle|=MLS_HSCROLL;
        if(style&mle_vscroll)
                flStyle|=MLS_VSCROLL;
        if(style&mle_readonly)
                flStyle|=MLS_READONLY;
        if(style&mle_border)
                flStyle|=MLS_BORDER;

        return FControl::Create(pParent,ID,
                                WC_MLE,pszText,
                                flStyle,rect);
}



//clipboard operations:
Bool FMLE::Cut() {
        WinSendMsg(GetHwnd(),MLM_CUT,0,0);
        return True;
}

Bool FMLE::Copy() {
        WinSendMsg(GetHwnd(),MLM_COPY,0,0);
        return True;
}
Bool FMLE::Clear() {
        WinSendMsg(GetHwnd(),MLM_CLEAR,0,0);
        return True;
}

Bool FMLE::Paste() {
        WinSendMsg(GetHwnd(),MLM_PASTE,0,0);
        return True;
}

//selection operations:
Bool FMLE::ReplaceSelection(const char *text) {
        WinSendMsg(GetHwnd(),MLM_INSERT,MPFROMP(text),0);
        return True;
}


//modify flag
Bool FMLE::HasBeenModified() {
        return (Bool)WinSendMsg(GetHwnd(),MLM_QUERYCHANGED,0,0);
}

Bool FMLE::SetModify(Bool modified) {
        WinSendMsg(GetHwnd(),MLM_SETCHANGED,MPFROMSHORT(modified),0);
        return True;
}


//text
int FMLE::GetTextLen() {
        SetFormat(MLFIE_CFTEXT);
        return LONGFROMMR(WinSendMsg(GetHwnd(),MLM_QUERYFORMATTEXTLENGTH,MPFROMSHORT(0),MPFROMLONG(0xffffffff)));
}

Bool FMLE::GetText(char *buffer, int maxbuf) {
        if(!SetImportExport(buffer,maxbuf))
                return False;
        ULONG bytes=GetTextLen();
        if(bytes>maxbuf)
                return False;
        IPT ipt=0;
        ULONG bytesExported=0xffff;     //max 64kb
        ULONG ulSuccess=Export(&ipt,&bytesExported);
        if(ulSuccess<bytes)
                return False;
        else
                return True;
}

Bool FMLE::SetText(const char *text) {
        SetFormat(MLFIE_CFTEXT);
        if(!SetImportExport((char*)text,strlen(text)))
                return False;
        //delete everything
        WinSendMsg(GetHwnd(),MLM_DELETE,MPFROMLONG(0),MPFROMLONG(0xfffffffe));
        //import the text
        IPT ipt=0;
        ULONG ulCopy=strlen(text);
        Import(&ipt,ulCopy);
        return True;   
}

Bool FMLE::LimitText(int limit) {
        ULONG ulFit=LONGFROMMR(WinSendMsg(GetHwnd(),MLM_SETTEXTLIMIT,MPFROMLONG(limit),0));
        if(ulFit==0)
                return True;
        else
                return False;
}


//readonly
Bool FMLE::IsReadonly() {
        return (Bool)WinSendMsg(GetHwnd(),MLM_QUERYREADONLY,0,0);
}

Bool FMLE::SetReadonly(Bool f) {
        WinSendMsg(GetHwnd(),MLM_SETREADONLY,MPFROMSHORT(f),0);
        return True;
}



//special os/2 operations:

void FMLE::SetFormat(USHORT usFormat) {
        WinSendMsg(GetHwnd(),MLM_FORMAT,MPFROMSHORT(usFormat),0);
}

Bool FMLE::SetImportExport(char *buffer, int bufferlength) {
        return (Bool)WinSendMsg(GetHwnd(),MLM_SETIMPORTEXPORT,MPFROMP(buffer),MPFROMLONG(bufferlength));
}

ULONG FMLE::Import(IPT *pipt, ULONG ulCopy) {
        return LONGFROMMR(WinSendMsg(GetHwnd(),MLM_IMPORT,MPFROMP(pipt),MPFROMLONG(ulCopy)));
}

ULONG FMLE::Export(IPT *pipt, ULONG *pCopy) {
        return LONGFROMMR(WinSendMsg(GetHwnd(),MLM_EXPORT,MPFROMP(pipt),MPFROMP(pCopy)));
}

