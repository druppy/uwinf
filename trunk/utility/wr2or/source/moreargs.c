#include "moreargs.h"
#include <stdlib.h>
#include <string.h>

#ifndef NULL
#define NULL 0
#endif

int insertArgument(char *argv[], int limit, int pos, int *argc, const char *args)
{
	char *p;
	int i;

	if(argv==NULL) return -1;
	if(limit<=0) return -1;
	if(argc==NULL) return -1;
	if(*argc>=limit) return -1;
	if(args==NULL) return -1;

	if(pos<0) pos=0;
	if(pos>*argc) pos=*argc;

	/* as far as I know strdup is non-ANSI ...*/
	p=malloc(strlen(args)+1);
	if(p==NULL) return -1;
	strcpy(p,args);

	/* move the last part of the arguments */
	for(i=*argc; i>pos; i--)
		argv[i]=argv[i-1];

	argv[pos]=p;
	++(*argc);

	return 0;
}
