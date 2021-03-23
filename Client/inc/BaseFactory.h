#pragma once
class BaseFactory
{
public:
	BaseFactory();

	//工厂造类
	virtual void* Create() = 0;

	~BaseFactory();
};

