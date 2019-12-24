#include "../../../include/design/responseChain.h"


Response*
RealInterceptorChain :: proceed(Request request) {
	
	printf("index : %d, size : %d \n",index,interfaces.size());
	if (index >= interfaces.size()) {
		printf("exceed interfaces size.");
		return NULL;
	}

	//找到指定元素的itor.
	list<Interface*>::iterator itor;
	itor = interfaces.begin();


	for (int i = 0; i < index; i++) {
		itor++;
	}

	this->index = index +1;
	this->request = request;

	Response* response = (*itor)->intercept(this);

	return response;
	
}

Request
RealInterceptorChain :: getRequest() {

	return request;

}