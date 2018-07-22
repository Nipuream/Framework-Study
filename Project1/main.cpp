

#include <binder/BinderService.h>
#include <pthread.h>
#include <sys/resource.h>
#include "yanghuiService.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

using namespace yanghui;
using namespace android;

int main(int argc, char **argv){
	setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_DISPLAY);
	sp<ProcessState> proc(ProcessState::self());
	sp<IServiceManager> sm(defaultServiceManager());
	sm->addService(String16(yanghuiService::getServiceName()), new yanghuiService);
	ProcessState::self()->startThreadPool();
	IPCThreadState::self()->joinThreadPool();
	return 0;
}