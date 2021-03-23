#include "CodecFactory.h"

CodecFactory::CodecFactory(){
	flag = false;
}

CodecFactory::CodecFactory(AskMessage * ask){
	flag = true;
	m_ask = ask;
}

void * CodecFactory::Create(){
	if (flag == true) {
		return new CodeC(m_ask);
	}

	return new CodeC;
}


CodecFactory::~CodecFactory(){
}
