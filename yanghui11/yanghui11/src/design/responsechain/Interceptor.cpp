#include "../../../include/design/responseChain.h"


Response* 
FirstInterceptor :: intercept(Chain* chain){

	Request request = chain->getRequest();
	request.reason += "This is first interceptor .";
	printf("This is first interceptor .\n");
	Response* response = chain->proceed(request);

	return response;
}

Response* 
SecondInterceptor::intercept(Chain* chain){
	Request request = chain->getRequest();
	request.reason += "This is Second interceptor.";
	printf("This is Second interceptor.\n");
	Response* response = chain->proceed(request);
	
	return response;
}

Response* 
LastInterceptor::intercept(Chain* chain) {
  
	Request request = chain->getRequest();
	request.reason += "This is last interceptor.";
	printf("This is last interceptor.\n");
	Response* response = chain->proceed(request);
	if (response != NULL) {
		response->result = request.reason;
	}
	else {
		printf("pre chain return null response.\n");
		response = new Response;
		response->result = request.reason;
	}

	return response;

}

Response* 
Interface::intercept(Chain* chain) {

	printf("Interface intercept impl...");

	return NULL;

}
