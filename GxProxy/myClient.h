#ifndef myClient_h__
#define myClient_h__

/*
�ͻ�������������ȴ���֤
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

	//0 ǰ���32�ĳ��� 1����ӽ�����
	int pto_type;

public:

	std::string m_name;
	struct sockaddr_in peer;

	int64 m_tiBirth; //����������ʱ��
	int64 m_tiBreak; //���������ʱ�� ����������жϿͻ����Ƿ�����

//	char  recvBuffer[65535]; //��������������ʱ�򱣴���
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
		CTT_TLS,	//��������
		CTT_AUTH,	//�Ѿ�ͨ����֤��
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
	���Ӵ���һ��Ŀ��
	*/
	//void SetProxyTarget(sockaddr_in _addr);

	void Decrypto(char* buf, int size);
	void Encrypto(char* buf, int size);

public:
	virtual void OnTcpSend(CxTcpClient* sender, const char* buf, int size);
	virtual void OnTcpRecv(CxTcpClient* sender, const char* buf, int size);
	
	virtual int DoCmd(const char* buf, unsigned int size);

public:
	/**
	�����µ�ͨ�ż�/����Կ
	*/
	void SetCryptoKey(std::string _key);

protected:

	//�������� ͨ����֤ ��ʼ����
	int m_nState;
	bool m_bSSL;

	std::string m_sCryptoKey;


public:
	uv_tcp_t* handle;
	CxTcpClient* m_proxy;	//���ĸ���������
	sockaddr_in m_proxy_addr; //Ŀ�����ĵ�ַ

	int m_iPrivilege;  //Ȩ��



private:
	std::string m_sCryptoKeyOld;


};

#pragma mark - 
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

/**
����ͻ�������
*/
class CxMyClientPool : public Singleton<CxMyClientPool>
{
public:
	virtual ~CxMyClientPool();

	void Init(uint _client_num=1024,unsigned int _start=1);


public:
	//CxSafeVector<CxMyClient*>	clients; //�����޸�

	CxSafeMap<int64, CxMyClient*> fd_clients;

	void CheckClientOnline();

	void Step();

	CxMyClient* findClientByFD(int64 fd,bool _create);

	time_t m_last_check_time;

};


#endif // myClient_h__
