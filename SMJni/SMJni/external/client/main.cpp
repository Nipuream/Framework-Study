#include <stdio.h>
#include "callback.h"
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include "../include/base.h"
#include <unistd.h>
#include "producter.h"

using namespace android;

sp<YhClientProxy> proxy;

void test(){
   LOGI("provider pointer.");
}

void productData(sp<YhClientProxy>& proxy){

    for(;;){
		
		Buffer buffer;
		size_t result = proxy->obtainBuffer(&buffer);
		LOGI("result = %d", result);
		if(result == RESULT_OK){
			LOGI("buffer mFrameCount : %d, mRaw : %p", buffer.mFrameCount, buffer.mRaw);
		
			size_t size = buffer.mFrameCount * sizeof(char);
			memset(buffer.mRaw, 0, size);
		
			char* helloStr = "hello yanghui11";
			size_t strSize = sizeof(char) * strlen(helloStr);

            for(int i = 0;i< (size/strSize);i++){
				memcpy(buffer.mRaw + i * strSize, helloStr, strSize);
			}

			if(size % strSize != 0){
                memcpy(buffer.mRaw + (size/strSize)*strSize, helloStr,size%strSize);
			}
			
			proxy->releaseBuffer(&buffer);
		}

		sleep(30);
	}
}

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

		size = sizeof(yanghui_track_cblk_t);
		int len = sizeof(char);
		size_t bufferSize = len * 50;
		

		yanghui_track_cblk_t* mCblk = static_cast<yanghui_track_cblk_t *>(iMem->pointer());

		if(mCblk != NULL){

          new(mCblk) yanghui_track_cblk_t();
		  void* mBuffer = (char*)mCblk + sizeof(yanghui_track_cblk_t);
		  proxy = new YhClientProxy(mCblk, mBuffer, 50, len);

          productData(proxy);
		}
		

	}   

	sleep(60 * 1000 * 30);

#ifdef TEST_MEMORY

    if(yanghuiService != NULL){

	    int closeRes = yanghuiService->closeMemory();
		LOGI("close memory result : %d", closeRes);
	}
#endif

	
	return 0;
}
