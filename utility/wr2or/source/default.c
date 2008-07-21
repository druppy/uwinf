/* file: default.c
 * author: ISJ
 * date: 1992-07-09
 *       1994-01-10 ISJ  added OS/2 support
 *       1994-01-10 ISJ  changed parameters and concept
 *       1994-01-11 ISJ  added support for 'stringed' args.
 *       1994-01-17 ISJ  changed concept (again!)
 *
 * defaults file scanning
 *
 * Released as Public Domain
 */


#include <stdio.h>
#include <errno.h>
//#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "default.h"
#include "moreargs.h"

#define DEFAULT_EXT    ".dfl"
#define MAX_LINE_LEN   512

#define iswhite(c) (isascii(c) && isspace(c))


static void stripQuotes(char *start, char *end) {
	char *s,*d;
	s=d=start;
	while(s<end) {
		if(*s=='"')
			s++;
		else
			*d++=*s++;
	}
	*d='\0';
}

static int ScanALine(char *l, int *dstargc, char *dstargv[], int limit, int *pos)
{
	char *p;
	for(p=l; *p; ) {
		while(*p && iswhite(*p)) p++;	//skip initial spaces
		if(*p) {
			char *argstart=p;
			char *argend;
			int inString=0;
			while(*p) {
				if((!inString) && iswhite(*p))
					break;
				if(*p=='"')
					inString=!inString;
				p++;

			}

			argend=p;
			if(*p) p++;
			stripQuotes(argstart,argend);
			if(insertArgument(dstargv,limit,(*pos)++,dstargc,argstart))
				return 1;
		}
	}

	return 0;
}


static int ScanFile(FILE *f, int *dstargc, char *dstargv[], int limit)
{
	char l[MAX_LINE_LEN];
	int r=0;
	int pos=0;

	while(r==0 && fgets(l,MAX_LINE_LEN-1,f)!=NULL) {
		if(*l=='\0')
			break;
		r=ScanALine(l,dstargc,dstargv,limit,&pos);
	}
	return r?0:1;
}




int scanDefaultsFile(const char *progname,
		     int *dstargc, char *dstargv[], int limit)
{
#if defined(__MSDOS__) || defined(OS2) || defined(__OS2__)
	char path1[_MAX_PATH];
	char path2[_MAX_PATH];
#else
#ifdef UNIX
	char path1[512];
	char path2[512];
#else
#error Operating system not defined
#endif
#endif
	FILE *f;
	int r=1;

	/* parameter checks */
	if(progname==NULL) return -1;
	if(dstargc==NULL)  return -1;
	if(dstargv==NULL)  return -1;
	if(limit<0)        return -1;

#if defined(__MSDOS__) || defined(OS2) || defined(__OS2__)
	{
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char name[_MAX_FNAME];
		char ext[_MAX_EXT];

		_splitpath(progname,drive,dir,name,ext);
		_makepath(path1,"","",name,DEFAULT_EXT);
		_makepath(path2,drive,dir,name,DEFAULT_EXT);
	}
#else
#ifdef UNIX
	{
		char *home;

		/*in unix we first search current directory the in HOME*/
		strcpy(path1,progname);
		strcat(path1,DEFAULT_EXT);
		if((home=getenv("HOME="))!=NULL) {
			strcpy(path2,home);
			strcat(path2,"/");
			strcat(path2,progname);
			strcat(path2,DEFAULT_EXT);
		} else
			path2[0]='\0';
	}
#else
#error Operating system not defined
#endif
#endif
	if((f=fopen(path1,"rt"))!=NULL) {
		/*defaults file found in current directory*/
		r=ScanFile(f,dstargc,dstargv,limit);
		fclose(f);
	} else {
		if(path2[0]) {
			if((f=fopen(path2,"rt"))!=NULL) {
				r=ScanFile(f,dstargc,dstargv,limit);
				fclose(f);
			}
		} else
			r=1;	/* not finding the defaults file is OK */
	}

	errno=0;/*reset, so other dont get "no such file or directory"*/

	if(*dstargc<limit) dstargv[*dstargc]=NULL;


	return r;
}
