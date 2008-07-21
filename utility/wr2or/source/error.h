/* file: errors.h
 * author: ISJ
 * date: 1992-06-29 created
 *       1994-01-09 Added C++ support
 *
 * define's for exit codes
 */

#ifndef ERROR_H
#define ERROR_H

#include <sysexits.h> /*extra header file, if you dont have it, get it from a
		       *UNIX system or alter the defines
		       */

#ifdef __cplusplus
extern "C" {
#endif

void ShowError(int ExitCode,const char *format,...);

#ifdef __cplusplus
};
#endif

#endif