#include "linkio.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static void GetTag(FILE *fp, char *buf) {
        int c;
        for(;;) {
                c=getc(fp);
                if(c==EOF) 
                        return;
                if(c==':')
                        break;
        }

        for(;;) {
                c=getc(fp);
                switch(c) {
                        case '.':
                                *buf='\0';
                                return;
                        case EOF:
                                return;
                        default:
                                *buf++=(char)c;
                }
        }
}

static char *skipWhite(char *p) {
        while(*p && (*p==' ' || *p=='\t' || *p=='\n'))
                p++;
        return p;
}

static char *copystringval(char *p, char *buf) {
        if(*p=='\'') {
                //qouted string
                p++;
                while(*p && *p!='\'')
                        *buf++=*p++;
                if(*p=='\'') p++;
        } else {
                while(*p && *p!=' ' && *p!='\t' && *p!='\n') 
                        *buf++=*p++;
        }
        *buf='\0';
        return p;
}

static char *copynumval(char *p, int &b) {
        char tmp[80];
        int i=0;
        while(*p>='0' && *p<='9')
                tmp[i++]=*p++;
        tmp[i]='\0';
        b=atoi(tmp);
        return p;
}

static int ParseLinkTag(char *tag, LinkArray &la) {
        Link l;
        memset(&l,0,sizeof(l));

        tag+=4; //skip "link"
        tag=skipWhite(tag);
        while(*tag) {
                if(strnicmp(tag,"reftype",7)==0) {
                        tag=skipWhite(tag+7);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        if(strnicmp(tag,"hd",2)==0) {
                                l.reftype=Link::heading;
                                tag+=2;
                        } else if(strnicmp(tag,"fn",2)==0) {
                                l.reftype=Link::footnote;
                                tag+=2;
                        } else if(strnicmp(tag,"launch",6)==0) {
                                l.reftype=Link::launch;
                                tag+=6;
                        } else if(strnicmp(tag,"inform",5)==0) {
                                l.reftype=Link::inform;
                                tag+=5;
                        } else {
                                //unknown value
                                return -1;
                        }
                } else if(strnicmp(tag,"res",3)==0) {
                        tag=skipWhite(tag+3);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        tag=copystringval(tag,l.res);
                } else if(strnicmp(tag,"refid",5)==0) {
                        tag=skipWhite(tag+5);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        tag=copystringval(tag,l.refid);
                } else if(strnicmp(tag,"database",8)==0) {
                        tag=skipWhite(tag+8);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        tag=copystringval(tag,l.database);
                } else if(strnicmp(tag,"object",6)==0) {
                        tag=skipWhite(tag+6);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        tag=copystringval(tag,l.object);
                } else if(strnicmp(tag,"data",4)==0) {
                        tag=skipWhite(tag+4);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        tag=copystringval(tag,l.data);
                } else if(strnicmp(tag,"x",1)==0) {
                        tag=skipWhite(tag+1);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        tag=copynumval(tag,l.x);
                } else if(strnicmp(tag,"y",1)==0) {
                        tag=skipWhite(tag+1);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        tag=copynumval(tag,l.y);
                } else if(strnicmp(tag,"cx",2)==0) {
                        tag=skipWhite(tag+2);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        tag=copynumval(tag,l.cx);
                } else if(strnicmp(tag,"cy",2)==0) {
                        tag=skipWhite(tag+2);
                        if(*tag!='=') return -1;
                        tag=skipWhite(tag+1);
                        tag=copynumval(tag,l.cy);
                } else {
                        //unknown attribute
                        return -1;
                }
                tag=skipWhite(tag);
        }
        la.add(l);
        return 0;
}


int ReadLinks(const char *filename, LinkArray &la) {
        FILE *fp=fopen(filename,"rt");
        if(fp) {
                la.clear();
                int r=0;
                char tag[256];
                GetTag(fp,tag);
                if(feof(fp)) {
                        r=-1;
                } else if(strnicmp(tag,"artlink",7)!=0) {
                        //first tag must be :artlink.
                        r=-1;
                } else {
                        for(;;) {
                                GetTag(fp,tag);
                                if(feof(fp)) {
                                        //eof of file without eartlink???
                                        //we accept that
                                } else if(strnicmp(tag,"link",4)==0) {
                                        ParseLinkTag(tag,la);
                                } else if(strnicmp(tag,"eartlink",8)==0) {
                                        //end of links
                                        break;
                                } else {
                                        //unknown tag
                                        r=-1;
                                        break;
                                }
                        }
                }

                fclose(fp);
                return r;
        } else
                return -1;
}




static int WriteLink(FILE *fp, const Link &l) {
        fprintf(fp,":link reftype=");
        switch(l.reftype) {
                case Link::heading:
                        fprintf(fp,"hd");
                        break;
                case Link::footnote:
                        fprintf(fp,"fn");
                        break;
                case Link::launch:
                        fprintf(fp,"launch");
                        break;
                case Link::inform:
                        fprintf(fp,"infrom");
                        break;
        }

        if(l.res[0])
                fprintf(fp," res=%s",l.res);
        if(l.refid[0])
                fprintf(fp," refid=%s",l.refid);
        if(l.database[0])
                fprintf(fp," database='%s'",l.database);
        if(l.object[0])
                fprintf(fp," object='%s'",l.object);
        if(l.data[0])
                fprintf(fp," data='%s'",l.data);

        fprintf(fp," x=%d y=%d cx=%d cy=%d",l.x,l.y,l.cx,l.cy);

        fprintf(fp,".\n");
        return 0;
}


int WriteLinks(const char *filename, const LinkArray &la) {
        FILE *fp=fopen(filename,"wt");
        if(fp) {
                fprintf(fp,":artlink.\n");
                for(int i=0; i<la.count(); i++)
                        WriteLink(fp,la[i]);
                fprintf(fp,":eartlink.\n");

                fclose(fp);
                return 0;
        } else
                return -1;
}

