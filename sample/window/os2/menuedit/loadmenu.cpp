#include "loadmenu.hpp"
#include <stdio.h>
#include <stdlib.h>


static int LoadMenu_BME(const char *filename, MenuItemList &mil) {
        FILE *fp=fopen(filename,"rb");
        if(!fp) return -1;

        int items;
        if(fread(&items,sizeof(items),1,fp)!=1) {
                fclose(fp);
                return -1;
        }

        for(int i=0; i<items; i++) {
                MenuItem mi;
                fread(&mi,sizeof(mi),1,fp);
                mil.insert(i,mi);
        }
        fclose(fp);
        return 0;
}

int LoadMenu(const char *filename, MenuItemList &mil) {
        //currently, we can only read .bme files
        int r;
        //note: _splitpath and _makepath is Watcom-specific.
        char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
        _splitpath(filename,drive,dir,fname,ext);

        char name[_MAX_PATH];

        //load the menu in our format
        _makepath(name,drive,dir,fname,".bme"); //"bme" = Bif Menu Editor
        r=LoadMenu_BME(name,mil);

        return r;
}
