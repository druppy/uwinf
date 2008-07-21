#define BIFINCL_MAINTHREAD
#define BIFINCL_SEMAPHORES
#include "thread\bifthread.h"
#include <stdlib.h>
#include "log.h"

/* A solution to 'The Dining Philosophers' problem using BIF/Thread */

class Philosopher : public FThread {
	int i;

	void eat();
	void think();
public:
	Philosopher(int n) { i=n; }
	virtual void Go();
};

FMutexSemaphore fork[5];
FSemaphore room(4);

FEventSemaphore dead[5];	//used for termination

void Philosopher::Go() {
	for(int cycles = rand()%10; cycles; cycles--) {
		think();

		room.Wait();
		fork[i].Request();
		fork[(i+1)%5].Request();

		eat();

		fork[i].Release();
		fork[(i+1)%5].Release();
		room.Signal();
	}

	//tell mainthread that we have died of a colesterol disease :-)
	dead[i].Post();
}


void Philosopher::think() {
	//Sleep(rand()%1000);
}

void Philosopher::eat() {
	//yum-yum
	//Sleep(rand()%1000);
}

class MyMainThread : public FMainThread {
public:
        MyMainThread(int argc, char **argv) : FMainThread(argc,argv) {}
        int Main(int,char **);
};

int MyMainThread::Main(int, char **) {
	int i;

	//create the philosophers and start them
	Philosopher *p[5];
	for(i=0; i<5; i++) {
		p[i] = new Philosopher(i);
		p[i]->Start();
	}

	//wait for the philosophers to die
	for(i=0; i<5; i++)
		dead[i].Wait();

	return 0;
}

FMainThread *FMainThread::MakeMainThread(int argc, char **argv) {
        return new MyMainThread(argc,argv);
}

int main(int argc, char **argv) {
        LOG_NEW(FileLog,"philosop.log");

        return _BIFMain(argc,argv);
}

