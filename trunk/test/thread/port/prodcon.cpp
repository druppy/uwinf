/* Producer-consumer problem with circular bounded buffer
 * Taken from
 * Solution using BIF/Thread
 */

#include "bifthread.h"


typedef int Item;

#define maxbuf 10

Item buffer[maxbuf];
int head=0;
int tail=0;

FSemaphore notfull(maxbuf);	//counts empty spaces
FSemaphore notempty(0);		//counts full spaces
//Invariant: notfull+notempty == maxbuf



class Producer : public FThread {
public:
	virtual void Go();
protected:
	void produce(Item &i);
	void append(Item& i);
};


class Consumer : public FThread {
public:
	virtual void Go();
protected:
	void consume(Item &i);
	void take(Item &i);
};


void Producer::Go() {
	for(;;) {
		Item i;
		produce(i);
		append(i);
	}
}

void Producer::produce(Item &i) {
	i=8;	//production is easy
	printf("producer: producing %d\n",int(i));
}

void Producer::append(Item &i) {
	notfull.wait();
	buffer[head]=i;
	head= (head+1)%maxbuf
	notempty.signal();
}

void Consumer::Go() {
	for(;;) {
		Item i;
		take(i);
		consume(i);
	}
}

void Consumer::take(Item& i) {
	notempty.wait();
	i = buffer[tail];
	tail = (tail+1)%maxbuf;
	notfull.signal();
}

void Consumer::consume(Item &i) {
	printf("consumer: consuming %d\n",int(i));
}


void main(void) {
	Producer p;
	Consumer c;
	p.Start();
	c.Start();
	sleep(30000);
}
