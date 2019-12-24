#ifndef _THREAD_BASE_H_
#define _THREAD_BASE_H_

#include <utils/Thread.h>
#include "../include/base.h"
#include <unistd.h>
#include <utils/Looper.h>

namespace android {
	

  class WorkThread : public Thread {
	  
	  public :
	    bool threadLoop(){
			
			LOGI("thread loop ...");
			sleep(3);
			return true;
		}
	
  };


  class Handler : public MessageHandler {

    public:
		void handleMessage(const Message& message){

            LOGI("handleMessage... message what : %d", message.what);
		}
  };


  class TestThread : public Thread {

     public:
	 	TestThread():Thread(true){   /** true 表示是否可调用Java函数**/
            looper = new Looper(true);
			handler = new Handler;
		}

	virtual bool threadLoop(){
       LOGI("thread loop ... wait 100 seconds.");
	   if(looper != NULL){
          looper->pollAll(100 * 1000);
	   }

	   return true; /** 返回true表示一直轮询调用，false表示只调用一次 **/
	}

	void sendMessage(){
       if(looper != NULL && handler != NULL){
          LOGI("looper->sendMessage");
		  looper->sendMessage(handler,Message(100));
	   }
	}

	inline sp<Looper>& getLooper(){return looper;}

	 private:
	 	sp<Looper> looper;
		sp<Handler> handler;

  };

 
  
}


#endif