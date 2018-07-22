
#include "base_socket.h"
#include "base_log.h"


socket_operator::socket_operator(){
	LOGI("create default socket_operator object ~");
}


socket_operator::socket_operator(int port){
	this->port = port;
	LOGI("port : %d",this->port);
	init_socket();
}

int 
socket_operator::init_socket(){

	LOGD("start open socket ~");
	int socketId = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == socketId){
		LOGE("open socket failed ~");
		return FAILED;
	}

	this->socketId = socketId;
	return SUCCESSFUL;
}

int 
socket_operator::bind_socket(){
	//��socket�󶨵�ĳһ���˿ں�
	struct sockaddr_in address;

	//�� socket ��ַ
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;

	//�󶨵����е�ַ
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	//���˿�ת��Ϊ�����ֽ�˳��
	address.sin_port = htonl(this->port);

	if (-1 == bind(this->socketId, (struct sockaddr*) &address, sizeof(address))){
		LOGE("bind socket failed ~");
		return FAILED;
	}

	return SUCCESSFUL;

}

unsigned short 
socket_operator::get_socket_port(){

	unsigned short port = 0;
	struct sockaddr_in address;
	socklen_t addressLength = sizeof(address);

	//��ȡsocket ��ַ
	if (-1 == getsockname(this->socketId, (struct sockaddr*) &address, &addressLength)){
		LOGE("get sock name failed ~");
	}
	else{
	   //���˿�ת��Ϊ�����ֽ�˳��
		port = ntohs(address.sin_port);
	}

	return port;
}

void 
socket_operator::listen_socket(int backlog){

	LOGI("Listening on socket ...");
	if (-1 == listen(this->socketId, backlog)){
		LOGE("listen failed ~");
	}

}


int 
socket_operator::accept_socket(){

	struct sockaddr_in address;
	socklen_t addressLength = sizeof(address);

	//�����͵ȴ������Ŀͻ�����
	LOGI("Wait for a client connection...");

	int clientSocket = accept(this->socketId, (struct sockaddr*) &address, &addressLength);
	
	//����ͻ�socket ��Ч
	if (-1 == clientSocket){
		LOGE(" client is invalid socket ...");
	}
	else{
		LOGD("Client connection from : %d", &address);
	}

	return clientSocket;
}

ssize_t 
socket_operator::receive_from_socket(int sd, char* buffer, size_t buffersize){

	LOGI("Receiving from the socket ...");
	ssize_t  recvSize = recv(sd, buffer, buffersize - 1, 0);

	//�������ʧ��
	if (-1 == recvSize){
		LOGE("receive from socket failed ~");
	}
	else{
	    //��NULL��β�������γ�һ���ַ���
		buffer[recvSize] = NULL;

		//������ݽ��ܳɹ�
		if (recvSize > 0){
			LOGI("Received %d bytes : %s",recvSize,buffer);
		}
		else{
			LOGD("Client disconnected");
		}
	}

	return recvSize;
}


ssize_t 
socket_operator::send_to_socket(int sd,void* buffer, size_t bufferLength){

	LOGI("Sending to the socket...");
	ssize_t sentSize = send(sd, buffer, bufferLength,0);

	if (-1 == sentSize){
		LOGE("send to socket  failed ~");
	}
	else{
		if (sentSize > 0){
			LOGI("Sent %d bytes : %s",sentSize,buffer);
		}
		else{
			LOGE("Client disconnected");
		}
	}

	return sentSize;
}

