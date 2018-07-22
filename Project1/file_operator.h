#ifndef _FILE_OPERATOR_
#define _FILE_OPERATOR_


/**

   Unix/Linux �ļ�����

*  Unix/Linux �Ƚ���Ҫ�������豸�ļ�:
*  /dev/console ϵͳ����̨��������Ϣ�������Ϣͨ���ᱻ���͵�����豸
* /dev/tty  ���һ�������п����ն˵Ļ����Ǹ�����豸�ļ�������������ն�(���̺���ʾ��������̺ʹ���)�ı���(�߼��豸)
* /dev/null  ���豸������д������豸���������������
*/

/**
*  ϵͳ���ú���
*  open
*  read
*  write
*  close
*  ioctl
*  lseek ���ļ��������Ķ�дָ���������
*  fstat  ������򿪵��ļ���������ص��ļ���״̬��Ϣ
*  stat    lstat  ���ص���ͨ���ļ����鵽��״̬��Ϣ
*  dup dup2 һ�ָ����ļ��������ķ���
*  ʹ��ϵͳ���û�Ӱ��ϵͳ�����ܣ�һ��ʹ�ÿ⺯��,�⺯���������������ݿ鳤��Ҫ��ʱ����ִ�еײ�ϵͳ���ã���ͼ���Ľ�����ϵͳ���õĿ�����
*/


/**
*  ��׼I/O�� <stdio.h>

*  �ڳ���������ʱ���������ļ������Զ��򿪵ģ��ֱ��� stdin(��׼����)��stdout(��׼���)��stderr(��׼�������)
*  FILE *fopen(const char *filename, const char *mode);
*  size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
*  size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
*  int fclose(FILE *stream);
*  int fflush(FILE *stream); ���ļ����������δд����������д����
*  int fseek(FILE * stream, long int offset, int whence);
*
*  int fgetc(FILE *stream); int getc(FILE *stream); int getchar();  fgetc�������ļ�����ȡ����һ���ֽڲ�������Ϊһ���ַ����أ��������ļ�β����󷵻�EOF
*  int fputc(int c, FILE *stream); int putc(int c, FILE *stream); int putchar(int c);
*
*  char *fgets(char *s, int n, FILE *stream); �Ѷ������ַ�д��sָ����ַ���
*  char *gets(char *s);   �ӱ�׼�����ȡ���ݲ����������Ļ��з����ڽ��յ��ַ�����β������һ��null�ֽڡ�
*
*  int printf(const char *format, ...);  ���ַ����������׼���
*  int sprintf(char *s, const char *format, ...);  ���ַ��������һ����β���ַ�д���ַ�����
*  int fprintf(FILE *stream, const char *format, ...); ���ַ���������ƶ����ļ���
*  ת������
*           %d, %i : ��ʮ���Ƹ�ʽ���һ������
*           %o, %x: �԰˽��ƻ�ʮ���������һ������
*           %c: ���һ���ַ�
*           %s: ���һ���ַ���
*           %f: ���һ���������ȣ�������
*           %e: �Կ�ѧ��������ʽ���һ��˫���ȸ�����
*           %g: ��ͨ�ø�ʽ���һ��˫���ȸ�����
*
*
*   ��printf���ƣ�scanf�Ƕ�ȡ
*   int scanf(const char * format, ...);
*   int fscanf(FILE *stream, const char *format, ...);
*   int sscanf(const char *s, const char *format, ...);
*   ת������һ���� %[] : ��ȡһ���ַ�����  %[^\n] �������н���
*
*  fgetpos: ����ļ����ĵ�ǰ����д��λ��
*  fsetpos: �����ļ����ĵ�ǰ����д��λ��
*  ftell: �����ļ�����ǰ����д��λ�õ�ƫ��ֵ
*  rewind: �����ļ�����Ķ�дλ��
*  freopen: ����ʹ��һ���ļ���
*  setvbuf: �����ļ����Ļ������
*  remove: �൱��unlink���������path���ļ��У����൱��rmdir����
*
*  int ferror(FILE *stream); ����һ���ļ����Ĵ����ʶ�������ʶ�����þͷ���һ������ֵ�����򷵻���
*  int feof(FILE *stream);  ����һ���ļ������ļ�β��ʶ������ñ�ʶ�����þͷ��ط���ֵ�����򷵻���
*  int clearerr(FILE *stream); �����streamָ����ļ������ļ�β��ʶ�ʹ����ʶ��
*
*  int fileno(FILE *stream);  ����ͨ���ļ�����ȡ�������ĸ��ײ���ļ���������ʧ�ܷ���-1
*  FILE *fdopen(int fildes, const char *mode); ���Խ��Ѵ򿪵��ļ���������һ���ļ������� �� mode �Ƿ���ģʽ
*
*/

/*
*  ��������ʧ����
*  #include <errno.h>
*  extern int errno;
*  <stdio.h>      ==>  void perror(const char *s);   ��������Ϣ��ӡ���ն�
*  <string.h>     ==>  char *strerror(int errnum);   ��������Ϣ������ƶ�������
*
*/


/*
*  �ļ���Ŀ¼ά��
*
*  #include <sys/stat.h>
*  int chmod(const char *path, mode_t mode); �ı��ļ���Ŀ¼�ķ���Ȩ��
*
*
*  #include <sys/types.h>
*  #include <unistd.h>
*  int chown(const char *path, uid_t owner, gid_t group); �����û�������chown���ı�һ���ļ�������
*
*/

/*
*   unlink ��link ��symlink ϵͳ����
*   #include <unistd.h>
*   int unlink(const char *path);  �����ļ�������������û�н��̴���������ļ��ͻᱻɾ��
*   int link(const char *path1, const char *path2);  ����һ��ָ�������ļ�path1�������ӣ���Ŀ¼����path2������
*   symlink(const char *path1,const char *path2);  ʹ��symlink�����������ӣ��������Ӹ��ļ���������
*/

/*
*  mkdir �� rmdir ϵͳ����
*  #include <sys/types/h>
*  #include <sys/stat.h>
*   int mkdir(const char *path, mode_t mode);
*
*   #include <unistd.h>
*   int rmdir(const char *path);
*/

/*
*   chdir �� getcwd
*   #include <unistd.h>
*   int chdir(const char *path); �л�Ŀ¼��������cd����
*   char *getcwd(char *buf, size_t size);  ȷ���Լ��ĵ�ǰ����Ŀ¼
*/

/*
*  ɨ��Ŀ¼
*  #include <sys/types.h>
*  #include <dirent.h>
*  DIR *opendir(const char *name);  ��һ��Ŀ¼������һ��Ŀ¼��
*
*  #include <sys/types.h>
*  #include <dirent.h>
*  struct dirent *readdir(DIR *dirp);  ִ��readdir��������һ��ָ�룬��ָ��ָ��Ľṹ�ﱣ����Ŀ¼��dirp����һ��Ŀ¼����й�����
*
*  #include <sys/types.h>
*  #include <dirent.h>
*  long int telldir(DIR *dirp);  telldir��������ֵ��¼��һ��Ŀ¼����ĵ�ǰλ��
*
*  #include <sys/types.h>
*  #include <dirent.h>
*  void seekdir(DIR *dirp, long int loc); ����Ŀ¼��dirp��Ŀ¼��ָ��
*
*  #include <sys/types.h>
*  #include <dirent.h>
*  int closedir(DIR *dirp); �ر�һ��Ŀ¼�����ͷ���֮��������Դ
*
*/

/*
*  ������ļ�ϵͳprocfs ==> /procĿ¼��ʽչ��  ����ֱ�ӻ�ȡ����������ں˵���Ϣ�����޸�����
*
*  fcntl ϵͳ���ã�����fcntlϵͳ���ã����ԶԴ򿪵��ļ�������ִ�и��ֲ��������������ǽ��и��ơ���ȡ�������ļ���������־����ȡ�������ļ�״̬��־���Լ����������ļ�����
*  mmap ���ڴ�ӳ�䣬mmap��������һ�ο��Ա��������������д���ڴ棬һ������������޸Ŀ��Ա��������򿴼���
*
*  #include <sys/mman.h>
*  void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off); ����һ��ָ��һ���ڴ������ָ�룬���ڴ����������ͨ��һ���򿪵��ļ����������ʵ��ļ������������
*  int msync(void *addr, size_t len, int flags); �Ѹ��ڴ�ε�ĳ�����ֻ������е��޸�д�ص���ӳ����ļ���
*  int munmap(void *addr, size_t len); �ͷ��ڴ��

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
