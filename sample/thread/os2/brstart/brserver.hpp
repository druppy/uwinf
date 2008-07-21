#ifndef __BRSERVER_HPP
#define __BRSERVER_HPP

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

/* Mutex to protect the low-level file handles. This is neccesary because the 
 * input/output is changed briefly during job creation for redirection 
 * purposes and other threads need to display things (job list, errors, etc.)
 * This mutex also prevent output from multiple threads being intermixed
 */
extern FMutexSemaphore stdio_mutex;

/* Flag to let the threads terminate voluntarily
 * When this flag is non-zero threads should terminate a as soon as possible
 * (but they should free any resources they have allocated)
 */
extern volatile int terminate_server;   

extern char resultCodeQueueName[];

class ThreadDeleteThread;
extern ThreadDeleteThread *threadDeleter;

#endif

