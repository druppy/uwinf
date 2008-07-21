#ifndef __PROTOCOL_HPP
#define __PROTOCOL_HPP

#include "brlimits.hpp"
#include <string.h>

#define DEFAULT_SERVER_PIPE_NAME "brserver"

/*
Client              Server
Close               ack                      client closes pipe,
                                             server disconnects

Query jobs          Job list

Set command         ack/nack                 data:command

Set parameters      ack/nack                 data:parameters

Set cwd             ack/nack                 data:program current directory

Set cd              ack/nack                 data:program current disk

Set output redir.   pipe name+postack/nack

Set input redir.    pipe name+postack/nack

Set nice name       ack                      Server remembers job name (command is default)

Start               ack/nack                 Server starts program

Wait                result code/nack         Client blocks until the job has
                                             finshed

Close server        ack/nack                 Server closes when all other
                                             jobs has finished
*/


/* Client requests */
struct CR_common {
        unsigned long requestType; //crt_xxxxx
        CR_common(unsigned long crt) : requestType(crt) {}
};

#define crt_close            0x0001
#define crt_queryjoblist     0x0002
#define crt_setcommand       0x0003
#define crt_setparameters    0x0004
#define crt_setcwd           0x0005
#define crt_setcd            0x0006
#define crt_setoutputredir   0x0007
#define crt_setinputredir    0x0008
#define crt_setnicename      0x0009
#define crt_start            0x000A
#define crt_wait             0x000B
#define crt_closeserver      0x000C

struct CR_Close : public CR_common {
        //no extra fields
        CR_Close() : CR_common(crt_close) {}
};

struct CR_QueryJobList : public CR_common {
        //no extra fields
        CR_QueryJobList() : CR_common(crt_queryjoblist) {}
};

struct CR_SetCommand : public CR_common {
        char command[128];             //asciz
        CR_SetCommand(const char *c)
          : CR_common(crt_setcommand)
          { strcpy(command,c); }
};

struct CR_SetParameters : public CR_common {
        char parameters[MAXPARAMETERLENGTH];          //NUL-terminated, last is double-NUL'ed
        CR_SetParameters(const char *p)
          : CR_common(crt_setparameters)
          { memcpy(parameters,p,MAXPARAMETERLENGTH); }
};

struct CR_SetCwd : public CR_common {
        char cwd[MAXCWDLENGTH];                  //only for 1 disk
        CR_SetCwd(const char *cd)
          : CR_common(crt_setcwd)
          { strcpy(cwd,cd); }
};

struct CR_SetCd : public CR_common {
        char drive[4];                  //"A:", "C:", ...
        CR_SetCd(const char *d)
          : CR_common(crt_setcd)
          { strcpy(drive,d); }
};

struct CR_SetOutputRedir : public CR_common {
        //no extra fields
        CR_SetOutputRedir()
          : CR_common(crt_setoutputredir)
          {}
};

struct CR_SetInputRedir : public CR_common {
        //no extra fields
        CR_SetInputRedir()
          : CR_common(crt_setinputredir)
          {}
};

struct CR_SetNiceName : public CR_common {
        char niceName[MAXNICENAMELENGTH];
        CR_SetNiceName(const char *nn)
          : CR_common(crt_setnicename)
          { strcpy(niceName,nn); }
};

struct CR_Start : public CR_common {
        unsigned long waitForTermination;
        CR_Start(unsigned long wft)
          : CR_common(crt_start), waitForTermination(wft)
          {}
};

struct CR_Wait : public CR_common {
        //no extra fields
        CR_Wait()
          : CR_common(crt_wait)
          {}
};

struct CR_CloseServer : public CR_common {
        //no extra fields
        CR_CloseServer()
          : CR_common(crt_closeserver)
          {}
};


/* Server answers */
struct SA_common {
        unsigned long answerType;        //sat_xxx
        SA_common(unsigned long sat) : answerType(sat) {}
};
#define sat_ack        0x0001
#define sat_nack       0x0002
#define sat_joblist    0x0003
#define sat_pipename   0x0004
#define sat_resultcode 0x0005
#define sat_postack    0x0006

struct SA_Ack : public SA_common {
        SA_Ack()
          : SA_common(sat_ack)
          {}
};

struct SA_Nack : public SA_common {
        char reason[MAXERRORMSGLENGTH];
        SA_Nack(const char *why)
          : SA_common(sat_nack)
          { strcpy(reason,why); }
};

struct SA_JobList : public SA_common {
        unsigned long jobs;
        struct job {
                char niceName[MAXNICENAMELENGTH];
                char command[MAXCOMMANDLENGTH];
                unsigned long status;  //0=running, 1=finished
        } j[ (4096-sizeof(SA_common)-sizeof(unsigned long)) / sizeof(job) ];
        SA_JobList()
          : SA_common(sat_joblist), jobs(0)
          {}
        void AddJob(const char *nn, const char *c, unsigned long s) {
                strcpy(j[jobs].niceName,nn);
                strcpy(j[jobs].command,c);
                j[jobs].status=s;
                jobs++;
        }
};

struct SA_PipeName : public SA_common {
        char pipename[MAXPIPENAMELENGTH];
        SA_PipeName(const char *p)
          : SA_common(sat_pipename)
          { strcpy(pipename,p); }
};

struct SA_ResultCode : public SA_common {
        unsigned long code;
        SA_ResultCode(unsigned long c)
          : SA_common(sat_resultcode), code(c)
          {}
};

struct SA_PostAck : public SA_common {
        SA_PostAck() : SA_common(sat_postack) {}
};
#endif
