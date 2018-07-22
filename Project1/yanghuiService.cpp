#include "yanghuiService.h"
#include "base_log.h"
#include "base_socket.h"
#include "socket_test.h"
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include "file_operator.h"
#include "posix_phread.h"
#include <utils/Thread.h>


namespace yanghui{

	void* access_data_from_sm(void *arg);

	yanghuiService* yanghuiService::sInstance = 0;
	socket_operator* socket_test;
	file_accessor *accssor;
	sp<MemoryHeapBase> mMemHeap;
	sp<MemoryBase> mMemBase;
	shared_use_st *use_st = NULL;

	yanghuiService* yanghuiService::getInstance(){
		return yanghuiService::sInstance;
	}

	yanghuiService::yanghuiService(){
		LOGI("yanghuiService  create .");
		initFunc();
		LOGI("start a thread.");
		pthread_t a_thread;
		int res = pthread_create(&a_thread, NULL, access_data_from_sm, NULL);
		if (res != 0){
			LOGE("Thread create failed");
		}

		yanghuiService::sInstance = this;
	}

	void yanghuiService::initFunc(){
		LOGI("enter initFunc");
		socket_test = new socket_operator(8080);
	}

	yanghuiService::~yanghuiService(){
		LOGI("yanghuiService destroy.");
	}

	status_t yanghuiService::native_opensocket(){
		LOGI("enter native_opensocket");
		return open_socket();
	}

	sp<IMemory>  yanghuiService::native_createShareMemory(){

		LOGI("enter native_createShareMemory");
		if (mMemHeap == NULL){
			//create ashmem memory.
			mMemHeap = new MemoryHeapBase(MEMORY_SIZE, 0, "yanghui service memory");
			mMemBase = new MemoryBase(mMemHeap, 0, MEMORY_SIZE);
			//init file operator.
			accssor = new file_accessor(OP_FILE_NAME);
		}
		else{
			LOGI("share memory has created ~");
		}

		void* address = mMemHeap->getBase();
		shared_use_st *use_st = new(address)shared_use_st();
		use_st->written = 0;

		return mMemBase;
	}

	char*  yanghuiService::native_readMemory(size_t len) {
	   
		LOGI("enter native_readMemory");
		LOGI("Need read data len : %d",len);

		void *result = NULL;
		if (accssor != NULL){
			result = accssor->read_file();
		}

		LOGD("native_readMemory result : %s", result);
		return (char *)result;
	}


	void *access_data_from_sm(void *arg){

		while (1){
			if (mMemHeap.get() != NULL) {

				void* address = mMemHeap->getBase();
				shared_use_st *use_st = (shared_use_st *)address;

				//不为0，表示可读
				if (use_st->written != 0){
		
					size_t size = strlen(use_st->text) * sizeof(char);
					char *buf = new char[size];
					memset(buf, 0, size);

					memcpy(buf, use_st->text, size );
					LOGI("read buf is : %s", buf);

					if (strlen(buf) > 0){
						  int res = accssor->put_file(buf, size);
						  LOGD("write into file res : %d", res);
						  if (res != 0){
							  use_st->written = 0;
						  }
					}

				}
			}
			sleep(5);
		}
		return NULL;
	}

}
