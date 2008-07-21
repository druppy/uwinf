#ifndef __JOB_HPP
#define __JOB_HPP

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#include "brlimits.hpp"

class ServerIPipe;
class ServerOPipe;

struct Job {
        char command[MAXCOMMANDLENGTH];
        char parameters[MAXPARAMETERLENGTH];
        char currentDrive[3];
        char currentDir[26][MAXCWDLENGTH];
        char outputPipeName[MAXPIPENAMELENGTH];
        char inputPipeName[MAXPIPENAMELENGTH];
        char niceName[MAXNICENAMELENGTH];

        int started;
        ULONG idSession;
        PID pid;
        ServerIPipe *inputPipe;
        ServerOPipe *outputPipe;
        int waitForTermination;                 
        FEventSemaphore event_terminated;       //Posted when the job has finished
        int terminated;                         //0=running
        USHORT resultCode;                      //Result code from job

        Job();
        ~Job();
};

#endif

