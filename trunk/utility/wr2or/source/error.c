/* file: error.c
 * author: ISJ
 * date: 1992-06-29
 *       1992-07-13 added default messages
 *       1994-02-19 Changed one line to make it ANSI
 */

#ifndef lint
#pragma warn -use
static char     sccsid[] = "@(#)error.c      1.0 (ISJ) 1992-06-30";
#endif /*not lint*/

#include <stdio.h>
#include <errno.h>
#include <process.h>
#include <stdarg.h>

#include "error.h"

void ShowError(int ExitCode,const char *format,...) {
	va_list ap;

	va_start(ap,format);
	if(format==NULL) {
		/* If the programmer does not provide his own error message,
		 * then we show the default error message for the exitcode
		 */
		switch(ExitCode) {
			case EX_OK:
					break; /*not an error*/
			case EX_USAGE:
					fprintf(stderr,"Usage error\n");
					break;
			case EX_DATAERR:
					fprintf(stderr,"Error in input file\n");
					break;
			case EX_NOINPUT:
					fprintf(stderr,"Could not open input file\n");
					break;
			case EX_NOUSER:
					fprintf(stderr,"The specified user does not exist\n");
					break;
			case EX_NOHOST:
					fprintf(stderr,"The specified host does not exist\n");
					break;
			case EX_UNAVAILABLE:
					fprintf(stderr,"Service unavailable or error in support program\n");
					break;
			case EX_SOFTWARE:
					fprintf(stderr,"Internal error\n");
					break;
			case EX_OSERR:
					fprintf(stderr,"Operating system error\n");
					break;
			case EX_OSFILE:
					fprintf(stderr,"Operating system file contains errors or was not found\n");
					break;
			case EX_CANTCREAT:
					fprintf(stderr,"Cannot create output file\n");
					break;
			case EX_IOERR:
					fprintf(stderr,"Error occurred while doing i/o\n");
					break;
			case EX_TEMPFAIL:
					fprintf(stderr,"Temporary error. Try again (later)\n");
					break;
			case EX_PROTOCOL:
					fprintf(stderr,"Protocol error\n");
					break;
			case EX_NOPERM:
					fprintf(stderr,"You have not permission for that\n");
					break;
			case EX_MEM:
					fprintf(stderr,"Not enough core\n");
					break;
			default:
					fprintf(stderr,"Unknown error\n");
		}/*switch*/
	} else
		vfprintf(stderr,format,ap);

	if(ExitCode==EX_IOERR && errno!=0) {
		/* If it was an i/o-error it might help the user if we display
		 * the (system-) message for this error
		 */
		perror("System reports");
	}

	va_end(ap);

	exit(ExitCode);
}
