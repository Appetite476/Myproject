#include <iostream>
#include "DealShareM.h"
#include <string.h>
using namespace std;

//创建共享内存
DealShareM::DealShareM(key_t key, int MaxNode):
ShareM(key, sizeof(int) + sizeof(NodeSHMInfo)*MaxNode),
ptr(NULL){
	int tempnum;
	
	//连接
	ptr = Get_Connection(0);
	
	//检查是否存在
	memcpy(&tempnum, ptr, sizeof(int));
	
	if(tempnum != MaxNode){
		//初始化共享内存
		memset(ptr, 0x00, sizeof(int) + MaxNode*sizeof(NodeSHMInfo));
	
		//写MaxNode
		memcpy(ptr, (void *)&MaxNode, sizeof(int));
	}
	//断开连接
	Disconnect(ptr);
	
	ptr = NULL;
}

DealShareM::DealShareM(const char *path, int MaxNode, int num):
ShareM(path, sizeof(int) + sizeof(NodeSHMInfo)*MaxNode, num),
ptr(NULL){
	int tempnum;
	
	//连接
	ptr = Get_Connection(0);
	
	//检查是否存在
	memcpy(&tempnum, ptr, sizeof(int));
	
	if(tempnum != MaxNode){
		//初始化共享内存
		memset(ptr, 0x00, sizeof(int) + MaxNode*sizeof(NodeSHMInfo));
	
		//写MaxNode
		memcpy(ptr, (void *)&MaxNode, sizeof(int));
	}
	//断开连接
	Disconnect(ptr);
	
	ptr = NULL;
}

//打开共享内存
DealShareM::DealShareM(key_t key):
ShareM(key),
ptr(NULL){}

DealShareM::DealShareM(const char *path, int num):
ShareM(path, num),
ptr(NULL){}

//连接共享内存
int DealShareM::ConnectShareM(int level){
	ptr = Get_Connection(level);
	if(ptr == (void*)-1){
		return -1;
	}
	
	cout<< "已连接共享内存..." <<endl; 
	return NOERR;
}

//断开共享内存 成功返回0 失败返回非0
int DealShareM::DisconnectShareM(){
	int ret = NOERR;
	if(ptr == NULL){
		return ret;
	}
	
	ret = Disconnect(ptr);
	ptr = NULL;
	return ret;
}

//写共享内存  //存在替代 不存在写
int DealShareM::Insert(NodeSHMInfo *node){
	int ret 	= NOERR;
	int i 		= 0;
	int tempnum = 0;
	if(node == NULL){
		ret = ParameterError;
		//打印日志
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func Insert err, Check (NodeSHMInfo *node)");
		return ret;
	}
	
	if(ptr == NULL){
		ret = InserErr;
		//打印日志
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func Insert err, Check ptr: NULL");
		return ret;
	}
	
	memcpy(&tempnum, ptr, sizeof(int));
	void * tmpptr = (char *)ptr + sizeof(int);
	NodeSHMInfo *temp = (NodeSHMInfo *)tmpptr;

	NodeSHMInfo empty;
	memset(&empty, 0x00, sizeof(NodeSHMInfo));
	for(i; i < tempnum; i++){
		if(strcmp(temp->clientId, node->clientId) == 0
		&& strcmp(temp->serverId, node->serverId) == 0){
			memset(temp, 0x00, sizeof(NodeSHMInfo));
			memcpy(temp, node, sizeof(NodeSHMInfo));
			return NOERR;
		}
		
		if(memcmp(temp, &empty, sizeof(NodeSHMInfo)) == 0){
			memcpy(temp, node, sizeof(NodeSHMInfo));
			return NOERR;
		}
		temp++;
	}
	

	//打印日志 共享向存 满了
	ret = FillErr;
	m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
	"func Insert err, ShareM Filled");
	return ret;
}

//读共享内存
void *DealShareM::Read_ShareM(const char *clientid, const char *serverid){
	int ret 	= NOERR;
	int i 		= 0;
	int tempnum = 0;
	
	if(ptr == NULL){
		ret = InserErr;
		//打印日志
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func Read_ShareM err, Check ptr: NULL");
		return NULL;
	}
	
	memcpy((void *)&tempnum, ptr, sizeof(int));
	void * tmpptr = (char *)ptr + sizeof(int);
	NodeSHMInfo *temp = (NodeSHMInfo *)tmpptr;
	
	for(i; i < tempnum; i++){
		if((strcmp(temp->clientId, clientid) == 0)
		&& (strcmp(temp->serverId, serverid) == 0)){
			NodeSHMInfo *out = new NodeSHMInfo;
			if(out == NULL){
				ret = MallocErr;
				m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
				"func Read_ShareM err, New Node Err");
				return NULL;
			}
			memset(out, 0x00, sizeof(NodeSHMInfo));
			memcpy(out, temp, sizeof(NodeSHMInfo));
			return (void *)out;
		}
		temp++;
	}
	
	//无
	cout << "未查询到结果" <<endl;
	return NULL;
}