/***
Filename: fmenu.cpp
Description:
  Implementation of the FMenu class
History:
  ISJ 96-10-13 Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_RESOURCETEMPLATE
#define WMSINCL_HARDWAREMOUSE
#define WMSINCL_BW
#define WMSINCL_MSGS
#include <bif.h>
#include <dbglog.h>
#include <string.h>

#include "../../../source/window/wms/menuwin.h"

static Bool parseMenuSetupString(const char *setup, uint16 *style, uint16 *attr, char *itemtext) {
        //Parse setupstring specified in the resources to produce
        //a menuitem style, attribute and text
        int props;
        char **name;
        char **value;
        if(!DialogResource::SplitSetup(setup,&props,&name,&value))
                return False;
        for(int p=0; p<props; p++) {
                if(stricmp(name[p],"text")==0 && value[p]) {
                        strcpy(itemtext,value[p]);
                } else if(stricmp(name[p],"disabled")==0) {
                        *attr |= MIA_DISABLED;
                } else if(stricmp(name[p],"enabled")==0) {
                        *attr &= ~MIA_DISABLED;
                } else if(stricmp(name[p],"checked")==0) {
                        *attr |= MIA_CHECKED;
                } else if(stricmp(name[p],"ownerdraw")==0) {
                        *style |= MIS_OWNERDRAW;
                } else if(stricmp(name[p],"command")==0) {
                        *style &= ~(MIS_COMMAND|MIS_SYSCOMMAND|MIS_HELP);
                        *style |= MIS_COMMAND;
                } else if(stricmp(name[p],"syscommand")==0) {
                        *style &= ~(MIS_COMMAND|MIS_SYSCOMMAND|MIS_HELP);
                        *style |= MIS_SYSCOMMAND;
                } else if(stricmp(name[p],"help")==0) {
                        *style &= ~(MIS_COMMAND|MIS_SYSCOMMAND|MIS_HELP);
                        *style |= MIS_HELP;
                } else if(stricmp(name[p],"break")==0) {
                        *style |= MIS_BREAK;
                } else if(stricmp(name[p],"static")==0) {
                        *style |= MIS_STATIC;
                } else if(stricmp(name[p],"nodismiss")==0) {
                        *attr |= MIA_NODISMISS;
                } else if(stricmp(name[p],"dismiss")==0) {
                        *attr &= ~MIA_NODISMISS;
                } else if(stricmp(name[p],"separator")==0) {
                        *style |= MIS_SEPARATOR;
                }
        }
        DialogResource::DeallocateSetup(name,value);
        return True;
}

static FWnd *loadMenu(FModule *module, int resId, WmsHWND hwndOwner) {
        //load resource and create a menu
        const MenuItemTemplate *r = (const MenuItemTemplate*)module->LoadMenu(resId);
        if(!r) return 0;

        FMenuWindow *pwnd=new FMenuWindow;
        if(!pwnd) return 0;
        if(!pwnd->Create(0,resId)) {
                delete pwnd;
                return 0;
        }
        WmsSetWindowOwner(pwnd->GetHwnd(),hwndOwner);

        for(;r->mis!=_MIS_END; r++) {
                MENUITEM mi;
                mi.pos = -1;
                mi.style=0;
                mi.attr=0;
                mi.id = (uint16)r->ID;
                mi.submenu = 0;
                mi.handle = 0;
                if(r->mis==_MIS_SUBMENU) {
                        FWnd *pwndSubMenu=loadMenu(module,r->ID,pwnd->GetHwnd());
                        if(!pwndSubMenu) {
                                delete pwnd;
                                return 0;
                        }
                        mi.submenu = pwndSubMenu->GetHwnd();
                        mi.style |= MIS_SUBMENU;
                } 

                char itemtext[256];
                if(!parseMenuSetupString(r->setup,&mi.style,&mi.attr,itemtext)) {
                        delete pwnd;
                        return 0;
                }

                WmsMRESULT mr = WmsSendMsg(pwnd->GetHwnd(),
                                           MM_INSERTITEM,
                                           MPFROMP(&mi),
                                           MPFROMP(itemtext)
                                          );
                if((sint16)UINT161FROMMR(mr)<0) {
                        delete pwnd;
                        return 0;
                }
        }
        return pwnd;
}

FMenu::FMenu(WmsHWND hwndOwner, int resId, FModule *module) {
        if(!module) module=GetCurrentApp();
        rep = new Rep;
        rep->refs=1;
        rep->shouldDelete=True;
        rep->pwnd = loadMenu(module,resId,hwndOwner);
}
FMenu::FMenu(FWnd *pwndOwner, int resId, FModule *module) {
        if(!module) module=GetCurrentApp();
        rep = new Rep;
        rep->refs=1;
        rep->shouldDelete=True;
        rep->pwnd = loadMenu(module,resId,pwndOwner?pwndOwner->GetHwnd():0);
}

FMenu::FMenu(WmsHWND hmenu, Bool d)
  : rep(new Rep)
{
        rep->refs=1;
        rep->shouldDelete = d;
        rep->pwnd = GetWndMan()->FindWnd(hmenu);
}

FMenu::FMenu(const FMenu &m) {
        rep=m.rep;
        rep->refs++;
}

FMenu& FMenu::operator=(const FMenu &m) {
        if(&m==this) return *this;
        if(--rep->refs==0) {
                if(rep->shouldDelete) {
                        if(rep->pwnd)
                                rep->pwnd->Destroy();
                        delete rep->pwnd;
                }
                delete rep;
        }
        rep=m.rep;
        rep->refs++;
        return *this;
}

FMenu::~FMenu() {
        if(--rep->refs==0) {
                if(rep->shouldDelete) {
                        if(rep->pwnd)
                                rep->pwnd->Destroy();
                        delete rep->pwnd;
                }
                delete rep;
        }
}

Bool FMenu::Popup(FWnd *parent, FWnd *owner, const FPoint *position) {
        if(!rep->pwnd) return False;
        if(!rep->pwnd->IsValid()) return False;
        if(!parent) parent=GetDesktop();
        if(owner)
                rep->pwnd->SetOwner(owner);

        {//set menu position
                FPoint p;
                if(position) {
                        p = *position;
                } else {
                        //position not specified - use current pointer pos
                        unsigned x,y;
                        WmsQueryPointerPos(&x,&y);
                        p.Set(x,y);
                        WmsMapWindowPoints(GetDesktop()->GetHwnd(), parent->GetHwnd(), &p, 1);
                }
                FRect mr;
                WmsSetWindowPos(GetHandle(), HWND_TOP, p.GetX(),p.GetY(),0,0, SWP_SIZE|SWP_MOVE|SWP_ZORDER);
        }
        return (Bool)WmsSendMsg(GetHandle(), MM_STARTMENUMODE, MPFROM2UINT16(False,True), 0);
}

Bool FMenu::AppendItem(const char *string, int nId, Bool enabled, Bool checked) {
        MENUITEM mi;
        mi.pos = -1;
        mi.style = MIS_TEXT|MIS_COMMAND;
        mi.attr=0;
        if(!enabled) mi.attr |= MIA_DISABLED;
        if(checked) mi.attr |= MIA_CHECKED;
        mi.id = (uint16)nId;
        mi.submenu = 0;
        mi.handle = 0;
        WmsMRESULT mr=WmsSendMsg(GetHandle(),
                                 MM_INSERTITEM,
                                 MPFROMP(&mi),
                                 MPFROMP(string)
                                );
        if((sint16)UINT161FROMMR(mr)<0)
                return False;
        else
                return True;
}

static uint16 runningMenuId=9999;
Bool FMenu::AppendItem(const char *pszItem, FMenu *submenu, Bool enabled) {
        MENUITEM mi;
        mi.pos = -1;
        mi.style = MIS_TEXT|MIS_SUBMENU;
        mi.attr=0;
        if(!enabled) mi.attr |= MIA_DISABLED;
        mi.id = runningMenuId++;
        mi.submenu = submenu->GetHandle();
        mi.handle = 0;
        WmsMRESULT mr=WmsSendMsg(GetHandle(),
                                 MM_INSERTITEM,
                                 MPFROMP(&mi),
                                 MPFROMP(pszItem)
                                );
        if((sint16)UINT161FROMMR(mr)<0)
                return False;
        else
                return True;
}

void FMenu::AppendSeparator() {
        MENUITEM mi;
        mi.pos = -1;
        mi.style = MIS_SEPARATOR;
        mi.attr = 0;
        mi.id = runningMenuId++;
        mi.submenu = 0;
        mi.handle = 0;
        WmsSendMsg(GetHandle(),
                   MM_INSERTITEM,
                   MPFROMP(&mi),
                   0
                  );
}

Bool FMenu::DeleteItem(int Id) {
        return (Bool)WmsSendMsg(GetHandle(),
                                MM_REMOVEITEM,          //todo:possible memory leak
                                MPFROM2UINT16(Id,True),
                                0
                               );
}

Bool FMenu::GetItem(int Id, char *buf, int buflen) {
        return (Bool)WmsSendMsg(GetHandle(),
                                MM_QUERYITEMTEXT,
                                MPFROM2UINT16(Id,buflen),
                                MPFROMP(buf)
                               );
}

FMenu FMenu::GetSubmenuFromPos(int nPos) {
        WmsMRESULT mr;

        mr=WmsSendMsg(GetHandle(),MM_ITEMIDFROMPOSITION,MPFROMUINT16(nPos),0);
        sint16 id=(sint16)UINT161FROMMR(mr);
        if(id<0) {
                FATAL("FMenu::GetSubmenuFromPos: no such submenu");
        }

        MENUITEM mi;
        WmsSendMsg(GetHandle(), MM_QUERYITEM, MPFROM2UINT16(id,False), MPFROMP(&mi));
        if((mi.style&MIS_SUBMENU)==0 ||
           mi.submenu==0) {
                FATAL("FMenu::GetSubmenuFromPos: item is not a submenu");
        }
        return FMenu(mi.submenu,False);
}

void FMenu::CheckItem(int Id, Bool check) {
        if(check)
                WmsSendMsg(GetHandle(), MM_SETITEMATTR, MPFROM2UINT16(Id,True), MPFROM2UINT16(MIA_CHECKED,MIA_CHECKED));
        else
                WmsSendMsg(GetHandle(), MM_SETITEMATTR, MPFROM2UINT16(Id,True), MPFROM2UINT16(MIA_CHECKED,0));
}

void FMenu::EnableItem(int Id, Bool enable) {
        if(enable)
                WmsSendMsg(GetHandle(), MM_SETITEMATTR, MPFROM2UINT16(Id,True), MPFROM2UINT16(MIA_DISABLED,0));
        else
                WmsSendMsg(GetHandle(), MM_SETITEMATTR, MPFROM2UINT16(Id,True), MPFROM2UINT16(MIA_DISABLED,MIA_DISABLED));
}

Bool FMenu::IsItemEnabled(int Id) {
        WmsMRESULT mr=WmsSendMsg(GetHandle(),MM_QUERYITEMATTR,MPFROM2UINT16(Id,True),0);
        return (Bool)((UINT161FROMMR(mr)&MIA_DISABLED)==0);
}

Bool FMenu::IsItemChecked(int Id) {
        WmsMRESULT mr=WmsSendMsg(GetHandle(),MM_QUERYITEMATTR,MPFROM2UINT16(Id,True),0);
        return (Bool)((UINT161FROMMR(mr)&MIA_CHECKED)!=0);
}

WmsHWND FMenu::GetHandle() {
        if(rep->pwnd)
                return rep->pwnd->GetHwnd();
        else
                return 0;
}


void FMenu::TurnIntoActionBar() {
        if(!rep->pwnd) return;
        WmsSetWindowStyleBits(GetHandle(), MS_ACTIONBAR, MS_ACTIONBAR);
}

void FMenu::EnterActionBar() {
        if(!rep->pwnd) return;
        WmsSendMsg(GetHandle(), MM_STARTMENUMODE, MPFROM2UINT16(False,True), 0);
}

