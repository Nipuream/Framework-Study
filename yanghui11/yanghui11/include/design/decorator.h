#pragma once
#include<stdio.h>

/*
   װ����ģʽ
   װ�����ļ�ֵ����װ�Σ�������Ӱ�챻װ���౾��ĺ��Ĺ��ܡ���һ���̳е���ϵ�У�����ͨ���ǻ���ġ�
*/


//�����ӿ�
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
