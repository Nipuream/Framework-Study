#ifndef _PROCESS_COMMUNICATE_H
#define _PROCESS_COMMUNICATE_H

/*

   ���̼�ͨ�� -> �ܵ�(pipe)

   #include <stdio.h>

   FILE *popen(const char *command, const char *open_mode);   command: �������������+����, open_mode: "r"/"w" ֻ��֧��һ��ģʽ
   int pclose(File *stream_to_close);  ����pclose��ֻ���ڱ����������ִ�н���ʱ������Ч


   �ܵ����ײ�ĵ��ã�
    #include <unistd.h>
	int pipe(int file_descriptor[2]);  �������ļ��������������ļ�������Ҫ�õײ� read��write ��������������


	IPC : Inter-Process Communication

	1. �ź��� �� ����һ�����������ֻ����������еȴ��ͷ����ź������ֲ�����
	    α���룺
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

     2.�����ڴ棺��������صĽ���ͨ���߼��ڴ��ַȥ����ͬһ������������ڴ棬һ��Դ���ڴ��������
	    
		#include <sys/shm.h>

		void *shmat(int shm_id, const void *shm_addr, int shmflg);  ���ӵ� һ�����̵ĵ�ַ�ռ䣬shm_id �����ڴ�ı�ʶ����shm_addr �����ڴ����ӵ���ǰ�����еĵ�ַλ�ã�shmflg ��һ��λ��־
		int shmctl(int shm_id, int cmd, struct shmid_ds *buf);
		int shmdt(const void *shm_addr);  �������ڴ�ӵ�ǰ�����з��룬�ɹ�����0��ʧ�ܷ���-1
		int shmget(key_t key, size_t size, int shmflg);  ���������ڴ棬 keyΪ�����ڴ��������size���ֽ�Ϊ��λ�ƶ���Ҫ������ڴ�������shmflg Ȩ�ޱ�־ ,����һ�������ڴ�ı�ʶ��

		3. ��Ϣ���У���Ϣ�����ṩ��һ�ִ�һ����������һ�����̷���һ�����ݿ�ķ���

		#include <sys/msg.h>

		int msgctl(int msqid, int cmd, struct msqid_ds *buf); ������Ϣ����
		int msgget(key_t key, int msgflg);   �����ͷ���һ����Ϣ����
		int msgrcv(int msqid, void *msg_ptr, size_t msg_sz, long int msgtype, int msgflg);  ��һ����Ϣ�����л�ȡ��Ϣ
		int msgsnd(int msqid, const void *msg_ptr, size_t msg_sz, int msgflg);  ����Ϣ��ӵ���Ϣ������


		���䣺
		Android �˴��������ڴ棺
		1. const size_t pagesize = getpagesize();  //��ȡϵͳ���ڴ�ҳ��С
		2. ���������ڴ棬 ashmem_create_region  ����ʵ�豸�ϴ� /dev/ashmem �豸�õ�һ���ļ�������
		3. mapfd(fd, size);  //ͨ��mmap ��ʽ�õ��ڴ��ַ

		ashmem �豸�������ⷽ�����˽ϴ�Ķ� �� �Ż����� MemoryHeapBase ���Ѿ�������ʵ��

*/


#endif