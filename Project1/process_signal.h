#ifndef _PROCESS_SIGNAL_
#define _PROCESS_SIGNAL_

#include <stdlib.h>

/*

ʲô�ǽ��̣�
һ������������һ�������̵߳ĵ�ַ�ռ����Щ�߳�����Ҫ��ϵͳ��Դ

�������̵߳�����
������cpu��Դ�������С��λ���߳���cpu���ȵ���С��λ���߳��ǽ����ڽ��̵Ļ����ϵ�һ�γ������е�λ��

1.�����½���
#include <stdlib.h>
int system(const char *string);  ʹ�����������

2.�滻����ӳ��  exec�������԰ѵ�ǰ�����滻Ϊһ���½��̣����³�������֮��ԭ���ĳ���Ͳ��������ˡ�
#include <unistd.h>
char **environ;
int exec1(const char *path, const char *arg0, ..., (char *)0);
int exec1p(const char *file, const char *arg0, ..., (char *)0);
int execle(const char *path, const  char *arg0, ..., (char *)0, char *const envp[]);

int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);

3.���ƽ���
#include <sys/types.h>
#include <unistd.h>

pid_t fork(void);  //������fork���÷��ص����ӽ��̵�pid���ӽ��̷��ص���0.


4.�ȴ�һ������
#include <sys/types.h>
#include <sys/wait.h>

pid_t wait(int *stat_loc);  waitϵͳ���ý���ͣ������ֱ�������ӽ��̽���Ϊֹ�����ص�pid��ͨ�����Ѿ��������е��ӽ��̵�PID,
                                             �� stat_loc���ǿ�ָ�룬״̬��Ϣ����д������ָ���λ��

pid_t waitpid(pid_t pid, int *stat_loc, int options); �����ȴ�ĳ���ض����̵Ľ���
pid_t waitpid(child_pid, (int *)0, WNOHANG); �ø����������Եļ��ĳ���ض����ӽ����Ƿ�����ֹ

5.��ʬ����
�ӽ�����ֹʱ�����븸����֮�����ϵ���ᱣ�֣�ֱ��������Ҳ������ֹ�򸸽��̵���wait�Ÿ��������ˣ����̱��д����ӽ��̵ı���
���������ͷţ���Ȼ�ӽ����Ѿ��������У�������Ȼ������ϵͳ�С�

*/


/*

�ź�

�ź���UNIX��Linuxϵͳ��ӦĳЩ������������һ���¼������յ����źŵĽ��̻���Ӧ�Ĳ�ȡһЩ�ж���

#include <signal.h>
void (*signal(int sig,void (*func)(int)))(int);  //�����ź�


#include <sys/types.h>
#include <signal.h>
int kill(pid_t pid,int sig);  //�����ź�

#include <unistd.h>
unsigned int alarm(unsigned int seconds); //��seconds����ŷ���һ��SIGALRM�ź�,����������Ϊ0����ȡ�����е���������

#include <signal.h>
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact); //һ����׳���źŽӿ�
sigaction �ṹ�����ٰ������³�Ա��
          void(*) (int)sa_handler
		  sigset_t sa_mask
		  int sa_flags






*/






#endif
