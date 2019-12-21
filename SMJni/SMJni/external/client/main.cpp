#include <stdio.h>
#include "callback.h"
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include "../include/base.h"
#include <unistd.h>

using namespace android;

int main(int argc, char** argv){
	
	sp<IServiceManager> sm = defaultServiceManager();
	sp<IBinder> binder = sm->getService(String16("yanghui.IyanghuiService"));
	sp<IyanghuiService> yanghuiService = interface_cast<IyanghuiService>(binder);
	if(yanghuiService != NULL){
		LOGI(" bpclient is not null..");
		sp<callback> cb = new callback();
		yanghuiService->setcallback(cb);
		status_t result = yanghuiService->openMemory("YhMemory", 1024);
		LOGI(">>>  open memory >>> result : %d", result);
	}
	
	sleep(30);

    if(yanghuiService != NULL){

	    int closeRes = yanghuiService->closeMemory();
		LOGI("close memory result : %d", closeRes);
	}
	
	return 0;
}