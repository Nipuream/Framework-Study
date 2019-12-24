#include "thread_base.h"
#include "../include/base.h"


using namespace android;


void testAndroidThread(){
	sp<WorkThread> thread = new WorkThread;
	// name, priority, stack
	thread->run("yanghui",PRIORITY_DEFAULT,0);
	
	thread->requestExit();
}

void testEventThread(){

	sp<TestThread> thread = new TestThread;
	thread->run("looperThread",PRIORITY_DEFAULT,0);
	
	thread->sendMessage();
	sleep(5);
	thread->sendMessage();
  
}

int main(int argc, char**argv){


//  testAndroidThread();

   testEventThread();
  

   sleep(60);
   return 0;
} 


