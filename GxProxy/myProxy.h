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
	0 Ĭ��ֵ ������һ��socket���ӵ�Ŀ�������
	1 ����һ��socket��Ŀ�������ͨ��
	*/
	int proxy_type;


};



#endif // myProxy_h__
