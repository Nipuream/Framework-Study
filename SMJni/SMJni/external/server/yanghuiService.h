#ifndef _YANGHUI_SERVICE_H_
#define _YANGHUI_SERVICE_H_

#include "../include/IyanghuiService.h"
#include "../include/Icallback.h"
#include <binder/BinderService.h>
#include "../include/yhshared.h"
#include <unistd.h>

namespace android {

    class ResumeThread;
	
	class yanghuiService : 
	   public BinderService<yanghuiService>,
	   public BnyanghuiService {

	   friend class ResumeThread;
       friend class BinderService<yanghuiService>;
       public:
       static const char* getServiceName() { return "yanghui.IyanghuiService"; }  
       virtual status_t openMemory(const char* fileName, status_t size);
       virtual int setcallback(const sp<Icallback>& callback);
	   virtual status_t closeMemory();
	   yanghuiService();
	   ~yanghuiService();
	   virtual sp<IMemory> getIMemory();
	   
       virtual     status_t    onTransact(
                                uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
								uint32_t flags);
								
		protected:
		   sp<Icallback> mcallback;
		   int fd;
		   void* addr;
		   sp<IMemory> imem;
		   sp<YhServerProxy> serverProxy;
		   sp<ResumeThread> thread;
		   

	};


    class ResumeThread : public Thread {
	 
	 public :
	   ResumeThread(yanghuiService* service):service(service){
          LOGI("ResumeThread init... service address : %p", service);
	   }
	   ~ResumeThread(){
	   	  //todo release resource.
          LOGI("destroy resume thread ...");
	   }
	   
	   bool threadLoop(){
		   
		   LOGI("Resume thread loop ...");
		   sleep(60);
		   if(service != NULL){
              sp<YhServerProxy> proxy = service->serverProxy;
			  if(proxy != NULL){
			  	  Buffer buffer;
				  buffer.mFrameCount = 50;
                  size_t result = proxy->obtainBuffer(&buffer);
				  LOGI("Resume thread obtain buffer result %d", result);

				  if(result == RESULT_OK){

                      LOGI("buffer mFrameCount : %d, mRaw : %p", buffer.mFrameCount, buffer.mRaw);
                      size_t size = buffer.mFrameCount * sizeof(char);
					  char* s = new char[size];
					  memcpy(s,buffer.mRaw,size);
					  LOGI("resume thread obtain buffer : %s", s);

					  proxy->releaseBuffer(&buffer);
					  LOGI("resume thread relase buffer.");
				  } else {
                      LOGE("obtain buffer failed...");
				  }
			  }
		   }
		   
		   return true;
	   }

	 private :
	 	yanghuiService* service;
 };

	
}


#endif
