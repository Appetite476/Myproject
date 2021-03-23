#include "CodesFactory.h"

CodesFactory::CodesFactory(){
	flag = false;
}

CodesFactory::CodesFactory(AnswerMessage * answer){
	flag = true;
	m_answer = answer;
}

void * CodesFactory::Create()
{
	if (flag == true) {
		return new CodeS(m_answer);
	}

	return new CodeS;
}

CodesFactory::~CodesFactory()
{
}
