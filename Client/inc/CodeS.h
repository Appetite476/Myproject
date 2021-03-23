#pragma once
#include "MyCode.h"

typedef struct ANSWERMESSAGE {
	int	   num;
	char   Toclientid[12];
	char   serverid[12];
	char   Randnum_Server[64];
}AnswerMessage;

// 使用: 创建一个类对象
// 1. 编码 - 需要将外界数据插入当前对象中
// 2. 解码 - 需要待解码的字符串, 从中解析出一系列的数据
// 3. 接收解码返回值不需要释放
class CodeS :
	public MyCode
{
public:
	CodeS();

	CodeS(AnswerMessage *message);

	//统一编码
	virtual int msgEncode(char** outData, int& len);
	//统一解码
	virtual void* msgDecode(char* inData, int inLen);

	~CodeS();
private:
	AnswerMessage *message;
};

