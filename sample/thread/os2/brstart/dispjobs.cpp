#include "job.hpp"
#include "dispjobs.hpp"
#include "joblist.hpp"
#include "brserver.hpp"

#include <stdio.h>

void DisplayJobList(void) {
        stdio_mutex.Request();
          printf("\n");
          printf("Type  Name                             I O Status    Result\n");
          printf("      Command\n");
  
          jobList.StartTraversal();
          for(Job *job=jobList.GetNext(); job; job=jobList.GetNext()) {
                printf("%-5.5s ",job->waitForTermination?"Sync":"ASync");
                printf("%-32.32s ",job->niceName);
                printf("%s ",job->inputPipeName[0]?"I":" ");
                printf("%s ",job->outputPipeName[0]?"O":" ");
                printf("%-9.9s ",job->terminated?"Terminated":"Running");
                if(job->terminated)
                        printf("%6u",(int)job->resultCode);
                printf("\n");

                printf("      %s",job->command);
                const char *p=job->parameters;
                printf(" ");
                while(p[0] || p[1]) {
                        printf("%s ",p);
                        while(*p) p++;
                        if(p[1]) p++;
                }
                printf("\n");
          }
          jobList.EndTraversal();
          fflush(stdout);               //flush output before releasing stdio to other threads
        stdio_mutex.Release();
}

