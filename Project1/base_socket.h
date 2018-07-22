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
�׽��� Socket.

���ԣ������͡�Э��

��AF_INET :ָ����Internet����
AF_UNIX : һ̨δ�����ļ�������׽���Ҳ����ʹ������������ĵײ�Э�����ļ�������/��������ĵ�ַ�����ļ���
...

���ͣ� stream����
datagram : ���ݱ�

1. �����׽��֣�
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);

2. �����׽���: ���׽��ֹ�����һ���ļ�ϵͳ��·����

#include <sys/socket.h>
 int bind(int socket, const struct sockaddr * address, size_t address_len);  //�󶨳ɹ�������0

 3. �����׽��ֶ���
 #include <sys/socket.h>
 int listen(int socket, int backlog); backlog��ʾ �����ɵ�δ�������ӵ������Ŀ

 4. ��������
  #include <sys/socket.h>
  int accept(int socket, struct sockaddr * address, size_t *address_len);

  5. �������ӣ��ͻ�������ͷ��������
  #include <sys/socket.h>
  int connect(int socket, const struct sockaddr *address, size_t address_len);

  6. �������ֽ�ת��Ϊ�����ֽ���
  #include <netint/in.h>

  host to network.
  unsigned long int htonl(unsigned long int hostlong);
  unsigned short int htons(unsigned short int hostshort); 

  network to host.
  unsigned long int ntohl(unsigned long int netlong);
  unsigned short int ntohs(unsigned short int netshort);

  7. ��ȡ������Ϣ
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
