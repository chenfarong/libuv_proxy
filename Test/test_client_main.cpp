

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <algorithm>

#include "../LibXs/XTcpClient.h"

using namespace std;

static int app_flag_stop = 0;


const char* XSTR_CMD_PROXY = "@!PROXY 127.0.0.1 7700\r\n\r\n";


class CxMyTcpClientDelegate : public XsTcpClientDelegate
{
public:
	virtual void OnConnectBefor(XsTcpClient* _sender, int _status) {};
	virtual void OnConnectAfter(XsTcpClient* _sender, int _status)
	{
		

		//TODO ��������Ŀ�� @!PROXY 127.0.0.1 7700
		_sender->Send(XSTR_CMD_PROXY, strlen(XSTR_CMD_PROXY));

		//TODO ��֪�Ѿ�����
	};

	virtual void OnDisconnect(XsTcpClient* _sender, int _status) {};

	virtual void OnSendAfter(XsTcpClient* _sender, const char* _buf, int _size) {};
	virtual void OnRecv(XsTcpClient* _sender, const char* _buf, int _size) {};

	/**
	�յ�һ���������ݰ�
	*/
	virtual void OnRecvPack(XsTcpClient* _sender, const char* _buf, int _size)
	{
		//��ӡ����Ļ��
		string str;
		str.append(_buf, _size);
		cout << str << endl;
	};
};


void work_tcp_step(XsTcpClient* tcp)
{
	while (app_flag_stop != 1)
	{
		//�ȴ����ӳɹ���ʼ
		tcp->Step();
	}

}

char RandomChar(char _left, char _right)
{
	int r = abs(_right - _left);
	int p = rand() % r;
	int l = min((int)_left, (int)_right);
	return (char)(l + p);
}

void SendStringRandom(XsTcpClient* _client)
{
	if (_client->IsConnected())
	{
		//��������ַ��� Ȼ����
		int l = rand()%255+1;
		string str;
		for (int i = 0; i < l; i++) str.push_back(RandomChar('a','z'));

		str.append("\r\n\r\n", 4);

		_client->Send(str.c_str(), str.length());
	}
}


int main(int argc, const char** argv)
{
	srand((unsigned int)time(NULL));

	CxMyTcpClientDelegate myTcpDelegate;
	XsTcpClient myClient;
	
	myClient.SetDelegate(&myTcpDelegate);

	myClient.Connect("127.0.0.1", 8000);

//	thread th(work_tcp_step,&myClient);

	while (app_flag_stop!=1)
	{

		myClient.Step();

		//TODO �����������
		SendStringRandom(&myClient);

		//TODO �������� ����ִ����

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

//	th.join();

	return 0;
}


