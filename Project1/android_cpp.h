#ifndef _ANDROID_CPP_H_
#define _ANDROID_CPP_H_


/*

 Android cpp api.
 
 1. RefBase.h   sp  wp.

 2. #include <utils/Thread.h>
       sp<Thread> t(bool  canJavaCall);  
	   ʵ��  threadLoop ���� ����fasle ���˳�������һֱѭ������
	   t->run();

3. ͬ����
     
	     1. ͬ������ Mutex mLock;   -->  Mutex::Autolock _l(mLock);
		 2. �������� Condition

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
			 ...... //�޸��߼�
			 mThreadExitedCondition.broadcast();  //���еȴ��Ŷ��ᱻ����
			 mLock.unlock();  //�ͷ���

		 }
		 
		 3. ԭ������ 
		  <utils/Atomic.h>

		  void android_atomic_write(int32_t value, volatile int32_t* addr);  ��ֵ����
		  int32_t android_atomic_inc(volatile int32_t* addr);  ��1
		  int32_t android_atomic_dec(volatile int32_t* addr); ��1
		  int32_t android_atomic_add(int32_t value, volatile int32_t* addr); �ӷ�����
		  int32_t android_atomic_and(int32_t value, volatile int32_t* addr);
		  int32_t android_atomic_or(int32_t value, volatile int32_t* addr);
		  int android_atomic_cmpxchg(int32_t oldvalue, int32_t newvalue, volatile int32_t* addr);  ����ֵΪ0���������ԭ�Ӳ���
		 
		 */



#endif