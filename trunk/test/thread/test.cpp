/***
Filename: test.cpp
Description:
  This file contains test drivers for all parts of BIF/Thread
  This file can be used for testing BIF/Thread with a new compiler
Host:
  DOS16: BC31, WC10-10.6,
  DOS32: WC10-10.6,
  WIN16: BC31, BC40, WC10-10.6
  WIN32: WC10-10.6
History:
  ISJ 94-07-07 Creation
***/

#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>
#include <stdio.h>

#include "dbglog.h"

#if defined(_Windows) || defined(__WINDOWS__) || defined(_WINDOWS) || defined(__NT__)
#  ifndef __WINDOWS_H
#    include <windows.h>
#  endif
#else
#include <time.h>
#endif

// Declare a stupid test thread
class MyThread : public FThread {
        int n;
public:
        MyThread(int _n) : FThread(), n(_n) {}
        virtual void Go();
};


//declare our test main thread
class MyMainThread : public FMainThread {
public:
        MyMainThread(int argc, char **argv)
          : FMainThread(argc,argv)
          { TRACE_METHOD2("MyMainThread::MyMainThread");}
        ~MyMainThread()
          { TRACE_METHOD2("MyMainThread::~MyMainThread");}

        virtual int Main(int,char *argv[]);
};



//Uncomment one of these test drivers at a time

/*
//---------------------------------------------------------------------------
// test that at least main thread works
void MyThread::Go() {
}

#define THREADS 1
int MyMainThread::Main(int,char**) {
        TRACE_METHOD2("MyMainThread::Main");
        FThreadManager::GetCurrentThread();
        return 0;
}
*/


/*
//---------------------------------------------------------------------------
// test that FThread::Wait() works
void MyThread::Go() {
        TRACE_METHOD2("MyThread::Go");
        Yield();
}

#define THREADS 1
int MyMainThread::Main(int,char**) {
        TRACE_METHOD2("MyMainThread::Main");
        FThread *tp= new MyThread(0);
        tp->Start();
        tp->Wait();
        Yield();
        Yield();
        return 0;
}
*/


/*
//---------------------------------------------------------------------------
// test that basic thread switching works
volatile int cont=0;

void MyThread::Go() {
        TRACE_METHOD2("MyThread::Go");
        Yield();
        cont=1;
        Yield();
}

int MyMainThread::Main(int,char**) {
        TRACE_METHOD2("MyMainThread::Main");
        FThread *tp= new MyThread(0);
        tp->Start();

        while(!cont)
                Yield();
        return 0;
}
*/




/*
//---------------------------------------------------------------------------
//test that Sleep() works
FEventSemaphore cont;

void MyThread::Go() {
	TRACE_METHOD2("MyThread::Go");
	Sleep(10000);
	cont.Post();
}

int MyMainThread::Main(int,char**) {
	TRACE_METHOD2("MyMainThread::Main");
	FThread *tp= new MyThread(0);
	tp->Start();
	cont.Wait();
	return 0;
}
*/


/*
//---------------------------------------------------------------------------
// Measure the speed of thread switching with one thread only
//Useful only under dos16+dos32+win16
// Logging should be turned off
long switches=0;

int MyMainThread::Main(int,char**) {
	TRACE_METHOD2("MyMainThread::Main");
	#define SECONDS 10
#if (BIFOS_ == BIFOS_WIN16_) || (BIFOS_ == BIFOS_WIN32_)
	long start = GetTickCount();
	long stop = start+SECONDS*1000;
	while(GetTickCount()<stop) {
		switches++;
		Yield();
	}
	char buf[256];
	sprintf(buf,"Switches: %ld in %d seconds\n = %ld switches per second",switches,SECONDS,long(switches/SECONDS));
	MessageBox(NULL,buf,"trace",MB_OK);
#else
	long start = clock();
	long stop = start + SECONDS*CLK_TCK;
	while(clock()<stop) {
		switches++;
		Yield();
	}
	printf("Switches: %ld in %d seconds\n",switches,SECONDS);
	printf(" = %ld switches per second\n",long(switches/SECONDS));
#endif
	return 0;
}
*/


/*
//---------------------------------------------------------------------------
// measure the speed of thread switching with a bunch of threads
//Useful only under dos16+dos32+win16
// Logging should be turned off
#define THREADS 10
long switches=0;
long stop;

void MyThread::Go() {
        TRACE_METHOD2("MyThread::Go");
#if (BIFOS_ == BIFOS_WIN16_) || (BIFOS_ == BIFOS_WIN32_)
        while(GetTickCount()<stop) {
#else
        while(clock()<stop) {
#endif
                switches++;
                Yield();
        }
}

int MyMainThread::Main(int,char**) {
        TRACE_METHOD2("MyMainThread::Main");
        #define SECONDS 10
        FThread *tp[THREADS];
        for(int i=0; i<THREADS; i++) {
                tp[i] = new MyThread(i);
                tp[i]->Start();
        }
#if (BIFOS_ == BIFOS_WIN16_) || (BIFOS_ == BIFOS_WIN32_)
        long start = GetTickCount();
        stop = start+SECONDS*1000;
        while(GetTickCount()<stop) {
                switches++;
                Yield();
        }
        char buf[256];
        sprintf(buf,"Switches: %ld in %d seconds\n = %ld switches per second",switches,SECONDS,long(switches/SECONDS));
        MessageBox(NULL,buf,"trace",MB_OK);
#else
        long start = clock();
        stop = start + SECONDS*CLK_TCK;
        while(clock()<stop) {
                switches++;
                Yield();
        }
        printf("Switches: %ld in %d seconds\n",switches,SECONDS);
        printf(" = %ld switches per second\n",long(switches/SECONDS));
#endif
        return 0;
}
*/



//---------------------------------------------------------------------------
// measure the speed of thread creation
//Useful only under dos16+dos32+win16
// Logging should be turned off
#define THREADS 10
long creations=0;
long stop;

void MyThread::Go() {
}

int MyMainThread::Main(int,char**) {
        TRACE_METHOD2("MyMainThread::Main");
        #define SECONDS 10
#if (BIFOS_ == BIFOS_WIN16_) || (BIFOS_ == BIFOS_WIN32_)
        long start = GetTickCount();
        stop = start+SECONDS*1000;
        while(GetTickCount()<stop) {
                MyThread t(0);
                t.Start();
                t.Wait();
                creations++;
        }
        char buf[256];
        sprintf(buf,"Creations: %ld in %d seconds\n = %ld creations per second",creations,SECONDS,long(creations/SECONDS));
        MessageBox(NULL,buf,"trace",MB_OK);
#else
        long start = clock();
        stop = start + SECONDS*CLK_TCK;
        while(clock()<stop) {
                MyThread t(0);
                t.Start();
                t.Wait();
                creations++;
        }
        printf("Creations: %ld in %d seconds\n",creations,SECONDS);
        printf(" = %ld creations per second\n",long(creations/SECONDS));
#endif
        return 0;
}



/*
//---------------------------------------------------------------------------
//test that FEventSemaphore works
FEventSemaphore cont;

void MyThread::Go() {
        TRACE_METHOD2("MyThread::Go");
        if(n==0) Sleep(1000);
        Yield();
        cont.Post();
        Yield();
}

#define THREADS 1
int MyMainThread::Main(int,char**) {
        TRACE_METHOD2("MyMainThread::Main");
        FThread *tp= new MyThread(0);
        tp->Start();
        cont.Wait();
        return 0;
}
*/



/*
//---------------------------------------------------------------------------
//test that FMutexSemaphore works
FMutexSemaphore ms;

void MyThread::Go() {
        TRACE_METHOD2("MyThread::Go");
        ms.Request();
        ms.Release();
}

int MyMainThread::Main(int,char**) {
        TRACE_METHOD2("MyMainThread::Main");
        FThread *tp= new MyThread(0);
        ms.Request();
        tp->Start();
        Yield();
        Yield();
        Yield();
        ms.Release();
        Yield();
        Yield();
        Yield();

        return 0;
}
*/

/*
//---------------------------------------------------------------------------
//test that FMutexSemaphore works and is invalid if the owning thread dies
FMutexSemaphore ms;
FEventSemaphore es;

void MyThread::Go() {
        TRACE_METHOD2("MyThread::Go");
        ms.Request();   //get the mutex
        es.Post();      //tell the main thread that we have gotten the mutex
}

int MyMainThread::Main(int,char**) {
        TRACE_METHOD2("MyMainThread::Main");
        FThread *tp= new MyThread(0);
        tp->Start();
        es.Wait();      //wait until the thread has gotten the mutex
        int r=ms.Request();     //this should fail
        ASSERT(r!=0);
        return 0;
}
*/

/*
//---------------------------------------------------------------------------
//test that FSemaphore works
FSemaphore s(0);

void MyThread::Go() {
        TRACE_METHOD2("MyThread::Go");
        Yield();
        s.Signal();
        s.Signal();
        Yield();
        s.Signal();
}

int MyMainThread::Main(int,char**) {
        TRACE_METHOD2("MyMainThread::Main");
        FThread *tp= new MyThread(0);
        tp->Start();
        s.Wait();
        s.Wait();
        s.Wait();
        return 0;
}
*/



DEFBIFTHREADMAIN(MyMainThread);

//LOG_NEW(FileLog,"test.log");
