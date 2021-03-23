#pragma once
#include "MyCode.h"

//CLIENTSTR
typedef struct ASKMESSAGE {
	int	   num;
	char   Clientid[12];
	char   Toserverid[12];
	char   AuthenticationCode[65]; //验证码
	char   Randnum_Client[64];
}AskMessage;

// 使用: 创建一个类对象
// 1. 编码 - 需要将外界数据插入当前对象中
// 2. 解码 - 需要待解码的字符串, 从中解析出一系列的数据
// 3. 接收解码返回值不需要释放
class CodeC :
	public MyCode
{
public:
	enum CmdType { NewOrUpdate = 1, Check, Revoke, View };
	CodeC();

	CodeC(AskMessage *m);

	//统一编码
	virtual int msgEncode(char** outData, int& len);
	//统一解码	析构函数释放返回值
	virtual void* msgDecode(char* inData, int inLen);

	~CodeC();

private:
	AskMessage *message = NULL;
};

