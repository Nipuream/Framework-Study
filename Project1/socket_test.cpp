#include "socket_test.h"
#include "base_log.h"

socket_operator* socket_test = new socket_operator(8080);


int open_socket(){

	int bind_result = socket_test->bind_socket();
	if (bind_result == FAILED){
	  LOGE("bind socket failed ~");
	  return YANGHUI_ERROR;
	}

	socket_test->listen_socket(4);

	int socketClient = socket_test->accept_socket();

	char buffer[MAX_BUFFER_SIZE];
	ssize_t recvSize;
	ssize_t sentSize;

	while (1){

	recvSize = socket_test->receive_from_socket(socketClient, buffer, MAX_BUFFER_SIZE);

	if (recvSize == 0){
	break;
	}

	sentSize = socket_test->send_to_socket(socketClient, buffer, MAX_BUFFER_SIZE);

	if (sentSize == 0){
	break;
	}

	}

	close(socketClient);
	return YANGHUI_OK;

}
