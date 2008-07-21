#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void processFile(FILE *ofp, const char *filename) {
        FILE *ifp=fopen(filename,"r");
        if(!ifp) {
                fprintf(stderr, "mktoc: Could not open '%s'\n",filename);
                perror("mktoc");
                exit(2);
        }
        char line[128];
        while(fgets(line,128,ifp)) {
                char *p=line;
                int level=atoi(line);
                while(*p!=' ') p++;
                p++;
                char *id=p;
                while(*p!=' ') p++;
                *p='\0';
                p++;
                char *text=p;
                while(*p && *p!='\n') p++;
                *p='\0';
                switch(level) {
                        case -1: break;
                        case 1: fprintf(ofp, "\\par\\sect\n"); break;
                        case 2: fprintf(ofp, "\\par\n"); break;
                        case 3: fprintf(ofp, "\\par\n"); break;
                        default: fprintf(ofp,"\\line\n");
                }
                for(int l=0; l<level-1; l++)
                        fprintf(ofp, "\\tab");
                if(level==1) fprintf(ofp, "{\\fs24");
                if(level==2) fprintf(ofp, "{\\b");
                fprintf(ofp, "{\\uldb %s}{\\v %s}",text,id);
                if(level==1 || level==2) fprintf(ofp, "}");
        }
        fclose(ifp);
        
        if(ferror(ofp)) {
                fprintf(stderr, "mktoc: error on output to '%s'\n",filename);
                perror("mktoc");
                exit(3);
        }
}

int main(int argc, char **argv) {
        printf("MKTOC (compiled: %s %s)\n",__DATE__,__TIME__);
        if(argc<3) {
                printf("Usage: mktoc toc.rtf [file1.toc file2.toc ... | @listfile]\n");
                return -1;
        }

        FILE *ofp=fopen(argv[1],"w");
        if(!ofp) {
                fprintf(stderr, "mktoc: Could not create 'toc.rtf'\n");
                perror("mktoc");
                return 1;
        }

        fprintf(ofp, "{\\rtf1\\ansi\n");
        fprintf(ofp, "{\\fonttbl\\f0\\fswiss Arial;}\n");
        fprintf(ofp, "\\deff0\n");
        fprintf(ofp, "\\fs20\n");
        fprintf(ofp, "#{\\footnote toc}\n");
        fprintf(ofp, "\\tx500\\tx750\\tx1000\\tx1250\\tx1500\n");

        if(argv[2][0]=='@') {
                //use listfile
                FILE *fp=fopen(argv[2]+1,"r");
                if(!fp) {
                        fprintf(stderr,"mktoc: Could not open '%s'\n",argv[2]);
                        perror("mktoc");
                        exit(4);
                }
                char line[128];
                while(fgets(line,128,fp)) {
                        if(line[0] && line[strlen(line)-1]=='\n')
                                line[strlen(line)-1]='\0';
                        processFile(ofp,line);
                }
                fclose(fp);
        } else {
                for(int a=2; a<argc; a++) {
                        processFile(ofp, argv[a]);
                }
        }
        
        fprintf(ofp, "\\page");
        fprintf(ofp,"}");
        fclose(ofp);
        
        return 0;
}
