#include "savemenu.hpp"
#include <stdio.h>
#include <stdlib.h>


static void Indent(FILE *fp, int level) {
        const spacesPerLevel=2;
        for(int s=0; s<level*spacesPerLevel; s++)
                putc(' ',fp);
}

static void dumpStyle(FILE *fp, const MenuItem &mi) {
        switch(mi.ItemType) {
                case MenuItem::it_text:
                        //fprintf(fp,"MIS_TEXT"); break;
                        break;
                case MenuItem::it_bitmap:
                        fprintf(fp,"MIS_BITMAP"); break;
                case MenuItem::it_separator:
                        fprintf(fp,"MIS_SEPARATOR"); break;
                case MenuItem::it_ownerdraw:
                        fprintf(fp,"MIS_OWNERDRAW"); break;
        }
        if(mi.Static) fprintf(fp,"|MIS_STATIC");
        if(mi.Help) fprintf(fp,"|MIS_HELP");
        if(mi.SysCommand) fprintf(fp,"|MIS_SYSCOMMAND");
        switch(mi.ItemBreak) {
                case MenuItem::ib_nobreak:
                        break; //this is default
                case MenuItem::ib_break:
                        fprintf(fp,"|MIS_BREAK"); break;
                case MenuItem::ib_breakseparator:
                        fprintf(fp,"|MIS_BREAKSEPARATOR"); break;
                case MenuItem::ib_buttonseparator:
                        fprintf(fp,"|MIS_BUTTONSEPARATOR"); break;
        }

}

static void dumpAttributes(FILE *fp, const MenuItem &mi) {
        int first=1;
        if(mi.Checked) {
                if(!first) putc('|',fp);
                fprintf(fp,"MIA_CHECKED");
                first=0;
        }
        if(mi.Disabled) {
                if(!first) putc('|',fp);
                fprintf(fp,"MIA_DISABLED");
                first=0;
        }
        if(mi.Framed) {
                if(!first) putc('|',fp);
                fprintf(fp,"MIA_FRAMED");
                first=0;
        }
        if(mi.NoDismiss) {
                if(!first) putc('|',fp);
                fprintf(fp,"MIA_NODISMISS");
                first=0;
        }
}


static void dumpItemText(FILE *fp, const char *s) {
        while(*s) {
                switch(*s) {
                        case '\t':
                                fprintf(fp,"\\t");
                                break;
                        case '\a':
                                fprintf(fp,"\\a");
                                break;
                        default: 
                                fprintf(fp,"%c",*s);
                }
                s++;
        }
}


static int SaveMenu_RC(const char *filename, const MenuItemList &mil) {
        FILE *fp=fopen(filename,"wt");
        if(!fp) return -1;

        int level=0;
        for(int i=0; i<mil.count(); i++) {
                const MenuItem &mi=mil.Get(i);
                switch(mi.EditType) {
                        case MenuItem::et_submenu:
                                Indent(fp,level);
                                if(i==0) {
                                        //first level
                                        fprintf(fp,"MENU %s\n",mi.ItemID);
                                } else {
                                        fprintf(fp,"SUBMENU \"");
                                        dumpItemText(fp,mi.ItemText);
                                        fprintf(fp,"\", %s, ",mi.ItemID);
                                        dumpStyle(fp,mi);
                                        fprintf(fp,"\n");
                                }

                                Indent(fp,level);
                                fprintf(fp,"BEGIN\n");
                                level++;
                                break;

                        case MenuItem::et_endsubmenu:
                                level--;
                                Indent(fp,level);
                                fprintf(fp,"END\n");
                                break;

                        case MenuItem::et_item:
                                Indent(fp,level);
                                fprintf(fp,"MENUITEM ");
                                if(mi.ItemType==MenuItem::it_separator) {
                                        fprintf(fp,"SEPARATOR");
                                } else {
                                        fprintf(fp,"\"");
                                        dumpItemText(fp,mi.ItemText);
                                        fprintf(fp,"\", %s, ",mi.ItemID);
                                        dumpStyle(fp,mi);
                                        fprintf(fp,", ");
                                        dumpAttributes(fp,mi);
                                }
                                fprintf(fp,"\n");
                }
        }

        fclose(fp);
        return 0;
}

static SaveMenu_MNU(const char *filename, const MenuItemList &mil) {
        FILE *fp=fopen(filename,"wb");
        if(!fp) return -1;

        int items=mil.count();
        if(fwrite(&items,sizeof(items),1,fp)!=1) {
                fclose(fp); remove(filename);
                return -1;
        }
        for(int i=0; i<items; i++) {
                const MenuItem &mi=mil.Get(i);
                if(fwrite(&mi,sizeof(mi),1,fp)!=1) {
                        fclose(fp); remove(filename);
                        return -1;
                }
        }
        fclose(fp);
        return 0;
}

int SaveMenu(const char *filename, const MenuItemList &mil) {
        int r;
        //note: _splitpath and _makepath is Watcom-specific.
        char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
        _splitpath(filename,drive,dir,fname,ext);

        char name[_MAX_PATH];

        //save the menu into our our format
        _makepath(name,drive,dir,fname,".bme"); //"bme" = Bif Menu Editor
        r=SaveMenu_MNU(name,mil);
        if(r!=0) return r;

        //save the menu as .MNU (resource-format)
        _makepath(name,drive,dir,fname,".mnu");
        r=SaveMenu_RC(name,mil);

        return r;
}

