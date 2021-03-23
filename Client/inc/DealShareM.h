#pragma once
#include "ShareM.h"

//将网点密钥信息写共享内存， 网点共享内存结构体
typedef struct _NodeSHMInfo
{	
	int 			status;			//密钥状态 0-有效 1无效
	char			clientId[12];	//客户端id
	char			serverId[12];	//服务器端id	
	int				seckeyid;		//对称密钥id
	unsigned char	seckey[128];	//对称密钥 //hash1 hash256 md5
}NodeSHMInfo;

class DealShareM:protected ShareM
{
public:
	enum ERRNUM{ParameterError = 5000, InserErr, FillErr, MallocErr};
	//创建共享内存 创建的存在 写日志 错误
	DealShareM(key_t key, int MaxNode);
	DealShareM(const char *path, int MaxNode, int num);
	
	//打开共享内存 
	DealShareM(key_t key);
	DealShareM(const char *path, int num);

	/*连接共享内存 0可读可写 SHM_RDONLY只读
	失败返回-1 成功返回0 需要打印日志判断结果
	*/
	int ConnectShareM(int level);
	
	//断开共享内存 成功返回0 失败返回非-1
	int DisconnectShareM();
	
	//写共享内存 写成功返回0 失败返回非0
	int Insert(NodeSHMInfo *node);
	
	//读共享内存
	void *Read_ShareM(const char *clientid, const char *serverid);
	
private:
	void *ptr;
};