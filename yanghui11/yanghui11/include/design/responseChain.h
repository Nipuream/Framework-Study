#pragma once
#include <string>
#include <list>

/*
   责任链模式
*/

using namespace std;

class Request {

public:
	string reason;
};

class Response {

public:
	string result;
};

class Interface {

public : 

	class Chain {
	public:
		virtual  Request  getRequest() = 0;
		virtual  Response* proceed(Request request) = 0;
	};
	virtual Response* intercept(Chain* chain) = 0;
};


class RealInterceptorChain : public Interface::Chain {

public :
	RealInterceptorChain(list<Interface*>& interfaces, Request& request , int index)
		: interfaces(interfaces), request(request), index(index) {}

	Response* proceed(Request request);
	Request getRequest();
	~RealInterceptorChain()
	{
		interfaces.clear();
	}

private:
	size_t index;
	Request request;
	list<Interface*> interfaces;

};

class FirstInterceptor : public Interface {

public:
	Response* intercept(Chain* chain);

};

class SecondInterceptor : public Interface {
public:
	Response* intercept(Chain* chain);
};

class LastInterceptor : public Interface {
public:
	Response* intercept(Chain* chain);
};
