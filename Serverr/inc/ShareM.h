#pragma once
#include "ItcastLog.h"
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>

const int NOERR = 0;

class ShareM
{
public:
	enum ERRTYPE{ ShareMGetErr=4000, ShareMInErr, ShareMDiscErr, ShareMDeleteErr};
	
	//初始化 创建共享内存 
	ShareM(key_t key, size_t size);
	
	ShareM(const char *path, size_t size, int num);
	
	//连接共享内存 打开获取shmid
	ShareM(key_t key);
	
	ShareM(const char *path, int num);
	
	/*
	连接共享内存 //0可读可写 SHM_RDONLY只读
	失败返回(void *)-1 成功返回shmaddr
	*/
	void *Get_Connection(int level);
	
	//断开连接 失败返回-1 成功返回0
	int Disconnect(void *shmaddr);
	
	//删除共享内存 成功返回0 失败返回-1
	int Remove_ShareM();
private:
	int m_shmid;
	void *shmaddr = NULL;
public:
	ItcastLog m_log;
};