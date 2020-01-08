#ifndef _PRODUCTER_H_
#define _PRODUCTER_H_

#include "../include/yhshared.h"

using namespace android;

class ProductThread : public Thread {

	   public:
		  ProductThread(YhClientProxy* proxy):proxy(proxy),index(0){
			   LOGI("init product thread...");
		  }

		  ~ProductThread(){
			   //todo release resources.
			   LOGI("destroy product thread...");
		  }

		  bool threadLoop(){

			  LOGI("Product thread loop...");
			  sleep(30);
			  if(proxy != NULL){
				  Buffer buffer;
				  size_t result = proxy->obtainBuffer(&buffer);
				  LOGI("product thread obtain buffer result : %d", result);

				  if(result == RESULT_OK){
					  size_t size = buffer.mFrameCount * sizeof(char);
					  LOGI("buffer mFrameCount : %d, mRaw : %p", buffer.mFrameCount, buffer.mRaw);
			          size = buffer.mFrameCount * sizeof(char);
			          memset(buffer.mRaw, 0, size);

			          char* helloStr = "hello yanghui11";
					  sprintf(helloStr,"%d",index);
			          memcpy(buffer.mRaw, helloStr, sizeof(char) * strlen(helloStr));

					  index ++;
					  proxy->releaseBuffer(&buffer);
					  LOGI("product thread release buffer.");
				  } else {
					  LOGE("Product thread obtain buffer failed.");
				  }
			  }

			  return true;
		  }
		  

	   private:
		   YhClientProxy* proxy;
		   int index;

		  

};






#endif


