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


#ifdef TEST_MEMORY

	if(yanghuiService != NULL){
		LOGI(" bpclient is not null..");
		sp<callback> cb = new callback();
		yanghuiService->setcallback(cb);
		status_t result = yanghuiService->openMemory("YhMemory", 1024);
		LOGI(">>>  open memory >>> result : %d", result);
	}

#endif

    if(yanghuiService != NULL){
		LOGI(" bpclient is not null..");
		const sp<IMemory>& iMem = yanghuiService->getIMemory();
		size_t size = iMem->size();
		int* addrPoint = static_cast<int*>(iMem->pointer());		
		LOGI("Get server Imem size : %d, imem addr : &d",size, *addrPoint);
	}   

	sleep(30);

#ifdef TEST_MEMORY

    if(yanghuiService != NULL){

	    int closeRes = yanghuiService->closeMemory();
		LOGI("close memory result : %d", closeRes);
	}
#endif

	
	return 0;
}
