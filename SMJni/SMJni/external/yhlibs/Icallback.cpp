#include "../include/Icallback.h"
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include "../include/IyanghuiService.h"
#include "../include/base.h"

namespace android {
	
	enum {
		NOTIFY_CALLBACK,
	};

	//客户端
	class Bpcallback : public BpInterface<Icallback>{
		
		public :
		  Bpcallback(const sp<IBinder>& impl) : BpInterface<Icallback>(impl){}
		  
           virtual int notifyCallback(int a){
			   LOGI("=============  Icallback.cpp Bpcallback::notifyCallback ================\n");
			   Parcel data,reply;
			   data.writeDupFileDescriptor(a);
			   remote()->transact(NOTIFY_CALLBACK, data, &reply);
			   return reply.readInt32();
		   }
		
	};
	
	
	
    IMPLEMENT_META_INTERFACE(callback, "yanghui.Icallback");
   
    //服务端
	status_t Bncallback::onTransact(uint32_t code,
                                     const Parcel& data,
                                     Parcel* reply,
                                     uint32_t flags){
		
		LOGI("===========  Icallback.cpp Bncallback onTransact  ================\n");
		switch(code){
			case NOTIFY_CALLBACK:{
				LOGI(">>>>>>>>>>>>>> Icallback.cpp onTransact notify callback >>>>>>>>>> \n");
				reply->writeInt32(notifyCallback(dup(data.readFileDescriptor())));
				return NO_ERROR;
			}
			break;
			default:{
				LOGI(" >>>>>>>>>>>>>>>>  Icallback.cpp >>>>>>>>>> Bncallback:: onTransact >>> \n");
			    return BBinder::onTransact(code, data, reply, flags);	
			}			
		}								 
	}
	
	
	
}