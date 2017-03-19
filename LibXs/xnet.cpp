
#include "xnet.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <fcntl.h>



#if defined(__WINDOWS__)

#include <winioctl.h>

#pragma comment(lib, "ws2_32.lib")
#else

int WSAGetLastError()
{
	return -1;
}

#endif

int xnet_init()
{
#if defined(__WINDOWS__)
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("WSAStartup function failed with error: %d\n", iResult);
		return iResult;
	}
	return iResult;
#endif
	return 0;
}

void xnet_cleanup()
{
#if defined(__WINDOWS__)
	WSACleanup();
#endif
}


#if defined(__WINDOWS__)
xnet_sock xnet_connect(std::string ipAddr, int port, timeval _TimeOut, int af,  bool _noblock)
{
	xnet_sock s=INVALID_SOCKET;
	//----------------------
	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup function failed with error: %d\n", iResult);
		return s;
	}
	//----------------------
	// Create a SOCKET for connecting to server
	SOCKET ConnectSocket;
	ConnectSocket = socket(af, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
//		WSACleanup();
		return s;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	sockaddr_in clientService;
	clientService.sin_family = af;
	//clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
	iResult = inet_pton(af, ipAddr.c_str(), &clientService.sin_addr.s_addr);
	clientService.sin_port = htons(port);

	unsigned long ul = 1;
	ioctlsocket(ConnectSocket, FIONBIO, &ul); //设置为非阻塞模式

	//----------------------
	// Connect to server.

/*
	iResult = connect(ConnectSocket, (SOCKADDR *)& clientService, sizeof(clientService));

	if (iResult == SOCKET_ERROR) {
		printf("connect function failed with error: %ld %s \n", WSAGetLastError(),ipAddr.c_str());
		iResult = closesocket(ConnectSocket);
		if (iResult == SOCKET_ERROR)
			wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
//		WSACleanup();
		return s;
	}
*/
	if (connect(ConnectSocket, (struct sockaddr *)&clientService, sizeof(clientService)) == -1)
	{
		int error = -1;
		int len = sizeof(int);
		fd_set set;
		FD_ZERO(&set);
		FD_SET(ConnectSocket, &set);
		if (select((int)ConnectSocket, NULL, &set, NULL, &_TimeOut) > 0)
		{
			getsockopt(ConnectSocket, SOL_SOCKET, SO_ERROR, (char *)&error, /*(socklen_t *)*/&len);
			if (error != 0) return INVALID_SOCKET;
		}
		else return INVALID_SOCKET;
	}


	if (!_noblock) {
		ul = 0;
		ioctlsocket(ConnectSocket, FIONBIO, &ul); //设置为阻塞模式
	}

	//wprintf(L"Connected to server.\n");
	s = ConnectSocket;

	return s;
}

#else
xnet_sock xnet_connect(std::string ipAddr, int port, timeval _TimeOut, int af, bool _noblock)
{
	xnet_sock s = INVALID_SOCKET;
	//设置一个socket地址结构client_addr,代表客户机internet地址, 端口
	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr)); //把一段内存区的内容全部设置为0
	client_addr.sin_family = AF_INET;    //internet协议族
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
	client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
										//创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0)
	{
		printf("Create Socket Failed!\n");
		//		exit(1);
	}
	//把客户机的socket和客户机的socket地址结构联系起来
	if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))
	{
		printf("Client Bind Port Failed!\n");
		//		exit(1);
	}

	//设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if (inet_aton(ipAddr.c_str(), &server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
	{
		printf("Server IP Address Error!\n");
		//		exit(1);
	}
	server_addr.sin_port = htons(port);
	socklen_t server_addr_length = sizeof(server_addr);
	//向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
	
#if(1)
	if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		printf("Can Not Connect To %s : %d!\n", ipAddr.c_str(),port);
		//		exit(1);
		return 0;
	}
#endif

#if(0)
	xnet_noblock(client_socket, true);

	if (connect(client_socket, (struct sockaddr *)&server_addr, server_addr_length) < 0)
	{
		int error = -1;
		int len = sizeof(int);
		fd_set set;
		FD_ZERO(&set);
		FD_SET(client_socket, &set);
		if (select((int)client_socket, NULL, &set, NULL, &_TimeOut) > 0)
		{
//			getsockopt(client_socket, SOL_SOCKET, SO_ERROR, (char *)&error, /*(socklen_t *)*/&len);
			if (error != 0) return INVALID_SOCKET;
		}
		else return INVALID_SOCKET;
	}

	xnet_noblock(client_socket, _noblock);
#endif

	return client_socket;
}


#endif

void xnet_disconnect(xnet_sock s)
{
#ifdef __WINDOWS__
	if (INVALID_SOCKET!=s) closesocket(s);
#else
	if (INVALID_SOCKET != s) close(s);
#endif
}

int xnet_send(xnet_sock s, const char * lpBuf, int size)
{
	if (INVALID_SOCKET == s) return 0;
	return send(s, lpBuf, size, 0);
}

int xnet_request(xnet_sock s, std::string cmdStr, std::vector<char>& buf_cont,bool encode/*=true*/)
{
	int rc = 0;
	size_t k = 0;
	char buf;
	int  sp = -1;

	if (INVALID_SOCKET == s) return 0;

	//对字符串中含有 \r\n\r\n 进行转义
	while ((sp = (int)cmdStr.find(XNET_ENDL)) != -1)
	{
		cmdStr.replace(cmdStr.begin() + sp, cmdStr.begin() + sp + 4, XNET_ENDL_R);
	}

	cmdStr += XNET_ENDL;

	//发送消息到服务器上
	rc = send(s, cmdStr.c_str(), (int)cmdStr.length(), 0);

	//一直读到两个 \r\n\r\n 后结束接收
	do {
		rc = recv(s, &buf, 1, 0);

		if (rc < 1) {
			//网络被断开 读异常
			break;
		}

		buf_cont.push_back(buf);

		if (buf_cont.size() >= 4 && strstr(&buf_cont[0], XNET_ENDL) != 0) 
		{
			break;
		}
	} while (true);

	if (!encode) return 0;
	if (buf_cont.size() < 1) return 0;

	//如果中间有\r\n\r\n &#13&#10&#13&#10 替换为 \r\n\r\n
	
	while(1)
	{
		char* em = strstr(&buf_cont[0], XNET_ENDL_R);
		if (em != 0) {
			size_t p = em - (char*)(&buf_cont[0]);
			buf_cont.erase(buf_cont.begin() + p, buf_cont.begin() + p + 16);
			for (size_t i = 0; i < strlen(XNET_ENDL); i++) buf_cont.insert(buf_cont.begin() + p + i, XNET_ENDL[i]);
		}
		else break;
	}

	return 0;
}

#if defined(__WINDOWS__)
int xnet_noblock(xnet_sock s, bool v)
{
	long cmd = v?FIONBIO:FIOASYNC;

		unsigned long NonBlock = 1;
		if (ioctlsocket(s, cmd, &NonBlock) == SOCKET_ERROR)
		{
			printf("ioctlsocket failed with error %d\n", WSAGetLastError());
			return -1;
		}
		else {
			printf("ioctlsocket is OK!\n");
		}

	return 0;
}

bool xnet_is_noblock(xnet_sock s)
{
	int r = 0;
	int l = sizeof(int);
	getsockopt(s, SO_OOBINLINE,SO_OOBINLINE,(char*)&r, &l);
	return r;
}

#else
int xnet_noblock(xnet_sock sock, bool v)
{
	int opts;
	opts = fcntl(sock, F_GETFL);

	if (opts < 0) {
		perror("fcntl(sock, GETFL)");
//		exit(1);
		return -1;
	}

	if (v) opts = opts | O_NONBLOCK;
	else opts = opts &~O_NONBLOCK;

	if (fcntl(sock, F_SETFL, opts) < 0) {
		perror("fcntl(sock, SETFL, opts)");
//		exit(1);
		return -1;
	}

	printf("fcntl %d %s\n",sock,v?"noblock":"block");

	return 0;
}

bool xnet_is_noblock(xnet_sock s)
{
	int opts;
	opts = fcntl(s, F_GETFL);
	return opts & O_NONBLOCK;
}

#endif


int xnet_step(void* arg,xnet_sock s, xnet_recv_proc proc, char* recvBuf, int recvBufSize)
{
	int result = 0;
	FD_SET WriteSet;
	FD_SET ReadSet;
	FD_SET ExceptSet;
//	DWORD i;
	DWORD Total;
//	DWORD Flags;
//	DWORD SendBytes;
//	DWORD RecvBytes;
	timeval TimeOut = {0,200};


		// Prepare the Read and Write socket sets for network I/O notification
		FD_ZERO(&ReadSet);
		FD_ZERO(&WriteSet);
		FD_ZERO(&ExceptSet);

		// Always look for connection attempts
		FD_SET(s, &ReadSet);
		FD_SET(s, &WriteSet);
		FD_SET(s, &ExceptSet);

		// Set Read and Write notification for each socket based on the
		// current state the buffer.  If there is data remaining in the
		// buffer then set the Write set otherwise the Read set


		if ((Total = select((int)s+1, &ReadSet, &WriteSet, &ExceptSet, &TimeOut)) == SOCKET_ERROR)
		{
			printf("select() returned with error %d\n", WSAGetLastError());
			return -1;
		}
		//else
		//	printf("select() is OK!\n");

#if(0)
		printf("%s %d %d\n",__FILE__,s,(int)Total);
#endif
		// Check each socket for Read and Write notification until the number
		// of sockets in Total is satisfied

			// If the ReadSet is marked for this socket then this means data
			// is available to be read on the socket
			if (FD_ISSET(s, &ReadSet))
			{
//				printf("Data is available now.\n");
				result = recv(s, recvBuf, recvBufSize, 0);

				if(result<1)
				{
					//if (WSAGetLastError() != WSAEWOULDBLOCK)
					//{
					//	printf("WSARecv() failed with error %d\n", WSAGetLastError());
					//}
				}
				else
				{
					(proc)(arg,s, recvBuf, result);
				}
			}

			// If the WriteSet is marked on this socket then this means the internal
			// data buffers are available for more data
			if (FD_ISSET(s, &ExceptSet))
			{
				//TODO 这里发生异常 服务器断开
				return -1;
			}



	return result;
}


//int xnet_send_pre_u32()



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
#if defined _MSC_VER

#include <WinSock2.h>
#include <Iphlpapi.h>
//#include <WinSock.h>
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

/*
typedef struct tagIPInfo
{
char ip[30];
}IPInfo;

bool GetLocalIPs(IPInfo* ips, int maxCnt, int* cnt)
{
//1.初始化wsa
WSADATA wsaData;
int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
if (ret != 0)
{
return false;
}
//2.获取主机名
char hostname[256];
ret = gethostname(hostname, sizeof(hostname));
if (ret == SOCKET_ERROR)
{
return false;
}
//3.获取主机ip
HOSTENT* host = gethostbyname(hostname);
if (host == NULL)
{
return false;
}
//4.逐个转化为char*并拷贝返回
*cnt = host->h_length < maxCnt ? host->h_length : maxCnt;
for (int i = 0; i < *cnt; i++)
{
in_addr* addr = (in_addr*)*host->h_addr_list;
strcpy_s(ips[i].ip, inet_ntoa(addr[i]));
}
return true;
}
*/


using namespace std;

// 函数声明
void output(PIP_ADAPTER_INFO pIpAdapterInfo);

// 程序入口
void myIpAddrList(std::vector<std::string>& result)
{

	//1.初始化wsa
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		return;
	}

	//PIP_ADAPTER_INFO结构体指针存储本机网卡信息
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//得到结构体大小,用于GetAdaptersInfo参数
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//如果函数返回的是ERROR_BUFFER_OVERFLOW
		//则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
		//这也是说明为什么stSize既是一个输入量也是一个输出量
		//释放原来的内存空间
		delete pIpAdapterInfo;
		//重新申请内存空间用来存储所有网卡信息
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}
	if (ERROR_SUCCESS == nRel)
	{
		//输出网卡信息
		//output(pIpAdapterInfo);

		IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
		do
		{
			//cout << pIpAddrString->IpAddress.String << endl;
			result.push_back(pIpAddrString->IpAddress.String);

			pIpAddrString = pIpAddrString->Next;

		} while (pIpAddrString);
		//pIpAdapterInfo = pIpAdapterInfo->Next;

	}
	//释放内存空间
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}
	//	getchar();
	//	return 0;
}

///函数作用,输出网卡信息
void output(PIP_ADAPTER_INFO pIpAdapterInfo)
{
	//可能有多网卡,因此通过循环去判断
	while (pIpAdapterInfo)
	{
		cout << "网卡名称：" << pIpAdapterInfo->AdapterName << endl;
		cout << "网卡描述：" << pIpAdapterInfo->Description << endl;
		cout << "网卡MAC地址：" << pIpAdapterInfo->Address;
		for (UINT i = 0; i < pIpAdapterInfo->AddressLength; i++)
			if (i == pIpAdapterInfo->AddressLength - 1)
			{
				printf("%02x\n", pIpAdapterInfo->Address[i]);
			}
			else
			{
				printf("%02x-", pIpAdapterInfo->Address[i]);
			}
		cout << "网卡IP地址如下：" << endl;
		//可能网卡有多IP,因此通过循环去判断
		IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
		do
		{
			cout << pIpAddrString->IpAddress.String << endl;
			pIpAddrString = pIpAddrString->Next;
		} while (pIpAddrString);
		pIpAdapterInfo = pIpAdapterInfo->Next;
		cout << "*****************************************************" << endl;
	}
	return;
}


#else
void myIpAddrList(std::vector<std::string>& result)
{
	FILE *fstream = NULL;

	std::string cmd = "ifconfig | grep inet | awk '{print $2}'";

	// 通过管道来回去系统命令返回的值
	if (NULL == (fstream = popen(cmd.c_str(), "r"))) {
		return;
	}

	//	size_t _len = GetFileContentSize(fstream);
	char buf[4096];
	size_t _len = 4096;
	_len = fread(buf, _len, 1, fstream);
	pclose(fstream);  // close pipe

					  //	buf[_len]=0;

	std::stringstream iss;
	iss.write(buf, strlen(buf));

	printf("%s\n", buf);

	std::string ipAddr;
	while (iss.good())
	{
		getline(iss, ipAddr);
		result.push_back(ipAddr);
	}
}
#endif




//////////////////////////////////////////////////////////////////////////


static int xnet_pton4(const char *src, unsigned char *dst) {
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	unsigned char tmp[sizeof(struct in_addr)], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr(digits, ch)) != NULL) {
			unsigned int nw = *tp * 10 + (pch - digits);

			if (saw_digit && *tp == 0)
				return -1;
			if (nw > 255)
				return -1;
			*tp = nw;
			if (!saw_digit) {
				if (++octets > 4)
					return -1;
				saw_digit = 1;
			}
		}
		else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return -1;
			*++tp = 0;
			saw_digit = 0;
		}
		else
			return -1;
	}
	if (octets < 4)
		return -1;
	memcpy(dst, tmp, sizeof(struct in_addr));
	return 0;
}


static int xnet_pton6(const char *src, unsigned char *dst) {
	static const char xdigits_l[] = "0123456789abcdef",
		xdigits_u[] = "0123456789ABCDEF";
	unsigned char tmp[sizeof(struct in6_addr)], *tp, *endp, *colonp;
	const char *xdigits, *curtok;
	int ch, seen_xdigits;
	unsigned int val;

	memset((tp = tmp), '\0', sizeof tmp);
	endp = tp + sizeof tmp;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if (*src == ':')
		if (*++src != ':')
			return -1;
	curtok = src;
	seen_xdigits = 0;
	val = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			val <<= 4;
			val |= (pch - xdigits);
			if (++seen_xdigits > 4)
				return -1;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!seen_xdigits) {
				if (colonp)
					return -1;
				colonp = tp;
				continue;
			}
			else if (*src == '\0') {
				return -1;
			}
			if (tp + sizeof(uint16_t) > endp)
				return -1;
			*tp++ = (unsigned char)(val >> 8) & 0xff;
			*tp++ = (unsigned char)val & 0xff;
			seen_xdigits = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + sizeof(struct in_addr)) <= endp)) {
			int err = xnet_pton4(curtok, tp);
			if (err == 0) {
				tp += sizeof(struct in_addr);
				seen_xdigits = 0;
				break;  /*%< '\\0' was seen by inet_pton4(). */
			}
		}
		return -1;
	}
	if (seen_xdigits) {
		if (tp + sizeof(uint16_t) > endp)
			return -1;
		*tp++ = (unsigned char)(val >> 8) & 0xff;
		*tp++ = (unsigned char)val & 0xff;
	}
	if (colonp != NULL) {
		/*
		* Since some memmove()'s erroneously fail to handle
		* overlapping regions, we'll do the shift by hand.
		*/
		const int n = tp - colonp;
		int i;

		if (tp == endp)
			return -1;
		for (i = 1; i <= n; i++) {
			endp[-i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return -1;
	memcpy(dst, tmp, sizeof tmp);
	return 0;
}



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////


int xnet_pton(int af, const char* src, void* dst) {
	if (src == NULL || dst == NULL)
		return -1;

	switch (af) {
	case AF_INET:
		return (xnet_pton4(src, (unsigned char*)dst));
	case AF_INET6: {
		int len;
		char tmp[46], *s; const char *p;
		s = (char*)src;
		p = strchr(src, '%');
		if (p != NULL) {
			s = tmp;
			len = p - src;
			if (len > 46 - 1)
				return -1;
			memcpy(s, src, len);
			s[len] = '\0';
		}
		return xnet_pton6(s, (unsigned char*)dst);
	}
	default:
		return -2;//UV_EAFNOSUPPORT;
	}
	/* NOTREACHED */
}

int xnet_ip6_addr(const char* ip, int port, struct sockaddr_in6* addr) {
	char address_part[40];
	size_t address_part_size;
	const char* zone_index;

	memset(addr, 0, sizeof(*addr));
	addr->sin6_family = AF_INET6;
	addr->sin6_port = htons(port);

	zone_index = strchr(ip, '%');
	if (zone_index != NULL) {
		address_part_size = zone_index - ip;
		if (address_part_size >= sizeof(address_part))
			address_part_size = sizeof(address_part) - 1;

		memcpy(address_part, ip, address_part_size);
		address_part[address_part_size] = '\0';
		ip = address_part;

		zone_index++; /* skip '%' */
					  /* NOTE: unknown interface (id=0) is silently ignored */
#ifdef _WIN32
		addr->sin6_scope_id = atoi(zone_index);
#else
		addr->sin6_scope_id = if_nametoindex(zone_index);
#endif
	}

	return xnet_pton(AF_INET6, ip, &addr->sin6_addr);
}


int xnet_addr(const char* ip, int port, struct sockaddr_in* addr,int family)
{
//	assert(addr != NULL);

	memset(addr, 0, sizeof(*addr));
	addr->sin_family = family;
	addr->sin_port = htons(port);

	if (ip == NULL || strcmp("0", ip) == 0)
	{
#ifdef _WIN32		
		addr->sin_addr.s_addr = INADDR_ANY;
#else
		inet_aton("0", &(addr->sin_addr));
#endif
		return 0;
	}

	return xnet_pton(AF_INET, ip, &(addr->sin_addr.s_addr));
}
