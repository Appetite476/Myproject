#pragma once
#include "CodeS.h"
#include "CodeC.h"
#include "DealShareM.h"
#include "TcpSocket.h"
#include "MysqL.h"

const int CLIENT_ILLEGAL = 100;
const int DATA_TAMPERED = 101;

class ServerInfo{
public:
	char			serverID[12];	// 服务器端编号
	char			dbUse[24]; 		// 数据库用户名
	char			dbPasswd[24]; 	// 数据库密码
	char			dbSID[24]; 		// 数据库
	char 			dbIP[128];		// ip
	unsigned short 	sPort;			// 服务器绑定的端口
	int				maxnode;		// 共享内存最大网点树 客户端默认1个
	int 			shmkey;			// 共享内存keyid 创建共享内存时使用
};

class Server{
public:
	//初始化共享内存 连接共享内存
	Server(ServerInfo *info);
	
	//生成随机字符串
	void MakeRandString(char *buf, size_t size);
	
	//秘钥协商
	int Secret_key_Negotiated(AnswerMessage *answer, TcpSocket *sock, AskMessage *clientask);
	
	//秘钥校验
	int Secret_key_Check();
	
	//秘钥注销
	int Secret_key_Cancel();
	
	//秘钥查看0
	int Secret_key_Watch();
	
	~Server();
private:
	DealShareM* share;
	ServerInfo s_info;
	Mysql s_mysql;
};

void Work();