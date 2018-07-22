#ifndef _PROCESS_SIGNAL_
#define _PROCESS_SIGNAL_

#include <stdlib.h>

/*

什么是进程？
一个其中运行着一个或多个线程的地址空间和这些线程所需要的系统资源

进程与线程的区别？
进程是cpu资源分配的最小单位，线程是cpu调度的最小单位，线程是建立在进程的基础上的一次程序运行单位。

1.启动新进程
#include <stdlib.h>
int system(const char *string);  使用情况不理想

2.替换进程映像  exec函数可以把当前进程替换为一个新进程，在新程序启动之后，原来的程序就不再运行了。
#include <unistd.h>
char **environ;
int exec1(const char *path, const char *arg0, ..., (char *)0);
int exec1p(const char *file, const char *arg0, ..., (char *)0);
int execle(const char *path, const  char *arg0, ..., (char *)0, char *const envp[]);

int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);

3.复制进程
#include <sys/types.h>
#include <unistd.h>

pid_t fork(void);  //父进程fork调用返回的是子进程的pid，子进程返回的是0.


4.等待一个进程
#include <sys/types.h>
#include <sys/wait.h>

pid_t wait(int *stat_loc);  wait系统调用将暂停父进程直到它的子进程结束为止，返回的pid，通常是已经结束运行的子进程的PID,
                                             若 stat_loc不是空指针，状态信息将被写入它所指向的位置

pid_t waitpid(pid_t pid, int *stat_loc, int options); 用来等待某个特定进程的结束
pid_t waitpid(child_pid, (int *)0, WNOHANG); 让父进程周期性的检查某个特定的子进程是否已终止

5.僵尸进程
子进程终止时，它与父进程之间的联系还会保持，直到父进程也正常终止或父进程调用wait才告结束。因此，进程表中代表子进程的表项
不会立刻释放，虽然子进程已经不再运行，但它仍然存在于系统中。

*/


/*

信号

信号是UNIX和Linux系统相应某些条件而产生的一个事件，接收到该信号的进程会相应的采取一些行动。

#include <signal.h>
void (*signal(int sig,void (*func)(int)))(int);  //处理信号


#include <sys/types.h>
#include <signal.h>
int kill(pid_t pid,int sig);  //发送信号

#include <unistd.h>
unsigned int alarm(unsigned int seconds); //在seconds秒后安排发送一个SIGALRM信号,将参数设置为0，则取消所有的闹钟请求

#include <signal.h>
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact); //一个健壮的信号接口
sigaction 结构体至少包括以下成员：
          void(*) (int)sa_handler
		  sigset_t sa_mask
		  int sa_flags






*/






#endif
