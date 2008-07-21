#define BIFINCL_IPC
#define BIFINCL_CLIENTSERVER
#define BIFINCL_CLIENT
#define BIFINCL_TCPCLIENT
#include <bif.h>

#include <stdio.h>
#include <time.h>

#include <stdio.h>
#include <i86.h>
void main(void) {

#define CONS 1000
        clock_t starttime=clock();
        int connectionsMade=0;
        for(int i=0; i<CONS; i++) {
                FTCPClientConnection con("localhost:5500");
//printf("%c",con.Fail()?'-':'*');
//fflush(stdout);
                if(!con.Fail()) connectionsMade++;
                con.Close();
        }
        clock_t endtime=clock();
        printf("running time: %f seconds\n", ((double)(endtime-starttime))/CLK_TCK);
        printf(" = %d connections per second\n", (int)(CONS/(((double)(endtime-starttime))/CLK_TCK)));
        printf(" %d of %d succeeded\n",connectionsMade,CONS);
}

