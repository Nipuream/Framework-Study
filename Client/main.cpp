#include "yanghuiService.h"
#include "IyanghuiService.h"
#include <binder/Binder.h>
#include "base_log.h"

using namespace yanghui;

int main(int argc, char **argv){

	LOGD("argc = %d , *argv params  = %s", argc, *(argv + 1));
	
 
	sp<IServiceManager> sm(defaultServiceManager());	
	sp<IBinder> binder = sm->getService(String16("yanghui_service"));
	sp<IyanghuiService> service = NULL;

	if (binder != NULL){
		 service = interface_cast<IyanghuiService>(binder);
	}
	
	/** Share memory example. **/

	if (service != NULL){

		char *params = *(++argv);
		LOGD("params =  %s", params);

		if (params == NULL || (strcmp(params, "write")) == 0){

			sp<IMemory> memory = service->native_createShareMemory();
			sp<IMemoryHeap> heap = memory->getMemory();
			int fd = heap->getHeapID();
			void*  address = heap->getBase();

			LOGI("fd = %d, address = %x");
			shared_use_st* use_st = (shared_use_st *)address;

			char* values = "Hello Share memory.";
			memcpy(use_st->text, values, sizeof(char)*strlen(values));
			LOGI("copy client values to share memory.");
			use_st->written = 1;
		}
		else if (strcmp(params, "read") == 0){
			
			void* result = service->native_readMemory(sizeof(char) * 10);
			LOGD("result : %s", result);

		}
	}

	return 0;
}