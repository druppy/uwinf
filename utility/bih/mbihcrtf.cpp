#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "mbihc.h"

static char HPJFilename[128];
static char TOCListFilename[128];
static char TOCFilename[128];

void startMultiHelp(const char *projectfilename) {
        //calculate .HPJ filename
        char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
        _splitpath(projectfilename,drive,dir,fname,ext);
        _makepath(HPJFilename,drive,dir,fname,".hpj");
        _makepath(TOCListFilename,drive,dir,fname,".tcl");
        _makepath(TOCFilename,drive,dir,"toc",".rtf");

        FILE *fp=fopen(HPJFilename,"w");
        if(!fp) {
                fprintf(stderr,"mbihc: Could not create '%s'\n",HPJFilename);
                perror("mbihc");
                exit(10);
        }

        fprintf(fp, "[OPTIONS]\n"
                    "compress=no\n"
                    "title= No name yet\n"
                    "warning=3\n"
                    ";contents=\n"
                    "\n"
                    "[CONFIG]\n"
                    "BrowseButtons()\n"
                    "\n"
                    "[FILES]\n"
                    "toc.rtf\n"
               );

        fclose(fp);

        remove(TOCListFilename);
}

static int isFileUptodate(const char *inname, const char *outname) {
        struct stat statBIH,statRTF;
        if(stat(inname,&statBIH)!=0) return 0;
        if(stat(outname,&statRTF)!=0) return 0;
        if(statRTF.st_mtime<statBIH.st_mtime) return 0;
        return 1;
}

static char RTFFilenameBuffer[2048]="";
static char TOCFilenameBuffer[2048]="";
static char biFilenameBuffer[2048]="";

void buildMultiHelp(int baselevel, const char *inname) {
        char outname[128];
        
        char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
        _splitpath(inname,drive,dir,fname,ext);
        _makepath(outname,drive,dir,fname,".rtf");

        //see if we need to compile the file
        if(!isFileUptodate(inname,outname)) {
                printf("Compiling %s -> %s\n",inname,outname);
                //Build arguments for BIHC
                char arg0[20];
                char arg1[128];
                char arg2[128];
                char arg3[20];
                char arg4[6];
                char arg5[20];
        
                strcpy(arg0,"bihc");
        
                strcpy(arg1,inname);
        
                strcpy(arg2,outname);
                
                strcpy(arg3,autoIndex?"/ai":"/nai");
        
                itoa(baselevel,arg4,10);
        
                strcpy(arg5,"/multi");
                
                int rc;
                rc = spawnlp(P_WAIT, "bihc", arg0,arg1,arg2,arg3,arg4,arg5,(char*)0);
                if(rc!=0) {
                        fprintf(stderr,"mbihc: Error executing bihc\n");
                        if(rc==-1) perror("mbihc");
                        exit(11);
                }
        } else
                printf("%s is up-to-date\n",outname);
        
                
        {
                //concatenate RTF filename to list of RTF files
                strcat(RTFFilenameBuffer,outname);
                strcat(RTFFilenameBuffer,"\n");
        }

        {
                //concatenate TOC-file to list of TOC files
                char TOCName[_MAX_PATH];
                _makepath(TOCName,drive,dir,fname,".toc");
                strcat(TOCFilenameBuffer,TOCName);
                strcat(TOCFilenameBuffer,"\n");
        }

        {
                //concatenate bi-file to list of bi files
                char biName[_MAX_PATH];
                _makepath(biName,drive,dir,fname,".bi");
                strcat(biFilenameBuffer,biName);
                strcat(biFilenameBuffer,"\n");
        }
}
        

void endMultiHelp() {
        //dump TOCFilenameBuffer to a file
        FILE *fp=fopen(TOCListFilename,"w");
        if(!fp) {
                fprintf(stderr,"mbihc: Could not create '%s'\n",TOCListFilename);
                exit(12);
        }
        fputs(TOCFilenameBuffer,fp);
        fclose(fp);
        
        //run mktoc
        char *argv0;
        char *argv1;
        char argv2[128];
        argv0 = "mktoc";
        argv1 = TOCFilename;
        strcpy(argv2,"@"); strcat(argv2,TOCListFilename);
        
        int rc = spawnlp(P_WAIT, "mktoc", argv0,argv1,argv2,(char*)0);
        if(rc!=0) {
                fprintf(stderr,"mbihc: Error executing mktoc\n");
                if(rc==-1) perror("mbihc");
                exit(11);
        }
        remove(TOCListFilename);

        {
                //add rtf files to HPJ file
                FILE *fp=fopen(HPJFilename,"a");
                if(!fp) {
                        fprintf(stderr, "mbihc: Could not re-open '%s'\n",HPJFilename);
                        perror("mbihc");
                        exit(10);
                }
                fputs(RTFFilenameBuffer,fp);
                //add bi files to HPF file
                fprintf(fp,"[bitmaps]\n");
                for(char *p=strtok(biFilenameBuffer,"\n"); p; p=strtok(0,"\n"))
                        if(*p)
                                fprintf(fp,"#include <%s>\n",p);
                fclose(fp);
        }
}


