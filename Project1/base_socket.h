#ifndef _BASE_H
#define _BASE_H
#include <sys/socket.h>

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
//sockaddr_un
#include <sys/un.h>

//htons,sockaddr_in
#include <netinet/in.h>

//inet_ntop
#include <arpa/inet.h>

//close,unlink
#include <unistd.h>

//offsetof
#include <stddef.h>

// strerror_r , memset
#include <string.h>

#include <sys/endian.h>


/*
套接字 Socket.

属性：域、类型、协议

域：AF_INET :指的是Internet网络
AF_UNIX : 一台未联网的计算机的套接字也可以使用这个域，这个域的底层协议是文件的输入/输出，它的地址就是文件名
...

类型： stream：流
datagram : 数据报

1. 创建套接字：
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);

2. 命名套接字: 将套接字关联到一个文件系统的路径名

#include <sys/socket.h>
 int bind(int socket, const struct sockaddr * address, size_t address_len);  //绑定成功，返回0

 3. 创建套接字队列
 #include <sys/socket.h>
 int listen(int socket, int backlog); backlog表示 可容纳的未处理连接的最大数目

 4. 接受连接
  #include <sys/socket.h>
  int accept(int socket, struct sockaddr * address, size_t *address_len);

  5. 请求连接：客户端请求和服务端连接
  #include <sys/socket.h>
  int connect(int socket, const struct sockaddr *address, size_t address_len);

  6. 将主机字节转化为网络字节序
  #include <netint/in.h>

  host to network.
  unsigned long int htonl(unsigned long int hostlong);
  unsigned short int htons(unsigned short int hostshort); 

  network to host.
  unsigned long int ntohl(unsigned long int netlong);
  unsigned short int ntohs(unsigned short int netshort);

  7. 获取网络信息
   #include <netdb.h>
   struct hostent *gethostbyaddr(const void *addr, size_t len, int type);
   struct hostent *gethostbyname(const char *name);

    struct hostent {
	   char *h_name;
	   char **h_aliases;
	   int h_addrtype;
	   int h_length;
	   char **h_addr_list
	};

	#include <netdb.h>
	struct servent *getservbyname(const char* name, const char *proto);
	struct servent *getservbyport(int port, const char *proto);

	struct servent {
	   char *s_name;
	   char **s_aliases;
	   int s_port;
	   char *s_proto;
	};
*/

#define SUCCESSFUL 0
#define FAILED -1

#define MAX_LOG_MESSAGE_LENGTH 256
#define MAX_BUFFER_SIZE 80


struct socket_operator{

public:
	socket_operator() ;
	socket_operator(int port);
	~socket_operator();
	int bind_socket();
	unsigned short get_socket_port();
	void listen_socket(int backlog);
	int accept_socket();
	ssize_t receive_from_socket(int sd,char* buffer, size_t buffersize);
	ssize_t send_to_socket(int sd,void* buffer, size_t bufferLength);

private :
	int port;
	int socketId;
	int init_socket();

};





#endif 
