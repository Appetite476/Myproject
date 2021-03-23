#pragma once
#include "MyASN1.h"
class MyCode :
	protected SequenceASN1
{
public:
	MyCode();
	//统一编码
	virtual int msgEncode(char** outData, int& len);
	//统一解码
	virtual void* msgDecode(char* inData, int inLen);

	virtual ~MyCode();
};

