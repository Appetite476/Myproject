#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <event2/listener.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include "Server.h"
#include "CodeC.h"
#include "CodeS.h"
#include "CodecFactory.h"
#include "CodesFactory.h"
#include "TcpSocket.h"

using namespace std;

Server *s_server = NULL;
static bool is_go;

class PtrPack{
public:
struct event *p_ev = NULL;
struct sockaddr_in p_addr;
};

void Work();

void Catch(int num){
	is_go = false;
	return;
}

void Catch2(int num, siginfo_t *info, void *something){
	struct event_base* base = (struct event_base*)info->si_ptr;
	//event_base_loopbreak(base);
	event_base_loopexit(base, NULL);
	return;
}

/*处理缓冲区事件的回调函数 读typedef void (*bufferevent_data_cb)
                                        (struct bufferevent *bev,   //bufferevent事件
                                         void *ctx )*/              //外部数据
void client_cb(evutil_socket_t sock, short w, void * ctx){
	int ret = NOERR;
	int clientsendlen;
	char *clientsend = NULL;
	PtrPack *pack = (PtrPack *)ctx;
	TcpSocket socket(sock);
	char ip[64] = {0};
	struct sockaddr_in ar = pack->p_addr;
	inet_ntop(AF_INET, &ar.sin_addr.s_addr, ip, sizeof(ip));
	
	if(w & EV_CLOSED){
		cout << ip << " disconnected..." << endl;
		event_del(pack->p_ev);
		event_free(pack->p_ev);
		evutil_closesocket(sock);
		if(pack != NULL){
			delete pack;
			pack == NULL;
		}
		return;
	}
	
	//接收客户端消息
	ret = socket.recvMsg(&clientsend, clientsendlen);
	if(ret != 0){
		if(ret == TcpSocket::TimeoutError){
			cout << "接收超时..." << endl;
			event_del(pack->p_ev);
			event_free(pack->p_ev);
			evutil_closesocket(sock);
			if(pack != NULL){
				delete pack;
				pack == NULL;
			}
			if(clientsend != NULL){
				delete clientsend;
				clientsend = NULL;
			}
			return;
		}
		cout << "接收数据失败,请查看日志" <<endl;
		event_del(pack->p_ev);
		event_free(pack->p_ev);
		evutil_closesocket(sock);
		if(pack != NULL){
			delete pack;
			pack == NULL;
		}
		if(clientsend != NULL){
			delete clientsend;
			clientsend = NULL;
		}
		return;
	}
	//解码
	BaseFactory *Factory = new CodecFactory;
	MyCode *code = (CodeC *)Factory->Create();
	AskMessage *ask = (AskMessage *)code->msgDecode(clientsend, clientsendlen);
	
	AnswerMessage answer;
	memset(&answer, 0x00, sizeof(answer));
	strcpy(answer.Toclientid, ask->Clientid);
	strcpy(answer.serverid, ask->Toserverid);
	//判断业务
	switch(ask->num){
	case CodeC::NewOrUpdate:
		s_server->Secret_key_Negotiated(&answer, &socket, ask);
		break;
	case CodeC::Check:
		s_server->Secret_key_Check();
		break;
	case CodeC::Revoke:
		s_server->Secret_key_Cancel();
		break;
	case CodeC::View:
		s_server->Secret_key_Watch();
		break;
	default:
		break;
	}
	
	if(Factory != NULL){
		delete Factory;
		Factory == NULL;
	}
	if(code != NULL){
		delete code;
		code == NULL;
	}
	if(clientsend != NULL){
		delete clientsend;
		clientsend = NULL;
	}
	return;
}

void commu_back(struct evconnlistener *listener,evutil_socket_t sock,
                struct sockaddr *addr,int len,void *ptr){
	//接收base数据 便于将事件挂入
	struct event_base* base = (struct event_base*)ptr; 
	
	//信号中断
	if(!is_go){
		union sigval mysigval;
		mysigval.sival_ptr = base;
		pid_t pid = getpid();
		sigqueue(pid, SIGUSR2, mysigval);
	}
	
	//打印客户端ip
	char ip[64] = {0};
	struct sockaddr_in *ar = (struct sockaddr_in *)addr;
	inet_ntop(AF_INET, &ar->sin_addr.s_addr, ip, sizeof(ip));
	cout << "ip: " << ip << " connect successful..." << endl;
	//初始化event事件 struct event * ev = NULL;
	struct event *ev;
	ev = new struct event;
	if (ev == NULL){
		//写日志
		cout << "mm_malloc err..." << endl;
		return;
	}
	//创建事件
	PtrPack *pack = new PtrPack;
	memset(&pack->p_addr, 0x00, sizeof(sockaddr_in));
	memcpy(&pack->p_addr, ar, sizeof(sockaddr_in));
	pack->p_ev = ev;
	if (event_assign(ev, base, sock, EV_READ|EV_PERSIST|EV_CLOSED, client_cb, pack) < 0)
	{
		//写日志
		delete ev;
		return;
	}
	event_add(ev, NULL);
}

int main(){
	//守护进程
	//Work();
	is_go = true;
	srand((unsigned int)time(NULL));
	
	//初始化Server
	ServerInfo info;
	memset(&info, 0x00, sizeof(info));
	strncpy(info.dbUse, getenv("DBUSER"), strlen(getenv("DBUSER")));
	strncpy(info.dbPasswd, getenv("DBPW"), strlen(getenv("DBPW")));
	strncpy(info.dbIP, getenv("DBIP"), strlen(getenv("DBIP")));
	strncpy(info.dbSID, getenv("DBNM"), strlen(getenv("DBNM")));
	info.maxnode = 20;
	info.sPort = 6666;
	info.shmkey = ftok("/bin", 100);
	strcpy(info.serverID, "0001");
	s_server = new Server(&info);
	
	//设置信号1
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = Catch;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1, &act, NULL);
	
	//设置信号2
	struct sigaction act2;
	act2.sa_flags = SA_SIGINFO;
	act2.sa_sigaction = Catch2;
	sigemptyset(&act2.sa_mask);
	sigaction(SIGUSR2, &act2, NULL);
	
	//创建eventbase
	struct event_base* base = event_base_new();
	
	//ip 端口信息
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(info.sPort);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/*创建套接字
	绑订
	监听
	等待并且接收请求*/
	/*struct evconnlistener* evconnlistener_new_bind(
	            struct event_base *base,        //根节点
                evconnlistener_cb cb,           //接收客户端后通讯回调函数
                void *ptr,                      //传给回调的数据
                unsigned flags,                 //LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE
                int backlog,                    //最多同时连接
                const struct sockaddr *sa,      
                int socklen)*/
	struct evconnlistener* listener = evconnlistener_new_bind(base,commu_back,base
	                                               ,LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE
												   ,-1,(struct sockaddr*)&server,sizeof(server));

	event_base_dispatch(base);                  //循环监听事件
	      
	event_base_free(base);                      //释放根节点
	
	delete s_server;
	return 0;
}