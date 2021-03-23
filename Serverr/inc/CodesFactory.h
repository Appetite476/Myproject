#pragma once
#include "BaseFactory.h"
#include "CodeS.h"

class CodesFactory :
	public BaseFactory
{
public:
	CodesFactory();

	CodesFactory(AnswerMessage *answer);

	virtual void* Create();

	~CodesFactory();
private:
	bool flag;
	AnswerMessage * m_answer = NULL;
};

