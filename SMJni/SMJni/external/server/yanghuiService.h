#ifndef _YANGHUI_SERVICE_H_
#define _YANGHUI_SERVICE_H_


#include "../include/IyanghuiService.h"
#include "../include/Icallback.h"
#include <binder/BinderService.h>
#include "../include/yhshared.h"

namespace android {
	
	class yanghuiService : 
	   public BinderService<yanghuiService>,
	   public BnyanghuiService {
	   
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
		   

	};
	
	
}


#endif
