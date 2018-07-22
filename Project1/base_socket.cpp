
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
	//将socket绑定到某一个端口号
	struct sockaddr_in address;

	//绑定 socket 地址
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;

	//绑定到所有地址
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	//将端口转换为网络字节顺序
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

	//获取socket 地址
	if (-1 == getsockname(this->socketId, (struct sockaddr*) &address, &addressLength)){
		LOGE("get sock name failed ~");
	}
	else{
	   //将端口转换为主机字节顺序
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

	//阻塞和等待进来的客户连接
	LOGI("Wait for a client connection...");

	int clientSocket = accept(this->socketId, (struct sockaddr*) &address, &addressLength);
	
	//如果客户socket 无效
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

	//如果接受失败
	if (-1 == recvSize){
		LOGE("receive from socket failed ~");
	}
	else{
	    //以NULL结尾缓冲区形成一个字符串
		buffer[recvSize] = NULL;

		//如果数据接受成功
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

