#include "../include/IyanghuiService.h"
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include "../include/Icallback.h"
#include "../include/base.h"
#include <utils/String8.h>


namespace android {
	
	
	enum {
		OPEN_MEMORY,
		SET_CALLBACK,
		CLOSE_MEMORY,
		GET_IMEMORY
	};
	
	
	//�ͻ���
	class BpyanghuiService : public BpInterface<IyanghuiService> {
		
		public : 
		   BpyanghuiService(const sp<IBinder>& impl) : BpInterface<IyanghuiService>(impl){}
		   
		   virtual status_t openMemory(const char* fileName, status_t size) {
			   
			   LOGI(" >>>>>>  BpyanghuiService::openMemory >>>> \n");
			   Parcel data,reply;
			   String8 str(fileName);
			   data.writeString8(str);
			   data.writeInt32(size);
			   remote()->transact(OPEN_MEMORY, data, &reply);
			   return reply.readInt32();
		   }
		   
		    virtual int setcallback(const sp<Icallback>& callback){
				
				LOGI(" >>>>>   BpyanghuiService::setcallback >>>>> \n");
				Parcel data, reply;
				data.writeStrongBinder(callback->asBinder());
				remote()->transact(SET_CALLBACK, data, &reply);
				return reply.readInt32();
			}

			virtual status_t closeMemory() {

			   LOGI(">>>> BpyanghuiService::closeMemory >>> \n");
			   Parcel data,reply;
			   remote()->transact(CLOSE_MEMORY, data, &reply);
			   return reply.readInt32();
			}

			
			virtual sp<IMemory> getIMemory() {
               LOGI(">>> BpyanghuiService::getIMemory >>> \n");
			   Parcel data,reply;
			   remote()->transact(GET_IMEMORY, data, &reply);
			   sp<IMemory> iMem = interface_cast<IMemory>(reply.readStrongBinder());
			   return iMem;
			}
	};

	
	  IMPLEMENT_META_INTERFACE(yanghuiService, "yanghui.IyanghuiService");
	  
	  //�����
	  status_t BnyanghuiService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags){
		  
		  LOGI(">>>>>>>>> BnyanghuiService::onTransact >>>>>>>\n");
		  switch(code){
			  case OPEN_MEMORY:{
				  
				  LOGI(">>>>>  BnyanghuiService::openMemory >>>>");
				  String8 str = data.readString8();
				  status_t size = data.readInt32();
				  reply->writeInt32(openMemory(str.string(), size));
				  return NO_ERROR;
			  }
			  break;
			  case SET_CALLBACK:{
				  LOGI(">>>>> BnyanghuiService::setCallback >>>> \n");
				  sp<Icallback> callback= interface_cast<Icallback>(data.readStrongBinder());
				  reply->writeInt32(setcallback(callback));
				  return NO_ERROR;
			  }
			  break;
			  case CLOSE_MEMORY:{

                 LOGI(">>>>  BnyanghuiService::closeMemory >>> \n");
				 status_t result = closeMemory();
				 reply->writeInt32(result);
                 return NO_ERROR;
			  }
			  break;
			  case GET_IMEMORY:{
			  	  LOGI(">>> GET_IMEMROY >>>>> \n");
              	  sp<IMemory> imem = getIMemory();
				  reply->writeStrongBinder(imem->asBinder());
				  return NO_ERROR;
			  	}
				break;
			  default:{
				  LOGI(">>>> BnyanghuiService::onTransact defalut... >>>");
				  return BBinder::onTransact(code,data,reply,flags);
			  }
		  }
	  }

	
	
	
}
