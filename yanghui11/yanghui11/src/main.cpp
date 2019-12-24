
#include<stdlib.h>
#include<stdio.h>

#include "../include/algorithm/sort.h"
#include "../include/design/responseChain.h"
#include "../include/design/decorator.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#define random(x,y) ((rand()%(y -x)) + x + 1)
using namespace std;

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

void createTest() {

	ofstream file("yanghui.txt");

	srand((int)time(0)); 
	for (int i = 0; i < 6; i++)
	{
		file << random(0,33) << " ";
	}

	file << random(0,16) << " ";
	file.close();

}

inline int add(int a, int b) {
	return a + b;
}

typedef int(*callback) (int, int);

int main(){

	//testDecorator();

	//createTest();


	callback cb = add;
	int result = cb(4, 5);
	printf("result : %d", result);
	system("PAUSE");
	return 0;
  
}











