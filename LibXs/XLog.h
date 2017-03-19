#ifndef XLog_h__
#define XLog_h__


/*


//TODO


//FIXME
日志文件超出最大值的时候还没处理


//ChangeLog

*/


#include <mutex>
#include <string>
#include <time.h>
#include <deque>
#include <queue>
#include <thread>

enum LOG_LEVEL
{
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_FATAL,
};


enum Color
{
	BLACK,
	RED,
	GREEN,
	BROWN,
	BLUE,
	MAGENTA,
	CYAN,
	GREY,
	YELLOW,
	LRED,
	LGREEN,
	LBLUE,
	LMAGENTA,
	LCYAN,
	WHITE
};

const int Color_count = int(WHITE) + 1;


class CxLog
{
public:
	static CxLog* _instance;

	struct CxLogLine
	{
		std::string content;
		CxLogLine* next; //下一个
		std::mutex m_mutex;

		CxLogLine()
		{
			next = NULL;
		}

		CxLogLine(std::string text, CxLogLine* prev=NULL)
		{
			next = NULL;
			content = text;

			if (prev) {
				std::lock_guard<std::mutex> lck(prev->m_mutex);
				prev->next = this;
			}
		}

	};


protected:
	std::mutex  m_mutex;

//	std::string m_path;//保存在什么路径
	std::string m_prefix; //日志文件名前面部分 /log/xgame_20170103.log
//	std::string m_logfname; //日志文件名


	long ltime;

	int m_level; //日志级别

	bool m_bDaily;	//是否要加上每天的标志
	bool m_bFile; //是否要保存到文件中

	char szDT[128];
	//	struct tm *newtime;
	
	CxLogLine* m_tail;


	std::queue<std::string>	 logQueueA;
	std::queue<std::string>	 logQueueB;

	bool activeA;

	//写线程编号 如果不为0的时候 关闭的时候等待
	std::thread*  m_worker;

	bool m_bDestory; //是否在准备释放了

public:
	CxLog();
	virtual ~CxLog();


	static void writeToDisk(CxLog* _log);

	std::string GetLogFilename();
protected:
	void CheckLogFile(struct tm* _tm);

public:
	static CxLog* Instance();

	void printf(int _level, const char* fmt, ...);

	Color GetLevelColor(int _level);

	static FILE* OpenFile(const char* fname);

	/**
	开启或者关闭每日日志文件
	*/
	void SetEnableDaily(bool _enable=true, const char* path=NULL);

	/**
	打开是否保存到文件中
	*/
	void SetEnableToFile(bool _enable=false, const char* path=NULL, bool _daily = false);

	/**
	设置日志文件名前面部分 目录部分将直接创建
	*/
	bool SetFilenamePrefix(std::string path, bool _daily=false);

	void SetLevel(int _level);

public:

	static void SetColor(bool stdout_stream, int color);
	static void ResetColor(bool stdout_stream = true);

	static std::string getProcessID();
	static std::string getProcessName(bool val=false);
	static std::string getProcessDir();
	static std::string GetFileContent(const std::string filename);

	static void fixPath(std::string &path);
	static std::string getPath(std::string path);
	static bool isDirectory(std::string path);
	static bool createRecursionDir(std::string path);

	static inline int LocalTime(struct tm*  const _Tm, time_t const* const _Time);
};



#if(1)

#define  XLOG_TRACE(fmt,...)   CxLog::Instance()->printf(0,fmt " %s %d \n" ,##__VA_ARGS__,__FILE__,__LINE__)
#define  XLOG_DEBUG(fmt,...)   CxLog::Instance()->printf(1,fmt " %s %d \n" ,##__VA_ARGS__,__FILE__,__LINE__)
#define  XLOG_INFO(fmt,...)   CxLog::Instance()->printf(2,fmt "\n" ,##__VA_ARGS__)
#define  XLOG_WARN(fmt,...)   CxLog::Instance()->printf(3,fmt " %s %d \n" ,##__VA_ARGS__,__FILE__,__LINE__)
#define  XLOG_ERROR(fmt,...)   CxLog::Instance()->printf(4,fmt " %s %d \n" ,##__VA_ARGS__,__FILE__,__LINE__)
#define  XLOG_FATAL(fmt,...)   CxLog::Instance()->printf(5,fmt " %s %d \n" ,##__VA_ARGS__,__FILE__,__LINE__)

#define  XLOG_LINE(l,fmt,...)  CxLog::_instance->printf(l,fmt " %s %d \n" ,##__VA_ARGS__,__FILE__,__LINE__)

void xlog_println(int type, const char* txt);

#else

#include "log4z.h"
using namespace zsummer::log4z;


inline void XLOG_Init(int _level = 0, const char* _path = 0)
{
	//start log4z
	if (_path) ILog4zManager::getRef().setLoggerPath(0, _path);
	ILog4zManager::getRef().start();
	ILog4zManager::getRef().setLoggerLevel(0, _level);
}


#define  XLOG_TRACE  LOGFMTT
#define  XLOG_DEBUG  LOGFMTD
#define  XLOG_INFO   LOGFMTI
#define  XLOG_WARN   LOGFMTW
#define  XLOG_ERROR  LOGFMTE
#define  XLOG_ALARM  LOGFMTA
#define  XLOG_FATAL  LOGFMTF
#endif

void XLOG_STREAM(int level, const char * log);

/**
将日志设置到远程服务器上
*/
void XLOG_SET_REMOTE(const char* _ipAddr, int _port);


#endif // XLog_h__
