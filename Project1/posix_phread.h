#ifndef _POSIX_PHREAD_H
#define _POSIX_PHREAD_H

#include <pthread.h>

/*

  �߳�

  �߳��ǽ����ڲ���һ����������
  ������ִ��fork����ʱ�����������ý��̵�һ���������������ӵ���Լ��ı�����PID,����ʱ�����Ҳ�Ƕ����ģ�
  ����ִ����ȫ�����ڸ����̣��������д���һ���߳�ʱ���µ�ִ���߳̽�ӵ���Լ���ջ�����ӵ���Լ��ľֲ���������
  �������Ĵ����߹���ȫ�ֱ������ļ����������źŴ������͵�ǰ��Ŀ¼״̬��


  #include <pthread.h>
  int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);  //��������ǽ�Ҫ����ִ�еĺ����ʹ��ݸ��ú����Ĳ���
  void pthread_exit(void *retval);
  int pthread_join(pthread_t th, void **thread_return);  //�ڶ���������ʶ�̷߳��صĽ��

  ͬ��
  1. ʹ���ź���ͬ��
  #include <semaphore.h>
  int sem_init(sem_t *sem, int pshared, unsigned int value);  //psharedΪ0��ʾ��ǰ���̵ľֲ��ź���
  int sem_wait(sem_t *sem);   ��ԭ�Ӳ����ķ�ʽ���ź�����1
  int sem_post(sem_t *sem);  ��ԭ�Ӳ����ķ�ʽ���ź�����1
  int sem_destroy(sem_t *sem); �������е���Դ

  2.�û���������ͬ��
  #include <pthread.h>
  int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
  int pthread_mutex_lock(pthread_mutex_t *mutex);
  int pthread_mutex_unlock(pthread_mutex_t *mutex);
  int pthread_mutex_destroy(pthread_mutex_t *mutex);

  ȡ��һ���߳�
  #include <pthread.h>
  int pthread_cancel(pthread_t thread);
  int pthread_setcancelstate(int state, int *oldstate);
  int pthread_setcanceltype(int type, int *oldtype);


*/




#endif
