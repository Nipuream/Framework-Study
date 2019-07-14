
#include<stdlib.h>
#include<stdio.h>

#include "../include/algorithm/sort.h"
#include "../include/design/responseChain.h"
#include "../include/design/decorator.h"

template<class T>
int length(T& arr) {
	return sizeof(arr) / sizeof(arr[0]);
}

void testSort() {
	int value[10] = { 4,81,68,19,6,10,62,77,13,87 };
	//bubbleSort(value,length(value));
	//insertSort(value,length(value));
	quickSort(value, 0, length(value) - 1);

	for (int i = 0; i < 10; i++) {
		printf(" %d ", value[i]);
	}
}

void testResponseChain() {

	Request request;
	list<Interface*> lists;

	Interface* firstInterceptor = new FirstInterceptor;
	Interface* secondInterceptor = new SecondInterceptor;
	Interface* lastInterceptor = new LastInterceptor;

	lists.push_back(firstInterceptor);
	lists.push_back(secondInterceptor);
	lists.push_back(lastInterceptor);

	request.reason = "Net request -->";

	RealInterceptorChain* chain = new RealInterceptorChain(lists, request, 0);
	Response* response = chain->proceed(request);
	string result = response->result;
	printf("result : %s",result.c_str());

	delete chain;

}

void testDecorator() {

	ConcretComponent* concrentComponent = new ConcretComponent;
	Component* component = new ConcreteDecorator(concrentComponent);
	component->biu();

}

int main(){

	testDecorator();
	system("PAUSE");

	return 0;
  
}











