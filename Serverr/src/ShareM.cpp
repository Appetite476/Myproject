#include "ShareM.h"
#include <iostream>
#include <sys/ipc.h>
#include <errno.h>

using namespace std;


//初始化 创建共享内存
ShareM::ShareM(key_t key, size_t size){
	int ret = NOERR;
	m_shmid = shmget(key, size, IPC_CREAT|IPC_EXCL|0700);
	
	if(m_shmid < 0){
		if(errno == EEXIST){
			m_shmid = shmget(key, 0, 0);
		}else{
				cout << "共享内存创建失败..." << endl;
				ret = ShareMGetErr;
				m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
				"func ShareM(key_t key, size_t size) err, ShareM Creat Err");
			}
	}else{
		cout << "共享内存创建成功..." << endl;
	}
}

//初始化 创建共享内存
ShareM::ShareM(const char *path, size_t size, int num){
	int ret = NOERR;
	key_t key= ftok(path, num);
	if(key < 0){
		int ret = -1;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func ShareM(char *path, size_t size, int num) err, ftok Err");
	}
	
	m_shmid = shmget(key, size, IPC_CREAT|IPC_EXCL|0700);
	if(m_shmid < 0){
		if(errno == EEXIST){
			m_shmid = shmget(key, 0, 0);
		}else{
			cout << "共享内存创建失败..." << endl;
			m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
			"func ShareM(char *path, size_t size, int num) err, ShareM Creat Err");
		}
	}else{
		cout << "共享内存创建成功..." << endl;
	}
}

//初始化共享内存 打开
ShareM::ShareM(key_t key){
	int ret = NOERR;
	m_shmid = shmget(key, 0, 0);
	if(m_shmid == -1){
		ret = ShareMGetErr;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func ShareM(key_t key) err, ShareM Get Err");
	}
}

ShareM::ShareM(const char *path, int num){
	int ret = NOERR;
	key_t key= ftok(path, num);
	if(key < 0){
		int ret = -1;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func ShareM(char *path, int num) err, ftok Err");
	}
	
	m_shmid = shmget(key, 0, 0);
	if(m_shmid == -1){
		ret = ShareMGetErr;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func ShareM(char *path, int num) err, ShareM Get Err");
	}
}

//连接共享内存
void *ShareM::Get_Connection(int level){
	int ret = NOERR;
	shmaddr = shmat(m_shmid, NULL, level);
	if(shmaddr == (void *)-1){
		ret = ShareMInErr;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func Get_Connection err, ShareM In Err");
	}
	return shmaddr;
}

//断开连接
int ShareM::Disconnect(void *addr){
	int ret = NOERR;
	ret = shmdt(addr);
	if(ret == -1){
		ret = ShareMDiscErr;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func Disconnect err, ShareM Disconnect Err");
	}
	return ret;
}

//删除共享内存
int ShareM::Remove_ShareM(){
	int ret = NOERR;
	ret = shmctl(m_shmid, IPC_RMID, NULL);
	if(-1 == ret){
		ret = ShareMDeleteErr;
		m_log.Log(__FILE__, __LINE__, ItcastLog::ERROR, ret,
		"func Remove_ShareM err, ShareM Delete Err");
	}
	return ret;
}