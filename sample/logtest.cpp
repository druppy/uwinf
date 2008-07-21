#include <dbglog.h>


void boo(void) {
	//internal kernel function
	TRACE_PROC0("boo");
	NOTE0("boo was called");
	WARN("boo was really called :-)");
}

void foo(void) {
	//documented api function
	TRACE_PROC1("foo");
	boo();
	NOTE1("foo is now exiting");
}

void main(void) {
	//application function
	int i=1;
	TRACE_PROC2("main");
	TRACE_FORMAT2("i=%d",i);
	NOTE2("main called");
	foo();

	ASSERT(i==0);
	VERIFY(i==0);
}

LOG_NEW(FileLog,"con");
