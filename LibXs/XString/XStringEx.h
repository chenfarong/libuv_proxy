#ifndef _XStringEx_h_
#define _XStringEx_h_

#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include <list>
#include <map>
#include <unordered_map>
#include <stdint.h>

#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <assert.h>
#include <cstdarg>
#include <time.h>
#include <ctype.h>
#include <sstream>
#include <algorithm>

//#include <functional> 
//#include <cctype>
//#include <locale>

//#include "StrFormat.h"
//#include "StrUtil.h"


using namespace std;

typedef unsigned char uchar;

class XTextFile;


class XString
{
public:
	static size_t ReplaceAll(string& str, string old_str, string new_str)
	{
		size_t rc = 0;
		size_t ol = old_str.length();
//		size_t nl = new_str.length();

		while(1) 
		{
			std::string::size_type p = str.find(old_str);
			if (p == std::string::npos) break;
			str.replace(str.begin() + p, str.begin() + p + ol, new_str);
			rc++;
		}

		return rc;
	}

	static size_t ReplaceForSQLComment(string& str)
	{
		size_t rc = ReplaceAll(str, "--", "&#45&#45");
		rc+= ReplaceAll(str, "/*", "&#47&#42");
		rc += ReplaceAll(str, "*/", "&#42&#47");
		return rc;
	}


	static inline void xsnprintf(char * buf, size_t len, const char * info, ...)
	{
		assert(len >= sizeof(char));
		int rs = 0;
		va_list args;
		va_start(args, info);
#ifdef _MSC_VER
		rs = ::_vsnprintf_s(buf, len - 1, len - 1, info, args);
#else
		rs = vsnprintf(buf, len - 1, info, args);
#endif
		va_end(args);

		if (rs<0) { buf[len - 1] = 0; }

	}


	/**
	找到 %s 的位置 判断其长度 累加 其他的留 256
	效率比较低 注意谨慎使用
	*/
	static std::string createWithFormat(const char* fmt, ...)
	{
		int rs = 0;
		const char* _start = NULL;
		const char* _end = NULL;
		size_t total = 256;
		va_list ap;
		va_start(ap, fmt);
		_end = fmt;

		do
		{
			_start = strstr(_end, "%");
			if (_start == NULL) break;
			_end = _start + 1;
			if (_end[0] == '%') continue;

			if (_end[0] == 's') {
				total += strlen(va_arg(ap, const char*));
			}
			else {
				total += 256;
				va_arg(ap, int);
			}

		} while (1);
		va_end(ap);

		va_start(ap, fmt);
		char *buf = new char[total + 1];
		//memset(buf, 0, total + 1);
#ifdef _MSC_VER
		rs = ::_vsnprintf_s_l(buf, total - 1, total - 1, fmt, NULL, ap);
		//rs = vsprintf(buf, fmt, ap);
		//rs = vsnprintf(buf, total - 1, fmt, ap);
#else
		rs = vsnprintf(buf, total - 1, fmt, ap);
#endif
		buf[rs] = 0;
		std::string s;
		s.append(buf, rs);
		delete[] buf;

		va_end(ap);
		return s;
	}


	static std::string createWithTime(time_t t)
	{
		char buf[256];
		struct tm tt = { 0 };
#if defined(_MSC_VER)
		localtime_s(&tt, &t);
#else //linux
		localtime_r(&t, &tt);
#endif

		xsnprintf(buf,256, "%d-%02d-%02d %02d:%02d:%02d",
			tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday,
			tt.tm_hour, tt.tm_min, tt.tm_sec);
		return buf;
	}

	static std::string createWithBool(bool v)
	{
		return v ? "true" : "false";
	}

	/**
	去除前面的空格
	*/
	static size_t TrimSpaceLeft(std::string& str)
	{
		size_t k = 0;
		//string::iterator i;
		//for (i = str.begin(); i != str.end(); i++) {
		//	if (!::isspace(*i)) {
		//		break;
		//	}
		//	k++;
		//}
		//if (i == str.end()) {
		//	str.clear();
		//}
		//else {
		//	if(i!=str.begin()) str.erase(str.begin(), i);
		//}
		string ch;
		for (size_t i = 0; i < str.length(); i++)
		{
			ch = str.substr(i, 1);
			if (ch.compare(" ") == 0) { k++;continue; }
			break;
		}
		if(k>0) str.erase(0, k);
		return k;
	}

	/**
	去除后面的空格
	*/
	static size_t TrimSpaceRight(std::string& str)
	{
		size_t k = 0;
		//string::iterator i;
		//for (i = str.end() - 1; ; i--) {
		//	if (!isspace(*i)) {
		//		if(i+1!=str.end()) str.erase(i + 1, str.end());
		//		break;
		//	}
		//	k++;
		//	if (i == str.begin()) {
		//		str.clear();
		//		break;
		//	}
		//}
		string ch;
		size_t i = str.length() - 1;
		for (; i >=0; i--)
		{
			ch = str.substr(i, 1);
			if (ch.compare(" ") == 0) {
				k++; continue; 
			}
			break;
		}
		if(k>0) str.erase(i+1);
		return k;
	}

	static size_t TrimSpaceLeftAndRight(std::string& str)
	{
		//return 0;
		return TrimSpaceLeft(str) + TrimSpaceRight(str);
	}

/*
	// trim from start
	static inline std::string& ltrim(std::string &s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	static inline std::string& rtrim(std::string &s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	static inline std::string& trim(std::string &s)
	{
		return ltrim(rtrim(s));
	}
*/

	/**
	在文本内容中获取第一行
	*/
	std::string textFirstLine(const char* src, int size, const char* emark = "\r\n")
	{
		std::string result;
		const char* p = strstr(src,emark);
		if (p > src + size) return "";
		result.append(src, p - src + strlen(emark));
		return result;
	}


};


template<typename T>
std::string XToString(T val)
{
	std::stringstream sss;
	sss << (T)val;
	return sss.str();
}

void test_xto_sting();

class XStringList
{
public:
/*  
	struct ABM
    {
        size_t a,b;
        size_t size(){return b-a;}
    };
*/
    vector<size_t> lines;
    
    string m_dat; //内容存储
    
public:
    XStringList(){}
    virtual ~XStringList(){}

protected:

	const char* m_xendl;
	const char* m_xclose;

	void ParseString(const char* _spchar = "\r\n", const char* _close = "\"");

public:
    
//    ssize_t Parse(const char* src);
//    ssize_t Parse2(const char* src,size_t fl);
    
    // _spchar 分隔字符串
//    size_t ParseEx(const char* lpBuf,size_t lpBufSize,const char* _spchar);
    
    /*
     _close 如果遇到这个符号 分割符号将被跳过，直到遇到第二个分割符号
     */
    size_t ParseEx2(const char* lpBuf,size_t lpBufSize=0,const char* _spchar="\r\n", const char* _close="\"");
    
public:    
    void FixABSpace();//删除前后的空格
public:    
   
    static void Test();
    
    int GetFeildLineVauleInt(string keyname);
    string GetFeildLineVauleString(string keyname);
//    ssize_t GetVaulePos(const char* fieldn);
    
    void List();
    
//    void Add(const char* text); //增加一行
	void push_back(string str);

public:
	/**
	行数
	*/
    size_t size();
    
    size_t LoadFromFile(const char* fname);
    size_t LoadFromString(string src,const char* lineMark="\r\n",const char* _closeMark="\"");

    virtual string operator[](size_t idx);
    string at(size_t idx);
	
	string GetLineString(size_t idx) {
		return at(idx);
	}


	//从头开始 返回完全符合这个词的后面一个词串
	//string NextLine(string str);
	
};



class XTokenizer : public XStringList
{
public:
	XTokenizer(){};

	XTokenizer(const std::string& _str, const std::string _delim=" ");
	XTokenizer(const char* _str,std::size_t sz=0, const std::string _delim=" ");
	virtual ~XTokenizer(){};

//   int         countTokens();
//   bool        hasMoreTokens();
//   std::string nextToken();
//   int         nextIntToken();
//   double      nextFloatToken();
//   std::string nextToken(const std::string& delim);
//   std::string remainingString();
//   std::string filterNextToken(const std::string& filterStr);

	/**
	 *
	 * */
	const char* GetTokenValue(std::string keyname);

	int GetTokenValueInt(const std::string& keyname);
	double  GetTokenValueFloat(const std::string& keyname);

	//第几个位置上
	int GetTokenValueInt(unsigned int _pos);

	string GetString(unsigned int _pos);
	std::string GetValueStringByIndex(unsigned int _idx);
	int GetValueIntByIndex(unsigned int _idx);
	/**
	* 返回关键字后面的内容
	*/
	string GetTokenRemain(const std::string& keyname);

	/**
	 * 是否存在
	 * */
	bool IsExistToken(const std::string& keyname);
	
	size_t GetTokenIndex(const string& keyname);


//	size_t Parse(const char* lpBuf,ssize_t lpBufSize=-1,const char* _spchar=" ", char _close='"');
	
	void Print();

	bool FirstTokenIs(const std::string& keyname);
	bool SecondTokenIs(const std::string& keyname);

	//忽略大小写
	bool FirstTokenIsIcase(const std::string& keyname);

public:
//		string m_dat;
//   std::list<string>  token_str;
//   std::string  delim;
   static string TokenClear(string str,string rstr=" \r\n",int _type=1);
   string GetTokenValueString(const string& keyname);
protected:
//	std::list<string>::iterator cur_token;
	
};

//void test_XTokenizer();

/**
文本参数
#字母开头的本行注释掉
=分开前后 如果行中不存在 = 抛弃
*/
class XTextParamList
{

	std::map<std::string, std::string> valueMap;
protected:
	
	void ParseLine(std::string line);

public:
	static std::string GetValue(std::string filename,std::string keyname);

	void LoadFromAsciiFile(std::string filename);
	void Parse(const char* _lpBuf, size_t _size);

	std::string GetValue(std::string keyname);
	std::wstring GetValueWString(std::string keyname);

	bool IsExist(std::string keyname);

	void Print();

	void LoadFile(const char* filename);

};

namespace XStringUtil
{
	bool utf8CharToUcs2Char(const char* utf8Tok, wchar_t* ucs2Char, unsigned int* utf8TokLen);
	void ucs2CharToUtf8Char(const wchar_t ucs2Char, char* utf8Tok);
	std::wstring utf8ToUcs2(const std::string& utf8Str);
	std::string ucs2ToUtf8(const std::wstring& ucs2Str);
};


class XTextFile
{
protected:
	uchar* dat;
	size_t		len;

public:
	static FILE* callfopen(const char* filepath, const char* mode)
	{
		assert(filepath);
		assert(mode);
#if defined(_MSC_VER) && (_MSC_VER >= 1400 ) && (!defined WINCE)
		FILE* fp = 0;
		errno_t err = fopen_s(&fp, filepath, mode);
		if (err) {
			return 0;
		}
#else
		FILE* fp = fopen(filepath, mode);
#endif
		return fp;
	}

public:
	XTextFile();
	virtual ~XTextFile();

	bool LoadFile(const char* filename);
	int LoadFile(FILE*);

	//operator std::string();
	const char* c_str() { return (const char*)dat; };
	size_t size() { return len; };

};


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
const char kUnreservedChar[] = {
	//0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, // 2
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 3
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 4
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // 5
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 6
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, // 7
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // C
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // D
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // E
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // F
};

const char kHexToNum[] = {
	// 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 1
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 2
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, // 3
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 4
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 5
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 6
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 7
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 8
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 9
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // A
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // B
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // C
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // D
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // E
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // F
};

class CxUrlEncoder {
public:
	static inline void Encode(const std::string &in, std::string *out, bool uppercase = false) {
		std::stringstream ss;

		for (std::string::const_iterator it = in.begin(); it != in.end(); ++it) {
			if (kUnreservedChar[*it]) {
				ss << *it;
			}
			else {
				ss << '%' << std::setfill('0') << std::hex;
				if (uppercase) ss << std::uppercase;
				ss << (int)*it;
			}
		}

		out->assign(ss.str());
	}

	static inline std::string Encode(const std::string &in, bool uppercase = false) {
		std::string out;
		Encode(in, &out, uppercase);
		return out;
	}

	static inline bool Decode(const std::string &in, std::string *out) {
		std::stringstream ss;

		std::string::const_iterator it = in.begin();

		char tmp, encoded_char;
		while (it != in.end()) {
			if (*it == '%') {
				if (++it == in.end()) return false;
				tmp = kHexToNum[*it];
				if (tmp < 0) return false;
				encoded_char = (16 * tmp);

				if (++it == in.end()) return false;
				tmp = kHexToNum[*it];
				if (tmp < 0) return false;
				encoded_char += tmp;

				++it;

				ss << encoded_char;
			}
			else {
				ss << *it;
				++it;
			}
		}

		out->assign(ss.str());

		return true;
	}

	static inline std::string Decode(const std::string &in) {
		std::string out;
		Decode(in, &out);
		return out;
	}

private:
};

class CxHttpHead
{
public:
	enum _action {
		GET,
		POST,
	};

public:

	std::unordered_map<std::string, std::string> field_values;

public:
	std::string Document;

	std::string version;

	int action;

public:

	void ParseGet(const char* buf, int size)
	{
		std::string str;
		str.append(buf, size);
		ParseGet(str);
	}

	void ParseGet(std::string _uri)
	{
		field_values.clear();

		std::string str= CxUrlEncoder::Decode(_uri);

		/*
		GET /index.html?op=download&ino=2941&type=file HTTP/1.1
		*/
		XTokenizer tok;
		tok.ParseEx2(str.c_str(), str.length(), " ");
		std::string _doc=tok.GetLineString(1);
		
		if (_doc.empty()) return;

		tok.ParseEx2(_doc.c_str(), _doc.length(), "?");
		Document = tok.GetLineString(0);
		
		if (tok.size() < 2) return;

		tok.ParseEx2(tok.at(1).c_str(), tok.at(1).length(), "&");

		for (size_t l=0;l<tok.size();l++)
		{
			string _feild = tok.GetLineString(l);
			printf("%s %d:%s\n",__FILE__,__LINE__,_feild.c_str());
			//XTokenizer tok2;
			//tok2.ParseEx2(_feild.c_str(), _feild.length(), "=");
			//if (tok2.size() == 2) {
			//	field_values[tok2.at(0)] = tok2.at(1);
			//}
			int pos=(int)_feild.find("=");
			if (pos > 0) {
				string k = _feild.substr(0, pos);
				string v = _feild.substr(pos + 1, -1);
				field_values[k] = v;
			}
		}

	}

	/**
	获取文档后面跟着的参数值
	*/
	const char* GetKeyValue(const std::string& kname)
	{
		auto it = field_values.find(kname);
		if (it != field_values.end()) return (it->second).c_str();
		return NULL;
	}

	int GetKeyValueInt(const std::string& kname)
	{
		const char* v = GetKeyValue(kname);
		if (v == NULL) return 0;
		return atoi(v);
	}

	std::string GetKeyValueString(const std::string& kname)
	{
		auto it = field_values.find(kname);
		if (it != field_values.end()) return (it->second);
		return "";
	}

};



#endif