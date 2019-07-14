#pragma once
#include<stdio.h>

/*
   装饰器模式
   装饰器的价值在于装饰，他并不影响被装饰类本身的核心功能。在一个继承的体系中，子类通常是互斥的。
*/


//基础接口
class Component {   
public:
	virtual void biu() = 0;
};


class ConcretComponent : public Component{

public:
	void biu() {
		printf("biubiubiu");
	};

};

class ConcreteDecorator : public Component{

public:
	ConcreteDecorator(Component* component) {
		this->component = component;
	};

	void biu() {
		printf("ready ? go !");
		this->component->biu();
	}

	~ConcreteDecorator()
	{
		delete component;
	}


private:
	Component* component;
};
