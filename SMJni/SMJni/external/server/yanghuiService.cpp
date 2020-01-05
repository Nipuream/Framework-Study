#include "yanghuiService.h"
#include "../include/base.h"
#include <cutils/ashmem.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <binder/MemoryDealer.h>

namespace android {
	
	
	status_t yanghuiService::openMemory(const char* fileName, status_t size){
		
		LOGI(">>>  yanghuiService  openMemory >>>> %s, %d", fileName, size);

		int fd = ashmem_create_region(fileName, size);
		void* result = (void*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		if(result != NULL){
             this->fd = fd;
			 this->addr = result;
			 LOGI("open memory successfully.");

             const char* helloStr = "hello memory";
             memset(result, 0, size);
			 memcpy(addr, helloStr, sizeof(char) * strlen(helloStr));
			 
			 sp<Icallback> c = this->mcallback;
		     if(c != NULL){
			    c->notifyCallback(fd);
		     }
		}
		
	
		return 1;
		
	}
	
	int yanghuiService::setcallback(const sp<Icallback>& callback){
		
		LOGI(" >>>>  yanghuiService setcallback >>> %p", callback.get());
		this->mcallback = callback;
		return 1;
	}


	status_t yanghuiService::closeMemory(){
	 
		LOGI(">>>  yanghuiService::closeMemory >>>> \n");
		int result = munmap(this->addr, 1024);
		if(result < 0){
           LOGE("munmap memory failed ...");
		}

		close(this->fd);
		this->fd = -1;
		return 1;
	}


	
	 sp<IMemory> yanghuiService::getIMemory(){

	    LOGI(">>> yanghuiService::getIMemory %p >>>> \n",(this->imem).get());
		return this->imem;
	}


	yanghuiService::yanghuiService(){

		LOGI("Enter yanghuiService ~");
		sp<MemoryDealer> mMemoryDealer = new MemoryDealer(1024 * 100, "yanghui_Memory");
		size_t size = sizeof(yanghui_track_cblk_t);
		LOGI("yanghui_track_cblk_t size : %d ",size);
		sp<IMemory> tmp = mMemoryDealer->allocate(size);

        yanghui_track_cblk_t* mCblk = static_cast<yanghui_track_cblk_t>(tmp->pointer());
		new (mCblk) yanghui_track_cblk_t();

		this->serverProxy = new YhServerProxy();
		
		this->imem = tmp;
	}

	yanghuiService::~yanghuiService(){

		if(this->imem != NULL){
            //(this->imem)->getHeap()->dispose();
			LOGI("destroy  yanghuiservice .");
		}

	}

	status_t yanghuiService::onTransact(
                                uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
								uint32_t flags){
	
        LOGI(">>>  yanghuiService onTransact >>> ");
		return BnyanghuiService::onTransact(code, data, reply, flags);
    }

}
