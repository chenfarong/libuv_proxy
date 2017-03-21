

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
		

		//TODO 发送连接目标 @!PROXY 127.0.0.1 7700
		_sender->Send(XSTR_CMD_PROXY, strlen(XSTR_CMD_PROXY));

		//TODO 告知已经连接
	};

	virtual void OnDisconnect(XsTcpClient* _sender, int _status) {};

	virtual void OnSendAfter(XsTcpClient* _sender, const char* _buf, int _size) {};
	virtual void OnRecv(XsTcpClient* _sender, const char* _buf, int _size) {};

	/**
	收到一个完整数据包
	*/
	virtual void OnRecvPack(XsTcpClient* _sender, const char* _buf, int _size)
	{
		//打印到屏幕上
		string str;
		str.append(_buf, _size);
		cout << str << endl;
	};
};


void work_tcp_step(XsTcpClient* tcp)
{
	while (app_flag_stop != 1)
	{
		//等待连接成功后开始
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
		//随机生成字符串 然后发送
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

		//TODO 随机发送内容
		SendStringRandom(&myClient);

		//TODO 接收命令 并且执行它

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

//	th.join();

	return 0;
}


