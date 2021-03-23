#include "Server.h"
#include "CodeC.h"
#include "CodeS.h"
#include "CodecFactory.h"
#include "CodesFactory.h"
#include "DealShareM.h"
#include "ItcastLog.h"
#include "TcpSocket.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

//初始化客户端 
Server::Server(ServerInfo *info){
	memcpy(&s_info, info, sizeof(s_info));
	share = new DealShareM(s_info.shmkey, s_info.maxnode);
	share->ConnectShareM(0);
	s_mysql.Connect_DB(s_info.dbSID, s_info.dbIP, s_info.dbUse, s_info.dbPasswd);
}

//生成随机字符串
void Server::MakeRandString(char *buf, size_t size){
	memset(buf, 0x00, size);
	char temp[] = "~@#^)[]{}(=+-";
	int type = 0;
	for(int i = 0; i < size - 1; i++){
		type = rand() % 4;
		switch(type){
		case 0://数字
			buf[i] = '0' + rand() % 10;
			break;
		case 1://大写
			buf[i] = 'A' + rand() % 26;
			break;
		case 2://小写
			buf[i] = 'a' + rand() % 26;
			break;
		case 3://特殊字符
			buf[i] = temp[(rand() % (strlen(temp)))];
			break;
		default:
			break;
		}
	}
	return;
}

//秘钥协商
int Server::Secret_key_Negotiated(AnswerMessage *answer, TcpSocket *sock, AskMessage *clientask){
	bool OK = true;
	char *EncodeOut = NULL;
	int EncodeOutLen, len, i;
	char key[64] = {0};
	unsigned char md[SHA256_DIGEST_LENGTH] = {0};
	do{
		//判断是否是合法clientid
		if(!s_mysql.Check_Client(clientask->Clientid)){
			answer->num = CLIENT_ILLEGAL;
			OK = false;
			memset(answer->Randnum_Server, '1' , sizeof(answer->Randnum_Server));
			cout << "clientid not legal..." << endl;
			break;
		}
		cout << "客户端数据合理..." << endl;
		//判断数据是否被篡改
		sprintf(key, "#%s+%s#", clientask->Clientid, clientask->Toserverid);
		HMAC_CTX * ctx = HMAC_CTX_new();
		HMAC_Init(ctx, key, strlen(key), EVP_sha256());
		HMAC_Update(ctx, (const unsigned char *)clientask->Randnum_Client, strlen(clientask->Randnum_Client));
		HMAC_Final(ctx, md, (unsigned int *)&len);
		HMAC_CTX_free(ctx);
		char temp[2*SHA256_DIGEST_LENGTH+1] = {0};
		//以十六进制写入temp
		for(i = 0; i < SHA256_DIGEST_LENGTH; i++){
			sprintf(&temp[2 * i], "%02x", md[i]);
		}
		if(strcmp(clientask->AuthenticationCode, temp) != 0){
			cout << "被篡改" <<endl;
			answer->num = DATA_TAMPERED;
			OK = false;
			memset(answer->Randnum_Server, '1' , sizeof(answer->Randnum_Server));
			break;
		}
		answer->num = s_mysql.Next_Id_Seckeyinfo();
		//生成随机字符串
		MakeRandString(answer->Randnum_Server, sizeof(answer->Randnum_Server));
		
	}while(false);
	/*cout << "num: " << answer->num << endl;
	cout << "toclient : " << answer->Toclientid << endl;
	cout << "server : " << answer->serverid << endl;
	cout << "rand :" << answer->Randnum_Server << endl;*/
	//编码
	BaseFactory *Factory = new CodesFactory(answer);
	MyCode *code = (CodeS *)Factory->Create();
	code->msgEncode(&EncodeOut, EncodeOutLen);
	
	//发送数据
	int ret = sock->sendMsg(EncodeOut, EncodeOutLen);
	if(ret != 0){
		if(ret == TcpSocket::TimeoutError){
			cout << "发送超时..." << endl;
			delete EncodeOut;
			delete Factory;
			delete code;
			sock->disConnect();
			return ret;
		}
		cout << "发送失败,请查看日志" <<endl;
		delete EncodeOut; 
		delete Factory;
		delete code;
		sock->disConnect();
		return ret;
	}
	cout <<"send successful" <<endl;
	do{
		//判断是否需要生成秘钥
		if(!OK){
			break; 
		}
		//生成秘钥
		char buf[256] = {0};
		unsigned char md1[SHA_DIGEST_LENGTH] = {0};
		sprintf(buf, "@%s+%s@", clientask->Randnum_Client, answer->Randnum_Server);
		//unsigned char *SHA1(const unsigned char *d, size_t n, unsigned char *md);
		SHA1((unsigned char *)buf, strlen(buf), md1);
		
		//写入共享内存
		NodeSHMInfo info;
		memset(&info, 0x00, sizeof(info));
		info.status = 1;
		strcpy(info.clientId, clientask->Clientid);
		strcpy(info.serverId, clientask->Toserverid);
		info.seckeyid = answer->num;
		for(i = 0; i < SHA_DIGEST_LENGTH; i++){
			sprintf((char *)&info.seckey[2 * i], "%02x", md1[i]);
		}
		share->Insert(&info);
		//写数据库
		if(!s_mysql.Insert_SecInfo(clientask->Clientid, info.seckey)){
			cout << "数据库写失败，查看日志..." << endl;
		}
		
		cout << "秘钥: " << info.seckey << endl;
		cout << "协商成功..." << endl;
		
		NodeSHMInfo *tmp = (NodeSHMInfo *)share->Read_ShareM(clientask->Clientid, clientask->Toserverid);
		cout << tmp->status << "\t" << tmp->clientId << "\t" << tmp->serverId << "\t" 
		     << tmp->seckeyid << "\t" << tmp->seckey << endl;
		
		delete tmp;
		tmp = NULL;
	}while(false);
	//释放资源
	if(EncodeOut != NULL){
		delete EncodeOut;
		EncodeOut = NULL;
	}
	if(Factory != NULL){
		delete Factory;
		Factory = NULL;
	}
	if(code != NULL){
		delete code;
		code = NULL;
	}
	return NOERR;
}

//秘钥校验
int Server::Secret_key_Check(){
	return NOERR;
}

//秘钥注销
int Server::Secret_key_Cancel(){
	return NOERR;
}

//秘钥查看
int Server::Secret_key_Watch(){
	return NOERR;
}

//退出客户端
Server::~Server(){
	share->DisconnectShareM();
	s_mysql.Disconnect_DB();
	if(share != NULL){
		delete share;
		share = NULL;
	}
	cout << "已退出..." << endl;
}

void Work(){
	pid_t pid = fork();
	if(pid > 0){
		exit(0);
	}
	//设置会长
	setsid();
	//改变工作目录
	chdir("/home/wjy/Safe_FTP_Station");
	//umask
	umask(0000);
	//重定向
	int fd = open("/dev/null", O_WRONLY);
	dup2(fd, STDIN_FILENO);
	//dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
}