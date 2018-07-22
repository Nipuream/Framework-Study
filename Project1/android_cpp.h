#ifndef _ANDROID_CPP_H_
#define _ANDROID_CPP_H_


/*

 Android cpp api.
 
 1. RefBase.h   sp  wp.

 2. #include <utils/Thread.h>
       sp<Thread> t(bool  canJavaCall);  
	   实现  threadLoop 方法 返回fasle 则退出，否则一直循环调用
	   t->run();

3. 同步：
     
	     1. 同步锁： Mutex mLock;   -->  Mutex::Autolock _l(mLock);
		 2. 条件锁： Condition

		 eg:
		 Condition mThreadExitedCondition;

		 status_t Thread::requestExitAndWait(){
		     ......
			 Mutex::Autolock _l(mLock);
			 ......
			 mThreadExitedCondition.wait(mLock);
		 }

		 int Thread::_threadLoop(void *user){

		     ......
			 mLock.lock();
			 ...... //修改逻辑
			 mThreadExitedCondition.broadcast();  //所有等待着都会被唤醒
			 mLock.unlock();  //释放锁

		 }
		 
		 3. 原子锁： 
		  <utils/Atomic.h>

		  void android_atomic_write(int32_t value, volatile int32_t* addr);  赋值操作
		  int32_t android_atomic_inc(volatile int32_t* addr);  加1
		  int32_t android_atomic_dec(volatile int32_t* addr); 减1
		  int32_t android_atomic_add(int32_t value, volatile int32_t* addr); 加法操作
		  int32_t android_atomic_and(int32_t value, volatile int32_t* addr);
		  int32_t android_atomic_or(int32_t value, volatile int32_t* addr);
		  int android_atomic_cmpxchg(int32_t oldvalue, int32_t newvalue, volatile int32_t* addr);  返回值为0，则进行了原子操作
		 
		 */



#endif