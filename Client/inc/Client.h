#pragma once
#include "CodeC.h"
#include "DealShareM.h"

const int CLIENT_ILLEGAL = 100;
const int DATA_TAMPERED = 101;

class Client{
public:
	//初始化共享内存 连接共享内存
	Client();
	
	//功能窗口
	void ShowMenu();
	
	//生成随机字符串
	void MakeRandString(char *buf, size_t size);
	
	//秘钥协商
	int Secret_key_Negotiated(AskMessage *ask);
	
	//秘钥校验
	int Secret_key_Check();
	
	//秘钥注销
	int Secret_key_Cancel();
	
	//秘钥查看
	int Secret_key_Watch();
	
	~Client();
private:
	DealShareM* share;
};
