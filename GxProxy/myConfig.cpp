
#include "myConfig.h"


template<>
CxMyConfig* Singleton<CxMyConfig>::si_instance = NULL;


CxMyConfig::CxMyConfig()
{
	SetAttrib("server", "ip", "0.0.0.0");
	SetAttrib("server", "port", "5454");
	SetAttrib("server", "ssl_enable", "0");
	SetAttrib("server", "ssl_port", "4545");
	SetAttrib("server", "proxy_type", "1"); //Ĭ��һ���ͻ���һ����������

	SetAttrib("client", "timeout", "30");			//�ͻ������30��û�з�����Ϣ�����������Ͽ��� �����0�Ͳ�������
	SetAttrib("client", "net_buf_size", "40960");	//�������ݻ�����
}


