#ifndef myClient_h__
#define myClient_h__

/*
客户端连接上来后等待验证
*/

#include "Singleton.h"
#include "LibXs.h"
#include "uv.h"
#include "SafeContainer.h"
#include "XNetBuffer.h"

class CxTcpClient;
class CxMyClient;



class CxTcpDelegate
{
public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size) {  };
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size) {  };
	virtual void OnTcpClose(CxTcpClient* sender) {};
	virtual void OnTcpOpen(CxTcpClient* sender) {};
};

class CxTcpClient
{
public:
	CxTcpClient();
	virtual ~CxTcpClient();

	virtual int Open(sockaddr_in _addr);
	virtual void Close();

	virtual int Send(const char* buf, int size);
	int SendPto(const char* buf, int size);

	virtual int Recv(const char* buf, int size);

	bool IsOnline();
	virtual void SetFD(int64 fd);

	virtual int DoCmd(const char* buf, unsigned int size) { return 0; };

	void SetDelegate(CxTcpDelegate* _delegate);

protected:
	CxTcpDelegate* m_delegate;
	int64 m_socket;
	CxNetBuffer m_input;

	//0 前面加32的长度 1后面加结束符
	int pto_type;



public:

	std::string m_name;
	struct sockaddr_in peer;

	int64 m_tiBirth; //连接上来的时间
	int64 m_tiBreak; //心跳的最后时间 根据这个来判断客户端是否还在线
	
	char m_recv_buf[40960]; //用来当数据来的时候保存用

private:
	
};

#pragma mark - 
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////


class CxMyClient : public CxTcpClient,public CxTcpDelegate
{
public:
	enum CLI_STATE
	{
		CTT_NEW,
		CTT_TLS,	//加密握手
		CTT_AUTH,	//已经通过验证了
	};

public:
	CxMyClient();
	virtual ~CxMyClient();

	void Reset();

	virtual int Open(sockaddr_in _addr);
	virtual void Close();

	void Accept();

	virtual int Send(const char* buf, int size);
	virtual void SetFD(int64 fd);


	/**
	连接代理到一个目标
	*/
	//void SetProxyTarget(sockaddr_in _addr);

	void Decrypto(char* buf, int size);
	void Encrypto(char* buf, int size);
public:
//	static void shutdown_cb(uv_shutdown_t* req, int status);
//	static void close_cb(uv_handle_t* handle);
	static void write_cb(uv_write_t* req, int status);

public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size);
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size);
	
	virtual int DoCmd(const char* buf, unsigned int size);

public:
	/**
	设置新的通信加/解密钥
	*/
	void SetCryptoKey(std::string _key);

protected:

	//加密握手 通过验证 开始正常
	int m_nState;
	bool m_bSSL;

	std::string m_sCryptoKey;


public:
	uv_tcp_t*  handle;
	CxTcpClient* m_proxy;	//和哪个代理链接
	sockaddr_in m_proxy_addr; //目标服务的地址

	int m_iPrivilege;  //权限
//	conn_rec* conn;

//	char m_input_buf[40960];

private:
	std::string m_sCryptoKeyOld;


};


typedef struct {
	uv_tcp_t handle;
	uv_shutdown_t shutdown_req;
	CxMyClient client;
} conn_rec;

typedef struct {
	uv_write_t req;
	uv_buf_t buf;
	CxMyClient* client;
} write_req_t;



#pragma mark - 
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

/**
管理客户端连接
*/
class CxMyClientPool : public Singleton<CxMyClientPool>
{
public:
	virtual ~CxMyClientPool();

	void Init(uint _client_num=1024,unsigned int _start=1);


public:

	CxSafeVector<conn_rec*>	clients; //重用

	conn_rec* NewConn();
	void DeleteConn(conn_rec*);

	CxSafeMap<int64, CxMyClient*> fd_clients;

	void CheckClientOnline();

	void Step();

	CxMyClient* findClientByFD(int64 fd,bool _create);
	void DisconnectAll();


	time_t m_last_check_time;

};


#endif // myClient_h__
