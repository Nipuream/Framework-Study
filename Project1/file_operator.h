#ifndef _FILE_OPERATOR_
#define _FILE_OPERATOR_


/**

   Unix/Linux 文件操作

*  Unix/Linux 比较重要的三个设备文件:
*  /dev/console 系统控制台，错误信息和诊断信息通常会被发送到这个设备
* /dev/tty  如果一个进程有控制终端的话，那个这个设备文件就是这个控制终端(键盘和显示屏，或键盘和窗口)的别名(逻辑设备)
* /dev/null  空设备，所有写向这个设备的输出都将被丢弃
*/

/**
*  系统调用函数
*  open
*  read
*  write
*  close
*  ioctl
*  lseek 对文件描述符的读写指针进行设置
*  fstat  返回与打开的文件描述符相关的文件的状态信息
*  stat    lstat  返回的是通过文件名查到的状态信息
*  dup dup2 一种复制文件描述符的方法
*  使用系统调用会影响系统的性能，一般使用库函数,库函数在数据满足数据块长度要求时安排执行底层系统调用，这就极大的降低了系统调用的开销。
*/


/**
*  标准I/O库 <stdio.h>

*  在程序启动的时候，有三个文件流是自动打开的，分别是 stdin(标准输入)、stdout(标准输出)、stderr(标准错误输出)
*  FILE *fopen(const char *filename, const char *mode);
*  size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
*  size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
*  int fclose(FILE *stream);
*  int fflush(FILE *stream); 将文件流里的所有未写出数据立刻写出。
*  int fseek(FILE * stream, long int offset, int whence);
*
*  int fgetc(FILE *stream); int getc(FILE *stream); int getchar();  fgetc函数从文件流里取出下一个字节并把它作为一个字符返回，当到达文件尾或错误返回EOF
*  int fputc(int c, FILE *stream); int putc(int c, FILE *stream); int putchar(int c);
*
*  char *fgets(char *s, int n, FILE *stream); 把读到的字符写到s指向的字符串
*  char *gets(char *s);   从标准输入读取数据并丢弃遇到的换行符，在接收的字符串的尾部加上一个null字节。
*
*  int printf(const char *format, ...);  把字符串输出到标准输出
*  int sprintf(char *s, const char *format, ...);  把字符串输出和一个结尾空字符写到字符串中
*  int fprintf(FILE *stream, const char *format, ...); 把字符串输出到制定的文件流
*  转化符：
*           %d, %i : 以十进制格式输出一个整数
*           %o, %x: 以八进制或十六进制输出一个整数
*           %c: 输出一个字符
*           %s: 输出一个字符串
*           %f: 输出一个（单精度）浮点数
*           %e: 以科学计数法格式输出一个双精度浮点数
*           %g: 以通用格式输出一个双精度浮点数
*
*
*   和printf类似，scanf是读取
*   int scanf(const char * format, ...);
*   int fscanf(FILE *stream, const char *format, ...);
*   int sscanf(const char *s, const char *format, ...);
*   转化符多一个： %[] : 读取一个字符集合  %[^\n] 读到换行结束
*
*  fgetpos: 获得文件流的当前（读写）位置
*  fsetpos: 设置文件流的当前（读写）位置
*  ftell: 返回文件流当前（读写）位置的偏移值
*  rewind: 重置文件流里的读写位置
*  freopen: 重新使用一个文件流
*  setvbuf: 设置文件流的缓冲机制
*  remove: 相当于unlink函数，如果path是文件夹，则相当于rmdir函数
*
*  int ferror(FILE *stream); 测试一个文件流的错误标识，如果标识被设置就返回一个非零值，否则返回零
*  int feof(FILE *stream);  测试一个文件流的文件尾标识，如果该标识被设置就返回非零值，否则返回零
*  int clearerr(FILE *stream); 清除由stream指向的文件流的文件尾标识和错误标识。
*
*  int fileno(FILE *stream);  可以通过文件流获取到底是哪个底层的文件描述符，失败返回-1
*  FILE *fdopen(int fildes, const char *mode); 可以将已打开的文件符创建成一个文件流操作 ， mode 是访问模式
*
*/

/*
*  函数调用失败码
*  #include <errno.h>
*  extern int errno;
*  <stdio.h>      ==>  void perror(const char *s);   将错误信息打印到终端
*  <string.h>     ==>  char *strerror(int errnum);   将错误信息输出到制定缓冲区
*
*/


/*
*  文件和目录维护
*
*  #include <sys/stat.h>
*  int chmod(const char *path, mode_t mode); 改变文件或目录的访问权限
*
*
*  #include <sys/types.h>
*  #include <unistd.h>
*  int chown(const char *path, uid_t owner, gid_t group); 超级用户可以用chown来改变一个文件的属主
*
*/

/*
*   unlink 、link 、symlink 系统调用
*   #include <unistd.h>
*   int unlink(const char *path);  减少文件的链接数，当没有进程打开它，这个文件就会被删除
*   int link(const char *path1, const char *path2);  创建一个指向已有文件path1的新链接，新目录项由path2给出。
*   symlink(const char *path1,const char *path2);  使用symlink创建符号链接，不会增加该文件的链接数
*/

/*
*  mkdir 和 rmdir 系统调用
*  #include <sys/types/h>
*  #include <sys/stat.h>
*   int mkdir(const char *path, mode_t mode);
*
*   #include <unistd.h>
*   int rmdir(const char *path);
*/

/*
*   chdir 和 getcwd
*   #include <unistd.h>
*   int chdir(const char *path); 切换目录，类似于cd命令
*   char *getcwd(char *buf, size_t size);  确定自己的当前工作目录
*/

/*
*  扫描目录
*  #include <sys/types.h>
*  #include <dirent.h>
*  DIR *opendir(const char *name);  打开一个目录并建立一个目录流
*
*  #include <sys/types.h>
*  #include <dirent.h>
*  struct dirent *readdir(DIR *dirp);  执行readdir函数返回一个指针，该指针指向的结构里保存着目录流dirp中下一个目录项的有关资料
*
*  #include <sys/types.h>
*  #include <dirent.h>
*  long int telldir(DIR *dirp);  telldir函数返回值记录着一个目录流里的当前位置
*
*  #include <sys/types.h>
*  #include <dirent.h>
*  void seekdir(DIR *dirp, long int loc); 设置目录流dirp的目录项指针
*
*  #include <sys/types.h>
*  #include <dirent.h>
*  int closedir(DIR *dirp); 关闭一个目录流并释放与之关联的资源
*
*/

/*
*  特殊的文件系统procfs ==> /proc目录形式展现  可以直接获取驱动程序和内核的信息，和修改配置
*
*  fcntl 系统调用：利用fcntl系统调用，可以对打开的文件描述符执行各种操作，包括对它们进行复制、获取和设置文件描述符标志，获取和设置文件状态标志，以及管理建议性文件锁。
*  mmap ：内存映射，mmap函数建立一段可以被两个或更多程序读写的内存，一个程序对它的修改可以被其他程序看见。
*
*  #include <sys/mman.h>
*  void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off); 创建一个指向一段内存区域的指针，该内存区域与可以通过一个打开的文件描述符访问的文件的内容相关联
*  int msync(void *addr, size_t len, int flags); 把该内存段的某个部分或整段中的修改写回到被映射的文件中
*  int munmap(void *addr, size_t len); 释放内存段

 */

#include <stdio.h>
#include <unistd.h>

struct file_accessor{

public:
	file_accessor(const char* file_name);
	~file_accessor(void);

	void *read_file() ;
	int put_file(void* args, size_t len) ;

private :
	const char* file_name;
	FILE* read_stream;
	FILE* write_stream;

};


#endif
