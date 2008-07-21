/* file: default.h
 * author: ISJ
 * date: 1992-07-09
 *       1994-01-10 ISJ  added C++ support
 *       1994-01-10 ISJ  changed parameters and concept
 *       1994-01-17 ISJ  changed concept (again!)
 *
 */

#ifndef DEFAULT_H
#define DEFAULT_H

#ifdef __cplusplus
extern "C" {
#endif

int scanDefaultsFile(const char *progname,
                     int *dstargc, char *dstargv[], int limit);

/* return value:
 *  -1   major trouble (eg: illegal parameters)
 *   0   minor trouble (eg: malloc() returning NULL)
 *   1   Ok
 */

#ifdef __cplusplus
};
#endif

#endif
