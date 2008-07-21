#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mbihc.h"

int autoIndex=0;

int main(int argc, char **argv) {
        printf("MBIHC - Build help from multiple .BIH files (compiled " __DATE__ " " __TIME__ ")\n");
        if(argc<2 || argc>3) {
                printf("Usage: mbihc <mbihc-file> [/ai|/nai]\n");
                return 1;
        }

        if(argc>=3) {
                if(stricmp(argv[2],"/nai")==0 || stricmp(argv[2],"/noautoindex")==0)
                        autoIndex=0;
                else if(stricmp(argv[2],"/ai")==0 || stricmp(argv[2],"/autoindex")==0)
                        autoIndex=1;
                else {
                        fprintf(stderr,"mbihc: Unknown option: '%s'\n",argv[2]);
                        return 3;
                }
        }

        FILE *listfile = fopen(argv[1],"r");
        if(!listfile) {
                fprintf(stderr,"Could not open '%s'\n",argv[1]);
                return 2;
        }
        
        startMultiHelp(argv[1]);
        
        char line[256];
        int lineno=0;
        while(fgets(line,256,listfile)) {
                lineno++;
                if(line[strlen(line)-1]=='\n')
                        line[strlen(line)-1]='\0';
                if(line[0] && line[0]!=';') {
                        char *l=strtok(line," \t"),
                             *infile=strtok(0," \t");
                        int baselevel = atoi(l);
                        if(infile==0 || baselevel<1) {
                                fprintf(stderr,"mbihc: error in %s in line %d\n",argv[1],lineno);
                                return 3;
                        }

                        buildMultiHelp(baselevel,infile);
                        
                }
        }
        
        endMultiHelp();
        
        fclose(listfile);
        
        return 0;
}

