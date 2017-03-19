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

typedef struct {
	uv_tcp_t handle;
	uv_shutdown_t shutdown_req;
	CxMyClient* client;
} conn_rec;

typedef struct {
	uv_write_t req;
	uv_buf_t buf;
	CxMyClient* client;
} write_req_t;



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


public:
	CxTcpClient();

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

//	char  recvBuffer[65535]; //用来当数据来的时候保存用
private:
	
};


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
	void Reset();

	virtual int Open(sockaddr_in _addr);
	virtual void Close();

	void Accept();

	virtual int Send(const char* buf, int size);

	//void OnRecvFromClient(const char* buf, int size);

	/**
	连接代理到一个目标
	*/
	//void SetProxyTarget(sockaddr_in _addr);

	void Decrypto(char* buf, int size);
	void Encrypto(char* buf, int size);

public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size);
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size);
	
	virtual int DoCmd(const char* buf, unsigned int size);

public:
	static void connect_cb(uv_connect_t* req, int status);
	static void write_cb(uv_write_t* req, int status);
	static void read_cb(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
	static void shutdown_cb(uv_shutdown_t* req, int status);
	static void close_cb(uv_handle_t* handle);
	static void alloc_cb(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf);

protected:

	int m_nState;

	sockaddr_in m_addr;

	bool m_bSSL;
	std::string m_sCryptoKey;




	int m_iPrivilege;  //权限

public:
	uv_tcp_t* handle;
	CxTcpClient* m_proxy;	//和哪个代理链接
	
	//用来连接
	uv_tcp_t client;
	uv_os_sock_t sock;

};

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
	void Init(uint _client_num);


public:
	//CxSafeVector<CxMyClient*>	clients; //后续修改

	CxSafeMap<int64, CxMyClient*> fd_clients;

	void CheckClientOnline();

	void Step();

	CxMyClient* findClientByFD(int64 fd,bool _create);



};


#endif // myClient_h__
