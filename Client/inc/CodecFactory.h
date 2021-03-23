#pragma once
#include "BaseFactory.h"
#include "CodeC.h"

class CodecFactory :
	public BaseFactory
{
public:
	CodecFactory();

	CodecFactory(AskMessage *ask);

	virtual void* Create();

	~CodecFactory();
private:
	bool flag;
	AskMessage *m_ask = NULL;
};

