#include "callback.h"
#include "binder/Parcel.h"
#include "../include/base.h"
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

namespace android {
	
	int callback::notifyCallback(int a){
		LOGI(" >>>  callback notifyCallback  -> a : %d >>>",a);

		void* result = (void*)mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, a, 0);
		char* aces = new char[100];
		memcpy(aces, result, sizeof(char)*100);
		LOGI("get memory result : %s", aces);

		delete[] aces;
		return 1;
	}
	
	status_t callback::onTransact(uint32_t code,
				const Parcel& data,
				Parcel* reply,
				uint32_t flags){
				
	    LOGI(" >>>   callback onTransact >>>>");
		return Bncallback::onTransact(code, data, reply, flags);
    }

}