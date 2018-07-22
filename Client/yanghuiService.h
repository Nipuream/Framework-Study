#ifndef _YANGHUI_H
#define _YANGHUI_H_

#include <utils/RefBase.h>
#include <binder/BinderService.h>
#include "IyanghuiService.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace yanghui{

#define MEMORY_SIZE 1024 * 10
#define OP_FILE_NAME "/sdcard/yanghui.txt"
#define TEXT_SZ 2048
#define SHARE_MEMORY_PATH "/data/system/yanghui11.txt"
typedef unsigned char byte;

	class yanghuiService : public BinderService<yanghuiService>,
		public BnyanghuiService{

	public:

		static yanghuiService* getInstance();

		static char const* getServiceName(){
			return "yanghui_service";
		}

		yanghuiService();
		virtual ~yanghuiService();

		status_t native_opensocket();
		//status_t native_createShareMemory();
		sp<IMemory> native_createShareMemory();
		char* native_readMemory(size_t len);

	private:
		static yanghuiService* sInstance;
		void initFunc();
	};


  struct shared_use_st {

		int written; //非0可读，0代表可写
		char text[TEXT_SZ];

	};


}


#endif

