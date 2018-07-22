#ifndef _PROCESS_COMMUNICATE_H
#define _PROCESS_COMMUNICATE_H

/*

   进程间通信 -> 管道(pipe)

   #include <stdio.h>

   FILE *popen(const char *command, const char *open_mode);   command: 被调用起进程名+参数, open_mode: "r"/"w" 只能支持一种模式
   int pclose(File *stream_to_close);  调用pclose，只有在被调用起进程执行结束时，才生效


   管道更底层的调用：
    #include <unistd.h>
	int pipe(int file_descriptor[2]);  参数是文件描述符，不是文件流，需要用底层 read和write 调用来访问数据


	IPC : Inter-Process Communication

	1. 信号量 ： 它是一个特殊变量，只允许对它进行等待和发送信号这两种操作。
	    伪代码：
		                semaphore sv = 1;
						
						loop forever {
						      P(sv);
							  critical code section;
							  V(sv);
							  noncritical code section;
						}

		#include <sys/sem.h>

		int semctl(int sem_id, int sem_num, int command, ...);
		int semget(key_t key, int num_sems, int sem_flags);
		int semop(int sem_id, struct sembuf *sem_ops, size_t num_sem_ops);

     2.共享内存：两个不想关的进程通过逻辑内存地址去操作同一块区域的物理内存，一般对大块内存区域操作
	    
		#include <sys/shm.h>

		void *shmat(int shm_id, const void *shm_addr, int shmflg);  连接到 一个进程的地址空间，shm_id 共享内存的标识符，shm_addr 共享内存连接到当前进程中的地址位置，shmflg 是一组位标志
		int shmctl(int shm_id, int cmd, struct shmid_ds *buf);
		int shmdt(const void *shm_addr);  将共享内存从当前进程中分离，成功返回0，失败返回-1
		int shmget(key_t key, size_t size, int shmflg);  创建共享内存， key为共享内存段命名，size以字节为单位制定需要共享的内存容量，shmflg 权限标志 ,返回一个共享内存的标识符

		3. 消息队列：消息队列提供了一种从一个进程向另一个进程发送一个数据块的方法

		#include <sys/msg.h>

		int msgctl(int msqid, int cmd, struct msqid_ds *buf); 控制消息队列
		int msgget(key_t key, int msgflg);   创建和访问一个消息队列
		int msgrcv(int msqid, void *msg_ptr, size_t msg_sz, long int msgtype, int msgflg);  从一个消息队列中获取消息
		int msgsnd(int msqid, const void *msg_ptr, size_t msg_sz, int msgflg);  把消息添加到消息队列中


		补充：
		Android 端创建共享内存：
		1. const size_t pagesize = getpagesize();  //获取系统中内存页大小
		2. 创建共享内存， ashmem_create_region  在真实设备上打开 /dev/ashmem 设备得到一个文件描述符
		3. mapfd(fd, size);  //通过mmap 方式得到内存地址

		ashmem 设备驱动在这方面做了较大改动 和 优化。在 MemoryHeapBase 中已经帮我们实现

*/


#endif