#ifndef myProxy_h__
#define myProxy_h__

#include "Singleton.h"

class CxMyProxy : public Singleton<CxMyProxy>
{
public:
	CxMyProxy();

public:
	void SendToClient(int fd, const char* buf, int size);
	void SendToTarget(int fd, const char* buf, int size);


	void OnRecvFromTarget(int fd, const char* buf, int size);
	void OnRecvFromClient(int fd, const char* buf, int size);


protected:
	/**
	0 默认值 单独的一个socket连接到目标服务器
	1 共享一个socket与目标服务器通信
	*/
	int proxy_type;


};



#endif // myProxy_h__
