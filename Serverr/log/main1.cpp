#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <event2/listener.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include "Server.h"
#include "CodeC.h"
#include "CodeS.h"
#include "CodecFactory.h"
#include "CodesFactory.h"
#include "TcpSocket.h"

using namespace std;

const short PORT = 6666;

class PtrPack{
public:
evutil_socket_t p_sock;
struct sockaddr_in p_addr;
};

/*处理缓冲区事件的回调函数 读typedef void (*bufferevent_data_cb)
                                        (struct bufferevent *bev,   //bufferevent事件
                                         void *ctx )*/              //外部数据
void read_cback(struct bufferevent *bev, void *ctx){
	int ret = NOERR;
	int clientsendlen;
	char *clientsend = NULL;
	PtrPack *pack = (PtrPack *)ctx;
	TcpSocket socket(pack->p_sock);
	//接收客户端消息
	ret = socket.recvMsg(&clientsend, clientsendlen);
	if(ret != 0){
		if(ret == TcpSocket::TimeoutError){
			cout << "接收超时..." << endl;
			return;
		}
		cout << "接收数据失败,请查看日志" <<endl;
		return;
	}
	//解码
	BaseFactory *Factory = new CodecFactory;
	MyCode *code = (CodeC *)Factory->Create();
	AskMessage *ask = (AskMessage *)code->msgDecode(clientsend, clientsendlen);
	cout << ask->num << endl << ask->Clientid << endl << ask->Toserverid << endl 
	<< ask->AuthenticationCode <<endl << ask->Randnum_Client <<endl;
	//判断业务
	
	bufferevent_write(bev,"111",3);
	//释放内存
	delete pack;
	return;
}

/*处理缓冲区事件的回调函数 写*/
void write_cback(struct bufferevent *bev, void *ctx){
	printf("send successful...\n");
}

/*处理操作异常的回调函数 typedef void (*bufferevent_event_cb)
                                    (struct bufferevent *bev,
                                     short events, 
                                     void *ctx)*/
void err_cback(struct bufferevent *bev,short events, void *ctx){
	if(events & BEV_EVENT_EOF){
        printf("clinet disconnected...\n");		
	}
	else if(events & BEV_EVENT_ERROR){
		printf("something wrong...\n");
	}
	//if(events & )
	bufferevent_free(bev);
}

void commu_back(struct evconnlistener *listener,evutil_socket_t sock,
                struct sockaddr *addr,int len,void *ptr){
	//打印客户端ip
	char ip[64] = {0};
	struct sockaddr_in *ar = (struct sockaddr_in *)addr;
	inet_ntop(AF_INET, &ar->sin_addr.s_addr, ip, sizeof(ip));
	cout << "ip: " << ip << " connect successful..." << endl; 
	//接收base数据 便于将事件挂入
	struct event_base* base = (struct event_base*)ptr;  
	//初始化buffevent事件
	struct bufferevent* bufev = NULL;
	//创建缓冲区事件
	bufev = bufferevent_socket_new(base,sock,BEV_OPT_CLOSE_ON_FREE);
	//设置bufferevent事件处理函数
	PtrPack *pack = new PtrPack;
	memset(pack, 0x00, sizeof(PtrPack));
	pack->p_sock = sock;
	memcpy(&pack->p_addr, ar, sizeof(sockaddr_in));
	bufferevent_setcb(bufev,read_cback,write_cback,err_cback,pack); /**传数据***/
	//设置读事件 默认读是disable 写是enable
	bufferevent_enable(bufev,EV_READ);
}

int main(){
	srand((unsigned int)time(NULL));
	//创建eventbase
	struct event_base* base = event_base_new();
	
	//ip 端口信息
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
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
	
	return 0;
}