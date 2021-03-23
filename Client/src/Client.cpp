#include "Client.h"
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

using namespace std;

//初始化客户端 
Client::Client(){
	share = new DealShareM("/bin", 1, 100);
	share->ConnectShareM(0);
	/*   For Test
	NodeSHMInfo info;
	memset(&info, 0x00, sizeof(NodeSHMInfo));
	info.status = 1;
	strcpy(info.clientId, "0001");
	strcpy(info.serverId, "0002");
	info.seckeyid = 2;
	strcpy((char *)info.seckey, "11111111111");
	share->Insert(&info);*/
}
	
//功能窗口
void Client::ShowMenu(){
	cout << "------------------------------" <<endl;
	cout << "*         1.密钥协商         *" <<endl;
	cout << "*         2.密钥检验         *" <<endl;
	cout << "*         3.密钥注销         *" <<endl;
	cout << "*         4.密钥查看         *" <<endl;
	cout << "*         0.退    出         *" <<endl;
	cout << "------------------------------" <<endl;
}

//生成随机字符串
void Client::MakeRandString(char *buf, size_t size){
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
int Client::Secret_key_Negotiated(AskMessage *ask){
	int len , i, msgEncodelen, ret, serverRequestlen;
	char key[64] = {0};
	unsigned char md[SHA256_DIGEST_LENGTH] = {0};
	char *msgEncodeoutdata = NULL;
	char *serverRequest    = NULL;
	AnswerMessage *answer  = NULL;
	
	//for test
	strcpy(ask->Clientid,"0001");
	strcpy(ask->Toserverid,"0002");

	//生成随机字符串
	MakeRandString(ask->Randnum_Client, sizeof(ask->Randnum_Client));

	//生成验证散列值
	sprintf(key, "#%s+%s#", ask->Clientid, ask->Toserverid);
	HMAC_CTX * ctx = HMAC_CTX_new();
	HMAC_Init(ctx, key, strlen(key), EVP_sha256());
	HMAC_Update(ctx, (const unsigned char *)ask->Randnum_Client, strlen(ask->Randnum_Client));
	HMAC_Final(ctx, md, (unsigned int *)&len);
	
	//以十六进制写入发送结构体
	for(i = 0; i < SHA256_DIGEST_LENGTH; i++){
		sprintf(&ask->AuthenticationCode[2 * i], "%02x", md[i]);
	}
	
	cout << "randClient: " << endl << ask->Randnum_Client << endl;
	cout << "out: " << ask->AuthenticationCode << endl;
	HMAC_CTX_free(ctx);
	
	//编码
	BaseFactory* Factory = new CodecFactory(ask);
	MyCode* code = (CodeC *)Factory->Create();
	code->msgEncode(&msgEncodeoutdata, msgEncodelen);
	cout << "编码结果:" <<endl;
	cout << msgEncodeoutdata <<endl;
	
	//for test
    AskMessage *out = (AskMessage *)code->msgDecode(msgEncodeoutdata, msgEncodelen);
	cout << out->num <<endl<<out->Clientid<<endl<<out->Toserverid<<endl
	<<out->AuthenticationCode<<endl<<out->Randnum_Client<<endl; 
	
	//连接服务器
	TcpSocket socket;
	ret = socket.connectToHost("39.97.76.127", 6666);
	if(ret != 0){
		if(ret == TcpSocket::TimeoutError){
			cout << "连接超时..." << endl;
			delete msgEncodeoutdata;
			delete serverRequest; 
			delete Factory;
			delete code;
			socket.disConnect();
			return ret;
		}
		cout << "连接失败,请查看日志" <<endl;
		delete msgEncodeoutdata;
		delete serverRequest; 
		delete Factory;
		delete code;
		socket.disConnect();
		return ret;
	}
	
	//发送数据
	ret = socket.sendMsg(msgEncodeoutdata, strlen(msgEncodeoutdata));
	if(ret != 0){
		if(ret == TcpSocket::TimeoutError){
			cout << "连接超时..." << endl;
			delete msgEncodeoutdata;
			delete serverRequest; 
			delete Factory;
			delete code;
			socket.disConnect();
			return ret;
		}
		cout << "连接失败,请查看日志" <<endl;
		delete msgEncodeoutdata;
		delete serverRequest; 
		delete Factory;
		delete code;
		socket.disConnect();
		return ret;
	}
	
	//接收服务器反馈
	ret = socket.recvMsg(&serverRequest, serverRequestlen);
	if(ret != 0){
		if(ret == TcpSocket::TimeoutError){
			cout << "接收超时..." << endl;
			delete msgEncodeoutdata;
			delete serverRequest; 
			delete Factory;
			delete code;
			socket.disConnect();
			return ret;
		}
		cout << "接收数据失败,请查看日志" <<endl;
		delete msgEncodeoutdata;
		delete serverRequest; 
		delete Factory;
		delete code;
		socket.disConnect();
		return ret;
	}
	//断开连接
	socket.disConnect();
	//解码
	delete Factory;
	delete code;
	Factory = new CodesFactory();
	code = (CodeS *)Factory->Create();
	answer = (AnswerMessage *)code->msgDecode(serverRequest, serverRequestlen);
	
	//解读数据
	if(answer->num == CLIENT_ILLEGAL){//协商失败 客户端不合法
		cout << "协商失败 您非合法用户..." <<endl;
		ret = -1;
		delete msgEncodeoutdata;
		delete serverRequest; 
		delete Factory;
		delete code;
		return ret;
	}else if(answer->num == DATA_TAMPERED){//协商失败 数据被篡改
		cout << "协商失败 请再试一次..." <<endl;
		ret = -1;
		delete msgEncodeoutdata;
		delete serverRequest; 
		delete Factory;
		delete code;
		return ret;	
	}
	// if(answer->num != 1){
		// ret = -1;
		// cout << "数据错误" << endl; 
		// delete msgEncodeoutdata;
		// delete serverRequest; 
		// delete Factory;
		// delete code;
		// return ret;
	// }
	cout << answer->num << endl;
	//cout << "客户端随机字符: " << answer->Randnum_Server << endl;
	
	//生成秘钥
	char buf[256] = {0};
	unsigned char md1[SHA_DIGEST_LENGTH] = {0};
	sprintf(buf, "@%s+%s@", ask->Randnum_Client, answer->Randnum_Server);
	//unsigned char *SHA1(const unsigned char *d, size_t n, unsigned char *md);
	SHA1((unsigned char *)buf, strlen(buf), md1);
	
	//写入共享内存
	NodeSHMInfo info;
	memset(&info, 0x00, sizeof(info));
	info.status = 1;
	strcpy(info.clientId, ask->Clientid);
	strcpy(info.serverId, ask->Toserverid);
	info.seckeyid = answer->num;
	for(i = 0; i < SHA_DIGEST_LENGTH; i++){
		sprintf((char *)&info.seckey[2 * i], "%02x", md1[i]);
	}
	cout << "秘钥: " << info.seckey << endl;
	share->Insert(&info);
	
	cout << "协商成功..." << endl;

	//释放资源
	delete msgEncodeoutdata;
	delete serverRequest; 
	delete Factory;
	if(code != 0){
		delete code;
		code = NULL;
	}
	return NOERR;
}

//秘钥校验
int Client::Secret_key_Check(){
	return NOERR;
}

//秘钥注销
int Client::Secret_key_Cancel(){
	return NOERR;
}

//秘钥查看
int Client::Secret_key_Watch(){
	return NOERR;
}

//退出客户端
Client::~Client(){
	share->DisconnectShareM();
	if(share != NULL){
		delete share;
		
		share = NULL;
	}
	cout << "已退出..." << endl;
}
