#ifndef _POSIX_PHREAD_H
#define _POSIX_PHREAD_H

#include <pthread.h>

/*

  线程

  线程是进程内部的一个控制序列
  当进程执行fork调用时，将创建出该进程的一个副本，这个进程拥有自己的变量和PID,它的时间调度也是独立的，
  它的执行完全独立于父进程，当进程中创建一个线程时，新的执行线程将拥有自己的栈（因此拥有自己的局部变量），
  但与它的创建者共享全局变量、文件描述符、信号处理函数和当前的目录状态。


  #include <pthread.h>
  int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);  //最后两个是将要启动执行的函数和传递给该函数的参数
  void pthread_exit(void *retval);
  int pthread_join(pthread_t th, void **thread_return);  //第二个参数标识线程返回的结果

  同步
  1. 使用信号量同步
  #include <semaphore.h>
  int sem_init(sem_t *sem, int pshared, unsigned int value);  //pshared为0表示当前进程的局部信号量
  int sem_wait(sem_t *sem);   以原子操作的方式给信号量减1
  int sem_post(sem_t *sem);  以原子操作的方式给信号量加1
  int sem_destroy(sem_t *sem); 清理所有的资源

  2.用互斥量进行同步
  #include <pthread.h>
  int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
  int pthread_mutex_lock(pthread_mutex_t *mutex);
  int pthread_mutex_unlock(pthread_mutex_t *mutex);
  int pthread_mutex_destroy(pthread_mutex_t *mutex);

  取消一个线程
  #include <pthread.h>
  int pthread_cancel(pthread_t thread);
  int pthread_setcancelstate(int state, int *oldstate);
  int pthread_setcanceltype(int type, int *oldtype);


*/




#endif
