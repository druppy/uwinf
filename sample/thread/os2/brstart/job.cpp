#include "job.hpp"

Job::Job() {
        command[0]='\0';
        parameters[0]='\0';
        currentDrive[0]='\0';
        for(int i=0; i<26; i++)
                currentDir[i][0]='\0';
        outputPipeName[0]='\0';
        inputPipeName[0]='\0';
        inputPipe=0;
        outputPipe=0;
        niceName[0]='\0';
        started=0;
        terminated=0;
}

Job::~Job() {
}
