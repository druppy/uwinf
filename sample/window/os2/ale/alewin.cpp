#define INCL_GPI
#define BIFINCL_STDFILEDIALOG
#define BIFINCL_MESSAGEBOX
#include "alewin.hpp"
#include "ale.mid"

#include "editlink.hpp"
#include "readbmp.hpp"
#include "linkio.hpp"

#include <mem.h>
#include <string.h>
#include <stdlib.h>

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

void ALEClientWindow::invalidateLink(int i) {
        FRect r;
        r.SetLeft(la[i].x);
        r.SetBottom(la[i].y);
        r.SetRight(la[i].x+la[i].cx);
        r.SetTop(la[i].y+la[i].cy);
        Invalidate(&r,False);
}

int ALEClientWindow::linkFromPoint(const FPoint &p) {
        for(int i=0; i<la.count(); i++) {
                if(p.GetX()>=la[i].x && p.GetX()<la[i].x+la[i].cx &&
                   p.GetY()>=la[i].y && p.GetY()<la[i].y+la[i].cy)
                {
                        return i;
                }
        }
        return -1;
}

void ALEClientWindow::setCurrentLink(int i) {
        if(currentLink>=0)
                invalidateLink(currentLink);
        currentLink=i;
        if(currentLink>=0)
                invalidateLink(currentLink);
}

ALEClientWindow::ALEClientWindow(FSizeRestrictionFrameHandler *srfh)
  : FClientWindow(),
    FCommandHandler(this),
    FCloseHandler(this),
    FTranslatedMouseHandler(this),
    FPaintHandler(this),
    FSizeRestrictionClientHandler(this,srfh)
{
        hbm=0;
        hpsBitmap=NULL;

        currentLink=-1;

        bitmapfilename[0]='\0';
        linkfilename[0]='\0';
        changed=False;

        creationInProgress=False;
}


ALEClientWindow::~ALEClientWindow() {
        if(hbm) {
                GpiSetBitmap(hpsBitmap,NULLHANDLE);
                GpiDeleteBitmap(hbm);
        }
        if(hpsBitmap) GpiDestroyPS(hpsBitmap);
        if(hdcBitmap) DevCloseDC(hdcBitmap);
}

Bool ALEClientWindow::Create(FAppWindow *pParent) {
        if(!FClientWindow::Create(pParent))
                return False;
        //create the memory DC and PS
        DEVOPENSTRUC dos={NULL, "DISPLAY", NULL, NULL, NULL, NULL, NULL, NULL, NULL};
        hdcBitmap = DevOpenDC(GetWndMan()->GetHAB(),
                              OD_MEMORY, 
                              "*", 
                              5, 
                              (PDEVOPENDATA)&dos, 
                              NULLHANDLE);
        if(!hdcBitmap)
                return False;

        //create the PS
        SIZEL sizel={0,0};
        hpsBitmap = GpiCreatePS(GetWndMan()->GetHAB(), hdcBitmap, &sizel, GPIA_ASSOC|PU_PELS);
        if(!hpsBitmap)
                      return False;
        return True;
}


void ALEClientWindow::ChangeLink(const Link &newLink) {
        //invalidate previous and new rectangle, so the user can see where the link is
        invalidateLink(currentLink);
        //update table
        la[currentLink] = newLink;
        invalidateLink(currentLink);
}


//size restrictions ---
Bool ALEClientWindow::GetMinimumWidth(int &w) {
        if(hbm) {
                w=bitmapWidth;
                return True;
        } else
                return False;
}
Bool ALEClientWindow::GetMinimumHeight(int &h) {
        if(hbm) {
                h=bitmapHeight;
                return True;
        } else
                return False;
}
Bool ALEClientWindow::GetMaximumWidth(int &w) {
        if(hbm) {
                w=bitmapWidth;
                return True;
        } else
                return False;
}
Bool ALEClientWindow::GetMaximumHeight(int &h) {
        if(hbm) {
                h=bitmapHeight;
                return True;
        } else
                return False;
}


//commands---
Bool ALEClientWindow::Command(FCommandEvent &ev) {
        switch(ev.GetItem()) {
                case MID_OPENFILE: {
                        LoadFile();
                        return True;
                }
                case MID_SAVEFILE: {
                        SaveFile();
                        return True;
                }
                case MID_EDITLINK: {
                        editCurrentLink();
                        return True;
                }
                case MID_NEXTLINK: {
                        if(la.count()>0) {
                                if(currentLink==-1)
                                        setCurrentLink(0);
                                else if(currentLink<la.count()-1)
                                        setCurrentLink(currentLink+1);
                                else
                                        setCurrentLink(0);
                        }
                        return True;
                }
                case MID_PREVLINK: {
                        if(currentLink==-1 || currentLink==0)
                                setCurrentLink(la.count()-1);
                        else
                                setCurrentLink(currentLink-1);
                        return True;
                }
                case MID_MOVELINKLEFT: {
                        if(currentLink>=0 && la[currentLink].x>0) {
                                invalidateLink(currentLink);
                                la[currentLink].x--;
                                invalidateLink(currentLink);
                                changed=True;
                        }
                        return True;
                }
                case MID_MOVELINKRIGHT: {
                        if(currentLink>=0) {
                                invalidateLink(currentLink);
                                la[currentLink].x++;
                                invalidateLink(currentLink);
                                changed=True;
                        }
                        return True;
                }
                case MID_MOVELINKUP: {
                        if(currentLink>=0) {
                                invalidateLink(currentLink);
                                la[currentLink].y++;
                                invalidateLink(currentLink);
                                changed=True;
                        }
                        return True;
                }
                case MID_MOVELINKDOWN: {
                        if(currentLink>=0 && la[currentLink].y>0) {
                                invalidateLink(currentLink);
                                la[currentLink].y--;
                                invalidateLink(currentLink);
                                changed=True;
                        }
                        return True;
                }

                case MID_BROADER: {
                        if(currentLink>=0) {
                                invalidateLink(currentLink);
                                la[currentLink].cx++;
                                invalidateLink(currentLink);
                                changed=True;
                        }
                        return True;
                }
                case MID_NARROWER: {
                        if(currentLink>=0 && la[currentLink].cx>1) {
                                invalidateLink(currentLink);
                                la[currentLink].cx--;
                                invalidateLink(currentLink);
                                changed=True;
                        }
                        return True;
                }
                case MID_TALLER: {
                        if(currentLink>=0) {
                                invalidateLink(currentLink);
                                la[currentLink].cy++;
                                invalidateLink(currentLink);
                                changed=True;
                        }
                        return True;
                }
                case MID_SHORTER: {
                        if(currentLink>=0 && la[currentLink].cy>1) {
                                invalidateLink(currentLink);
                                la[currentLink].cy--;
                                invalidateLink(currentLink);
                                changed=True;
                        }
                        return True;
                }


                case MID_DELETELINK: {
                        if(currentLink>=0) {
                                int l=currentLink;
                                setCurrentLink(-1);
                                la.remove(l);
                                changed=True;
                        }
                        return True;
                }
                case MID_DELETEALLLINKS: {
                        la.clear();     //zap all links
                        Invalidate(0);  //invalidate whole window
                        changed=True;
                        return True;
                }
                default: 
                        return False;
        }
}


Bool ALEClientWindow::Paint(FPaintEvent &) {
        FRect pr;
        HPS hps=WinBeginPaint(GetHwnd(),NULL,&pr);
        if(hps) {
                if(hbm) {
                        //draw the bitmap
                        FPoint p(pr.GetLeft(),pr.GetBottom());
                        WinDrawBitmap(hps,
                                      hbm,
                                      &pr,
                                      &p,
                                      0,
                                      0,
                                      DBM_NORMAL|DBM_IMAGEATTRS
                                     );
                } else {
                        //no bitmap loaded - make the window white
                        WinFillRect(hps,&pr,CLR_WHITE);
                }

                //draw link rectangles
                for(int i=0; i<la.count(); i++) {
                        FRect r(la[i].x,la[i].y,la[i].x+la[i].cx,la[i].y+la[i].cy);
                        WinDrawBorder(hps,
                                      &r,
                                      currentLink==i?2:1,currentLink==i?2:1,
                                      CLR_BLACK,CLR_BLACK,
                                      0
                                     );
                }

                //draw selection rectangle
                if(creationInProgress) {
                        FRect r(MIN(createStart.GetX(),createEnd.GetX()),
                                MIN(createStart.GetY(),createEnd.GetY()),
                                MAX(createStart.GetX(),createEnd.GetX()),
                                MAX(createStart.GetY(),createEnd.GetY()));
                        WinDrawBorder(hps,&r,1,1,CLR_BLACK,CLR_BLACK,0);
                }

                WinEndPaint(hps);
        }
        return True;
}


Bool ALEClientWindow::MouseSingleSelect(FMouseEvent &ev) {
        int i=linkFromPoint(ev.GetPoint());
        if(i>=0) {
                setCurrentLink(i);
        }
        return True;
}

Bool ALEClientWindow::MouseOpen(FMouseEvent&) {
        editCurrentLink();
        return True;
}

Bool ALEClientWindow::MouseSelectStart(FMouseEvent &ev) {
        FPoint p=ev.GetPoint();
        if(linkFromPoint(p)==-1) {
                creationInProgress=True;
                createStart=createEnd=p;
        }
        return False;
}

Bool ALEClientWindow::MouseSelectEnd(FMouseEvent&) {
        if(creationInProgress) {
                creationInProgress=False;
                int lx=MIN(createStart.GetX(),createEnd.GetX());
                int rx=MAX(createStart.GetX(),createEnd.GetX());
                int by=MIN(createStart.GetY(),createEnd.GetY());
                int ty=MAX(createStart.GetY(),createEnd.GetY());
                Link l;
                memset(&l,0,sizeof(l));
                l.x=lx;
                l.cx=rx-lx;
                l.y=by;
                l.cy=ty-by;
                int i=la.add(l);
                setCurrentLink(i);
                if(!editCurrentLink()) {
                        setCurrentLink(-1);
                        la.remove(i);
                }
        }
        return True;
}

Bool ALEClientWindow::MouseMove(FMouseMoveEvent &ev) {
        if(creationInProgress) {
                FRect r;
                r.SetLeft(MIN(createStart.GetX(),createEnd.GetX()));
                r.SetRight(MAX(createStart.GetX(),createEnd.GetX()));
                r.SetBottom(MIN(createStart.GetY(),createEnd.GetY()));
                r.SetTop(MAX(createStart.GetY(),createEnd.GetY()));
                Invalidate(&r);
                createEnd=ev.GetPoint();
                r.SetLeft(MIN(createStart.GetX(),createEnd.GetX()));
                r.SetRight(MAX(createStart.GetX(),createEnd.GetX()));
                r.SetBottom(MIN(createStart.GetY(),createEnd.GetY()));
                r.SetTop(MAX(createStart.GetY(),createEnd.GetY()));
                Invalidate(&r);
        }
        return False;
}




Bool ALEClientWindow::Close(FEvent&) {
        if(changed) {
                switch(MessageBox(this,"The links has been modified. Save?",linkfilename,mb_yesnocancel)) {
                        case mbid_yes:
                                if(SaveFile())
                                        return False;
                                else
                                        return True;    //error ocurred - eat the event
                        case mbid_no:
                                changed=False;
                                return False;
                        case mbid_cancel:
                        default: 
                                return True;
                }

        } else
                return False;
}

Bool ALEClientWindow::CloseUnconditionally(FEvent&) {
        if(changed) {
                if(WriteLinks(linkfilename,la)) 
                        changed=False;
        }
        return False;
}


Bool ALEClientWindow::editCurrentLink() {
        if(currentLink>=0) {
                Link save=la[currentLink];
                FEditLinkDialog dlg(la[currentLink],this);
                if(dlg.DoModal(this)!=DID_OK) {
                        invalidateLink(currentLink);
                        la[currentLink]=save;
                        invalidateLink(currentLink);
                        return False;
                } else {
                        changed=True;
                        return True;
                }
        } else
                return False;
}



Bool ALEClientWindow::LoadBitmap(const char *filename) {
        HBITMAP hbmNew=ReadBitmapFromFile(filename,hpsBitmap);

        if(hbmNew) {
                if(hbm) {
                        GpiSetBitmap(hpsBitmap,NULLHANDLE);
                        GpiDeleteBitmap(hbm);
                }
                hbm=hbmNew;
                BITMAPINFOHEADER2 bih2;
                bih2.cbFix=sizeof(bih2);
                GpiQueryBitmapInfoHeader(hbm,&bih2);
                bitmapWidth=bih2.cx;
                bitmapHeight=bih2.cy;

                //update filenames
                strcpy(bitmapfilename,filename);
                char drive[_MAX_DRIVE],dir[_MAX_DIR],name[_MAX_FNAME],ext[_MAX_EXT];
                _splitpath(filename,drive,dir,name,ext);
                _makepath(linkfilename,drive,dir,name,".dat");

                //load links
                la.clear();
                if(ReadLinks(linkfilename,la)!=0)
                        la.clear();

                //update frame title
                GetAppWindow()->SetSubtitle(filename);

                //make the window fit the bitmap
                ApplySizeRestrictions();

                //invalidate window
                Invalidate(0);

                return True;
        } else
                return False;
}

Bool ALEClientWindow::LoadFile() {
        if(changed) {
                switch(MessageBox(this,"The links has been modified. Save?",linkfilename,mb_yesnocancel)) {
                        case mbid_yes:
                                if(SaveFile())
                                        break;
                                else
                                        return False;
                        case mbid_no:
                                break;
                        case mbid_cancel:
                        default: 
                                return False;
                }
        }

        //use previous path if any
        char filename[256];
        if(bitmapfilename[0]) {
                char drive[_MAX_DRIVE],dir[_MAX_DIR],name[_MAX_FNAME],ext[_MAX_EXT];
                _splitpath(bitmapfilename,drive,dir,name,ext);
                _makepath(filename,drive,dir,"*",".bmp");
        } else {
                strcpy(filename,"*.bmp");
        }
        FStdOpenFileDialog dlg;
        Bool b=dlg.DoModal(this,filename);
        if(!b)
                return False;
        if(LoadBitmap(filename)) {
                changed=False;
                return True;
        } else
                return False;
}

Bool ALEClientWindow::SaveFile() {
        if(WriteLinks(linkfilename,la)==0) {
                changed=False;
                return True;
        } else {
                MessageBox(this,"Could not save links",linkfilename,mb_ok);
                return False;
        }
}

