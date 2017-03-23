
/*

写到文件中的时候用另外一个线程来执行

*/



#include "XLog.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stdarg.h>
#include <assert.h>
#include <thread>
#include <mutex>
#include <assert.h>
#include <ctime>
#include <time.h>
#include <string.h>
#include <fstream>

#ifndef PLATFORM

#if _MSC_VER
#define PLATFORM 0

#include <windows.h>
#include <io.h>
#include <shlwapi.h>
#include <process.h>
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "User32.lib")
#pragma warning(disable:4996)

#else
#define PLATFORM 1

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>

#endif

#endif

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#include <libproc.h>
#endif


typedef unsigned char uint8;
typedef unsigned short WORD;

namespace Xs
{
#ifndef _MSC_VER


#include <sys/time.h>

	unsigned long Now()
	{
		struct timeval now;
		//获得当前精确时间(1970年1月1日到现在的时间)，
		//执行时间（微秒） = 1000000 * (tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec;
		gettimeofday(&now, NULL);
		return 1000 * now.tv_sec + now.tv_usec / 1000;
	}

#else

#include <windows.h>


	//该函数从0开始计时，返回自设备启动后的毫秒数（不含系统暂停时间）。
	unsigned long Now()
	{
		return  ::GetTickCount();
	}

	int gettimeofday(struct timeval * val, struct timezone *)
	{
		if (val)
		{
			LARGE_INTEGER liTime, liFreq;
			QueryPerformanceFrequency(&liFreq);
			QueryPerformanceCounter(&liTime);
			val->tv_sec = (long)(liTime.QuadPart / liFreq.QuadPart);
			val->tv_usec = (long)(liTime.QuadPart * 1000000.0 / liFreq.QuadPart - val->tv_sec * 1000000.0);
		}
		return 0;
	}

#endif

};


void worker_write_to_disk(CxLog* _log)
{

}

CxLog* CxLog::_instance = NULL;

void xlog_println(int type, const char* txt)
{
	CxLog::_instance->printf(type,"%s\n", txt);
}

//////////////////////////////////////////////////////////////////////////

std::string CxLog::getProcessID()
{
	std::string pid = "0";
	char buf[260] = { 0 };
#ifdef WIN32
	DWORD winPID = GetCurrentProcessId();
	_snprintf_s(buf, 255, 255, "%06u", winPID);
	pid = buf;
#else
	snprintf(buf, sizeof(buf), "%06d", getpid());
	pid = buf;
#endif
	return pid;
}

std::string CxLog::GetFileContent(const std::string filename)
{
	std::string str;
	std::ifstream inf(filename.c_str());
	if (!inf.good()) return str;

	inf.seekg(0, std::ifstream::end);
	size_t l = inf.tellg();
	inf.seekg(0, std::ifstream::beg);
	str.resize(l);
	inf.read(&str[0], l);
	inf.close();
	return str;
}

std::string CxLog::getProcessName(bool val)
{
	std::string name;
	char buf[260] = { 0 };
#ifdef WIN32
	if (GetModuleFileNameA(NULL, buf, 259) > 0)
	{
		name = buf;
		if (val) return name;
	}
	std::string::size_type pos = name.rfind("\\");
	if (pos != std::string::npos)
	{
		name = name.substr(pos + 1, std::string::npos);
	}
	pos = name.rfind(".");
	if (pos != std::string::npos)
	{
		name = name.substr(0, pos - 0);
	}

#elif defined(__APPLE__)

	proc_name(getpid(), buf, 260);
	name = buf;
	return name;
#else

#if defined(__WINDOWS__)
	_snprintf_s(buf, sizeof(buf), 255, "/proc/%d/cmdline", (int)getpid());
#else
	snprintf(buf, sizeof(buf), "/proc/%d/cmdline", (int)getpid());
#endif


	if (val) {
		name = GetFileContent(buf);
		//const char** arg=&name.c_str();
		//printf("%s\n",arg[0]);
		//const char** arg=&(name.c_str());
		//printf("%s\n",arg[0]);
		//这里取第一个参数然后
		//return name;
	}


	std::string::size_type pos = name.rfind("/");
	if (pos != std::string::npos)
	{
		name = name.substr(pos + 1, std::string::npos);
	}
#endif

	return name;
}



std::string CxLog::getProcessDir()
{
	std::string str = getProcessName(true);
	return getPath(str);
}

int CxLog::LocalTime(struct tm* const _Tm, time_t const* const _Time)
{
#if defined(_MSC_VER)
	return localtime_s(_Tm, _Time);
#else //linux
	localtime_r(_Time, _Tm);
	return 0;
#endif
}

CxLog::CxLog()
{
	m_level = -1;
	m_bDaily = true;
	m_prefix = CxLog::getProcessName();
	m_worker = NULL;
	m_bDestory = false;
	m_bFile = false;
	m_tail = NULL;
}

CxLog::~CxLog()
{
	ResetColor();

	m_bDestory = true;
	//等待写线程完成
	if (m_worker) {
		m_worker->join();
	}

}

#if(1)
void CxLog::writeToDisk(CxLog* _log)
{
	CxLogLine* _m_tail=NULL;
	CxLogLine* _m_tail_tmp = NULL;
	while (!_log->m_bDestory)
	{
		//休眠1秒
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		if(_log->m_tail==NULL) continue;

		_log->m_mutex.lock();
		_m_tail = _log->m_tail;
		_log->m_tail = NULL;
		_log->m_mutex.unlock();

		//打开文件
		FILE* _myFile = CxLog::OpenFile(_log->GetLogFilename().c_str());
		if (_myFile)
		{
			while (_m_tail)
			{
				fwrite(_m_tail->content.c_str(), _m_tail->content.length(), 1, _myFile);
				//fwrite("\r\n", 2, 1, _myFile);
				_m_tail_tmp = _m_tail;
				_m_tail = _m_tail->next;
				delete _m_tail_tmp;
			}
			fclose(_myFile);
		}


	}
}
#else
void CxLog::writeToDisk(CxLog* _log)
{
	while (!_log->m_bDestory)
	{
		std::queue<std::string>* _active =
			_log->activeA ? &_log->logQueueA :
			&_log->logQueueB;

		_log->m_mutex.lock();
		_log->activeA = !_log->activeA;
		_log->m_mutex.unlock();

		//打开文件
		FILE* _myFile = CxLog::OpenFile(_log->GetLogFilename().c_str());
		if (_myFile)
		{
			while (!_active->empty()) {
				std::string s = _active->front();
				_active->pop();
				fwrite(s.c_str(), s.length(), 1, _myFile);
				//fwrite("\r\n", 2, 1, _myFile);
			}
			fclose(_myFile);
		}

		//休眠1秒
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}
#endif

std::string CxLog::GetLogFilename()
{
	if (m_prefix.empty()) return getProcessName() + ".log";

	if (m_bDaily) 
	{
		char buf[16];
		time_t _now = time(NULL);
		struct tm tt = { 0 };
		CxLog::LocalTime(&tt, &_now);
		strftime(buf, 16, "%Y-%m-%d", &tt);
		return m_prefix + std::string(buf) + ".log";
	}

	return m_prefix + ".log";
}

void CxLog::CheckLogFile(struct tm* _tm)
{
	return;
	//!!! FIXME

//	strftime(szDT, 128,"%Y-%m-%d", _tm);
//	if (m_logfname.find(szDT) == std::string::npos || m_f==NULL)
//	{
//		m_logfname = m_prefix + std::string(szDT) + ".log";
//		if (m_f) fclose(m_f);
//#ifdef _MSC_VER
//		fopen_s(&m_f, m_logfname.c_str(), "a+b");
//#else
//		m_f = fopen(m_logfname.c_str(), "a+b");
//	
//#endif
//		assert(m_f);
//	}
}

CxLog* CxLog::Instance()
{
	if (_instance == NULL) {
		_instance = new CxLog();
	}
	return _instance;
}


uint16_t NowMilliseconds()
{
#if defined(_MSC_VER)
	SYSTEMTIME st;
	::GetLocalTime(&st);
	return st.wMilliseconds;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint16_t)(tv.tv_usec / 1000);
#endif
}

void CxLog::printf(int _level, const char* fmt, ...)
{
	char buf[4096];
	std::string str;
	va_list ap;

	if (_level < m_level) return; //不记录
	

	//避免多线程冲突问题
	std::lock_guard<std::mutex> lck(m_mutex);



	//根据级别打印不同的颜色
	SetColor(true, GetLevelColor(_level));

	time_t _now = time(NULL);
	struct tm tt = { 0 };

#if defined(_MSC_VER)
	localtime_s(&tt, &_now);

//	struct timeval tv;

	strftime(buf, 128,
		"%Y-%m-%d %H:%M:%S ", &tt);

	sprintf(szDT, "[%s] %s :%0.4d ",getProcessID().c_str(),buf, NowMilliseconds());
//	strcat(szDT, buf);

#else //linux
	localtime_r(&_now, &tt);
	//struct timeval tv;
	//gettimeofday(&tv, NULL);
	//sprintf(buf, ":%.4d ", (int)(tv.tv_usec/1000));
	strftime(buf, 128,"%Y-%m-%d %H:%M:%S", &tt);
//	strcat(szDT, buf);
	sprintf(szDT, "[%s] %s :%0.4d ", getProcessID().c_str(), buf, NowMilliseconds());
#endif



	
	//fprintf(stdout, "%s", szDT);
	//va_start(ap, fmt);
	//vfprintf(stdout, fmt, ap);
	//va_end(ap);
	//fflush(stdout);

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	str = std::string(szDT) + std::string(buf)/*+"\n"*/;
	std::cout << str;
	fflush(stdout);

	//保存到准备写的队列中
	if (m_bFile) 
	{
#if(1)
		if (m_tail == NULL)
		{
			m_tail = new CxLogLine(str, NULL);
		}
		else {
			new CxLogLine(str, m_tail);
		}
#else
		//这里如果队列太长了 就需要
		std::queue<std::string>* _myQueue =
			activeA ? &logQueueA : &logQueueB;
		_myQueue->push(str);
#endif
	}

	if (_level == LOG_FATAL)
	{
#ifdef _DEBUG
		exit(-1);
#endif
	}

	ResetColor();
}

/*
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
*/

Color CxLog::GetLevelColor(int _level)
{
	switch (_level)
	{
	case LOG_LEVEL::LOG_TRACE:
		return Color::LCYAN;
	case LOG_LEVEL::LOG_DEBUG:
		return Color::WHITE;
	case LOG_LEVEL::LOG_INFO:
		return Color::GREY;
	case LOG_LEVEL::LOG_WARNING:
		return Color::YELLOW;
	case LOG_LEVEL::LOG_ERROR:
		return Color::MAGENTA;
	case LOG_LEVEL::LOG_FATAL:
		return Color::RED;
	default:
		return Color::WHITE;
	}
}

FILE* CxLog::OpenFile(const char* fname)
{
	std::string sfname;
	if (fname == NULL)
	{
		sfname = getProcessName()+".log"; //FIXME
	}
	else sfname = fname;

	FILE* _myfile = NULL;

//#ifdef _MSC_VER
//	fopen_s(&_myfile, sfname.c_str(), "ab");
//#else
//	_myfile = fopen(sfname.c_str(), "ab");
//#endif

	_myfile = fopen(sfname.c_str(), "ab");
	return _myfile;
}

void CxLog::SetEnableDaily(bool _enable, const char* path)
{
	 SetFilenamePrefix(path ? path : getProcessName(), _enable);
}


void CxLog::SetEnableToFile(bool _enable/*=true*/,const char* path/*=""*/, bool _daily /*= false*/)
{

	SetFilenamePrefix(path ? path : getProcessName(), _daily);

	if (m_worker)
	{
		m_bDestory = true;
		m_worker->join();
		m_bDestory = false;
		m_worker = NULL;
	}

	if (_enable && m_worker==NULL)
	{
		//打开写文件的线程
		m_worker = new std::thread(CxLog::writeToDisk,this);
	}

	m_bFile = _enable;

}

bool CxLog::SetFilenamePrefix(std::string path,bool _daily)
{
	m_bDaily = _daily;
	m_prefix = path;
	path =CxLog::getPath(path);
	return createRecursionDir(path);
}

void CxLog::SetLevel(int _level)
{
	m_level = _level;
}

void CxLog::SetColor(bool stdout_stream, int color)
{
#if PLATFORM == PLATFORM_WINDOWS

	static WORD WinColorFG[Color_count] =
	{
		0,                                                  // BLACK
		FOREGROUND_RED,                                     // RED
		FOREGROUND_GREEN,                                   // GREEN
		FOREGROUND_RED | FOREGROUND_GREEN,                  // BROWN
		FOREGROUND_BLUE,                                    // BLUE
		FOREGROUND_RED | FOREGROUND_BLUE,// MAGENTA
		FOREGROUND_GREEN | FOREGROUND_BLUE,                 // CYAN
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,// WHITE
															// YELLOW
															FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
															// RED_BOLD
															FOREGROUND_RED | FOREGROUND_INTENSITY,
															// GREEN_BOLD
															FOREGROUND_GREEN | FOREGROUND_INTENSITY,
															FOREGROUND_BLUE | FOREGROUND_INTENSITY,             // BLUE_BOLD
																												// MAGENTA_BOLD
																												FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
																												// CYAN_BOLD
																												FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
																												// WHITE_BOLD
																												FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
	};

	HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
	SetConsoleTextAttribute(hConsole, WinColorFG[color]);
#else

	enum ANSITextAttr
	{
		TA_NORMAL = 0,
		TA_BOLD = 1,
		TA_BLINK = 5,
		TA_REVERSE = 7
	};

	enum ANSIFgTextAttr
	{
		FG_BLACK = 30, FG_RED, FG_GREEN, FG_BROWN, FG_BLUE,
		FG_MAGENTA, FG_CYAN, FG_WHITE, FG_YELLOW
	};

	enum ANSIBgTextAttr
	{
		BG_BLACK = 40, BG_RED, BG_GREEN, BG_BROWN, BG_BLUE,
		BG_MAGENTA, BG_CYAN, BG_WHITE
	};

	static uint8 UnixColorFG[Color_count] =
	{
		FG_BLACK,                                           // BLACK
		FG_RED,                                             // RED
		FG_GREEN,                                           // GREEN
		FG_BROWN,                                           // BROWN
		FG_BLUE,                                            // BLUE
		FG_MAGENTA,                                         // MAGENTA
		FG_CYAN,                                            // CYAN
		FG_WHITE,                                           // WHITE
		FG_YELLOW,                                          // YELLOW
		FG_RED,                                             // LRED
		FG_GREEN,                                           // LGREEN
		FG_BLUE,                                            // LBLUE
		FG_MAGENTA,                                         // LMAGENTA
		FG_CYAN,                                            // LCYAN
		FG_WHITE                                            // LWHITE
	};

	fprintf((stdout_stream ? stdout : stderr), "\x1b[%d%sm", UnixColorFG[color], (color >= YELLOW && color < Color_count ? ";1" : ""));
#endif
}



void CxLog::ResetColor(bool stdout_stream)
{
#if PLATFORM == PLATFORM_WINDOWS
	HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#else
	fprintf((stdout_stream ? stdout : stderr), "\x1b[0m");
#endif
}


bool CxLog::isDirectory(std::string path)
{
#ifdef WIN32
	return PathIsDirectoryA(path.c_str()) ? true : false;
#else
	DIR * pdir = opendir(path.c_str());
	if (pdir == NULL)
	{
		return false;
	}
	else
	{
		closedir(pdir);
		pdir = NULL;
		return true;
	}
#endif
}


void CxLog::fixPath(std::string &path)
{
	if (path.empty()) { return; }
	for (std::string::iterator iter = path.begin(); iter != path.end(); ++iter)
	{
		if (*iter == '\\') { *iter = '/'; }
	}
	if (path.at(path.length() - 1) != '/') { path.append("/"); }
}

std::string CxLog::getPath(std::string path)
{
	if (path.empty()) { return path; }
	for (std::string::iterator iter = path.begin(); iter != path.end(); ++iter)
	{
		if (*iter == '\\') { *iter = '/'; }
	}
	
	int pos = (int)path.rfind("/");
	if (pos>0) {
		path.erase(pos, -1);
	}
	return path;
}

bool CxLog::createRecursionDir(std::string path)
{
	if (path.length() == 0) return true;
	std::string sub;
	fixPath(path);

	std::string::size_type pos = path.find('/');
	while (pos != std::string::npos)
	{
		std::string cur = path.substr(0, pos - 0);
		if (cur.length() > 0 && !isDirectory(cur))
		{
			bool ret = false;
#ifdef WIN32
			ret = CreateDirectoryA(cur.c_str(), NULL) ? true : false;
#else
			ret = (mkdir(cur.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0);
#endif
			if (!ret)
			{
				return false;
			}
		}
		pos = path.find('/', pos + 1);
	}

	return true;
}
