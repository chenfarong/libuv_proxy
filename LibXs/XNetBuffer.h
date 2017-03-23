
#ifndef _LOOPBUFFER2_H_
#define _LOOPBUFFER2_H_

#include <map>
#include <vector>
#include <memory>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <mutex>

#if(0)
#include "XStringCoder.h"
#endif

#ifndef uchar
typedef unsigned char uchar;
#endif

typedef unsigned char uint8;
typedef std::vector<uint8> xarray_uint8;

#define X_MAX_NETBUFFER		40960

#define X_MAX_NSIZE			65535

#define X_CLRF  "\r\n"
#define X_CLRF2 "\r\n\r\n"

#define X_CLRF2_C "&#13;&#10;&#13;&#10;"

//#define X_CLRF2_SIZE  strlen(X_CLRF2_C)

class CxNetBufferHelper
{
public:
	static bool x_str_first(const char* _left, const char* _right, unsigned int _size)
	{
		for (unsigned int i = 0; i < _size; i++)
		{
			if (_right[i] != _left[i]) return false;
		}
		return true;
	}

	static void xarray_uint8_append(xarray_uint8& des, const char* buf, unsigned int size)
	{
		size_t op = des.size();
		des.resize(des.size() + size);
		memcpy(&des[op], buf, size);
	}

	static bool x_str_include(const char* _des, unsigned int _size, char c)
	{
		for (unsigned int i = 0; i < _size; i++) if (_des[i] == c) return true;
		return false;
	}

	static std::string x_str_255t(char c)
	{
		//&#13;
		char buf[32];
		sprintf(buf, "&#%d;", (int)c);
		return buf;
	}

//#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

	static char x_str_255f(std::string s)
	{
		int r = 0;
		//找到第一个&# 找到第一个数字后第一个不是数字
		int _posL = s.find("&#");
		int _posR = s.find(";", _posL);
		if (_posL != -1 && _posR != -1)
		{
			std::string a = s.substr(_posL + 2, _posR - _posL - 2);
			r = atoi(a.c_str());
		}
		return (char)r;
	}

};

class CxDChunk
{
protected:
	char* dat;
	size_t len;

public:
	CxDChunk()
	{
		//dat.resize(4096);
		dat = NULL;
		len = 0;
	}

	virtual ~CxDChunk() {
		if (dat) delete[] dat;
		dat = NULL;
	}

	CxDChunk(const void* _dat, size_t _size)
	{
		dat = NULL;
		len = 0;
		SetData(_dat,_size);
	}
	
	char* c_str()
	{
		//if (dat.size() < 1) return NULL;
		//return (char*)&dat[0];
		return dat;
	}
	
	std::string CppStr()
	{
		std::string str;
		str.append(dat, len);
		return str;
	}

	void SetData(const void* _dat,size_t _size)
	{
		assert(_size > 0 && _dat);
		//dat.resize(_size);
		resize(_size);
		len = _size;
		memcpy(c_str(), _dat, _size);
	}
	
	size_t length() { return len; }

	void resize(size_t _size) 
	{
		if (dat) delete[] dat;
		dat = new char[_size + 1];
		len = _size;
	}
};

class CxDChunkP32 : public CxDChunk
{
public:
	CxDChunkP32(const void* _dat, unsigned int _size)
	{
		//dat.resize(_size + 4);
		len = _size;
		dat = new char[_size + 4 + 1];
		memcpy(dat, &_size,4);
		memcpy(dat+4, _dat, _size);
	}
};

#if(1)


class CxDChunkEMark : public CxDChunk
{
public:
	class CxStringCoder255 
	{
		char m_buffer[256];
		int  m_size;

		inline bool IsReserve(char c)
		{
			for (int i = 0; i < m_size; i++) {
				if (m_buffer[i] == c) return true;
			}
			return false;
		}

		const char* m_format = "&#%d;";
	public:



		/**
		设置要转换的预留
		*/
		void SetConvReserve(const char* _src, int _size)
		{
			if (_src == NULL) _src = "\r\n\r\n";
			if (_size < 1) _size = (int)strlen(_src);

			assert(_size < 256);
			m_size = _size;
			memcpy(m_buffer, _src, _size);
		}

		void Encode(std::vector<uint8>& des, const char* src, size_t size)
		{
			char buf[32];
			des.clear();
			for (size_t i = 0; i < size; i++)
			{
				char c = src[i];
				if (IsReserve(c))
				{
					snprintf(buf, 32, m_format, c);
					for (size_t j = 0; j < strlen(buf); j++) des.push_back(buf[j]);
				}
				else des.push_back(src[i]);
			}
		}

		void Decode(std::vector<uint8>& des, const char* src, size_t _size)
		{
			for (size_t i = 0; i < _size; i++)
			{
				if ((i + 1) < _size && src[i] == '&' &&  src[i + 1] == '#')
				{
					size_t k = i + 2;
					std::vector<uint8> code;
					while (src[k] != ';' && k < _size)
					{
						code.push_back(src[k]);
						k++;
					}
					code.push_back('\0');
					int _code = atoi((const char*)&code[0]);

					char c = (char)_code;
					des.push_back(c);
					i = k;
				}
				else
				{
					des.push_back(src[i]);
				}
			}

		}

	};
public:
	
	CxStringCoder255 coder;

	void WritePackage(const void* _dat, unsigned int _size,const char* _emark,int _emark_len)
	{
		if (_emark == NULL) _emark = "\r\n\r\n";
		if (_emark_len < 1) _emark_len = (int)strlen(_emark);

		//先进行转码
		xarray_uint8 des;
		coder.SetConvReserve(_emark, _emark_len);
		coder.Encode(des, (const char*)_dat, _size);
		

		resize(des.size() + _emark_len);
		memcpy((char*)&dat[0], (const char*)&des[0], des.size());
		memcpy((char*)&dat[des.size()], _emark, _emark_len);
	}

    
    void WritePackage2(const char* _dat, unsigned int _size,
                       const char* _emark,unsigned int _emark_len)
    {
        if (_emark == NULL) _emark = "\r\n\r\n";
        if (_emark_len < 1) _emark_len = (int)strlen(_emark);
        
        
        if(_size<_emark_len)
        {
            resize(_size+_emark_len);
            memcpy((char*)&dat[0], _dat, _size);
            memcpy((char*)&dat[_size], _emark, _emark_len);
            return;
        }
        
        //先进行转码
        xarray_uint8 des;
        int l=_size-_emark_len;
        
        for(int i=0;i<l;i++)
        {
            if(CxNetBufferHelper::x_str_first(_dat+i,_emark,_emark_len)){
				CxNetBufferHelper::xarray_uint8_append(des,X_CLRF2_C,strlen(X_CLRF2_C));
                i+=(_emark_len-1);
            }else des.push_back(_dat[i]);
        }
        
        resize(des.size() + _emark_len);
        memcpy((char*)&dat[0], (const char*)&des[0], des.size());
        memcpy((char*)&dat[des.size()], _emark, _emark_len);
    }
    
    //void SwapBuffers(xarray_uint8& des)
	//{
	//	//dat.swap(des);
	//	resize(des.size());
	//	memcpy(dat, (char*)&des[0], des.size());
	//	des.clear();
	//}

	void ReadPackageFix(const char* _emark, int _emark_len)
	{
		if (length() < 1) return;

		if (_emark == NULL) _emark = "\r\n\r\n";
		if (_emark_len < 1) _emark_len = (int)strlen(_emark);
		//先进行转码
		xarray_uint8 des;
		coder.SetConvReserve(_emark, _emark_len);
		coder.Decode(des, c_str(), length());
//		if (des.size() >= _emark_len) des.resize(des.size() - _emark_len);
//		des.swap(dat);
		resize(des.size());
		memcpy(dat, (char*)&des[0], des.size());
	}

    void ReadPackageFix2(const char* _emark, int _emark_len)
    {
        if (length() < 1) return;
        
        if (_emark == NULL) _emark = "\r\n\r\n";
        if (_emark_len < 1) _emark_len = (int)strlen(_emark);
        //先进行转码
        xarray_uint8 des;
        int l=len-_emark_len;
        for(int i=0;i<l;i++)
        {
            if(CxNetBufferHelper::x_str_first(dat+i,_emark,_emark_len))
            {
				CxNetBufferHelper::xarray_uint8_append(des,X_CLRF2,strlen(X_CLRF2));
                i+=3;
            }else des.push_back(dat[i]);
        }
        
		//处理末尾字符 如果最后是 &#13; &#10; 那么就替换掉

        resize(des.size());
        memcpy(dat, (char*)&des[0], des.size());
    }
    
};
#endif

class CxNetBuffer
{
protected:
//	std::vector<uchar> _storage;
	unsigned int _rpos, _wpos;

	char* data;

	std::mutex  mtx_class;
	unsigned int _total;

public:
	CxNetBuffer()
	{
		_rpos = _wpos = 0;
//		_storage.resize(65535);
		_total = 65535;
		data = NULL;// new char[_total];
		do {
			data = (char*)malloc(_total);
		} while (data == NULL);
	}

	virtual ~CxNetBuffer()
	{
		free(data);
	}

	char* c_str()
	{
		return data;
	}

	char* getWritePtr() {
		return data+_wpos;
	}

	char* getReaderPtr()
	{
		return data+_rpos;
	}

	size_t getWriteSpace(size_t _asize=1) {
		size_t  l = _total - _wpos;
		if (l < _asize) {
			l = _asize-l;
			//_storage.resize(_storage.size() + l+1);
			_total = _total + l + 1;
			data = (char*)realloc(data, _total);
		}
		return l;
	}

	size_t getContentLength()
	{
		return _wpos-_rpos;
	}

	void fixRwPos()
	{
		if (_rpos >= _wpos) 
		{
			_rpos = _wpos = 0;
			//TODO 如果缓存太大可以缩小一些
			//printf("---------到头 %lu:%lu----------\n",_rpos,_wpos);
			//std::cout.flush();
			//printf(".");
		}

	}

	CxDChunk* getChunkPreUint32()
	{
		std::lock_guard<std::mutex> lck(mtx_class);

		CxDChunk* chunk = NULL;
		size_t cs = getContentLength();
		if (cs < 4) return NULL; //还不够前面标记长度的内容
		uint32_t* pks = (uint32_t*)getReaderPtr();
		uint32_t ns = *pks + 4;
		if (cs < ns) return NULL; 
		assert(*pks>0);
		//rposInc(4); //去掉前面长度标记
		if(*pks>0) chunk = new CxDChunk(getReaderPtr()+4,*pks);
		rposInc(ns);
		//fixRwPos();
		return chunk;
	}

	CxDChunk* getChunkPreUint32Ex()
	{
		std::lock_guard<std::mutex> lck(mtx_class);

		CxDChunk* chunk = NULL;
		size_t cs = getContentLength();
		if (cs < 4) return NULL; //还不够前面标记长度的内容
		uint32_t* pks = (uint32_t*)getReaderPtr();
		uint32_t ns = *pks + 4;
		if (cs < ns) return NULL;
		assert(*pks > 0);  //出现了问题
		//rposInc(4); //去掉前面长度标记
		if (*pks > 0) chunk = new CxDChunk(getReaderPtr(), ns);
		rposInc(ns);
		//fixRwPos();
		return chunk;
	}


	void resize(size_t _Size) {
//		_storage.resize(_Size);
		_total = _Size;
		data = (char*)realloc(data, _total);
	}


	void resetZero() 
	{
		_wpos = 0;
		_rpos = 0;
	}

	void expandTo(size_t _size)
	{
		size_t  l = _total - _wpos;
		if (l < _size) {
			l = _size-l;
			_total += l;
			resize(_total);
		}
	}

	inline void ClampI(unsigned int& pos)
	{
		//if (pos >= _storage.size())
		//{
		//	pos = 0;// _storage.size() - 1; //FIXME
		//}
		//if (pos < 1) pos = 0;
	}

	void wposInc(int _offset)
	{
		_wpos += _offset;
		ClampI(_wpos);
	}

	void rposInc(int _offset)
	{
		_rpos += _offset;
		ClampI(_rpos);
		fixRwPos();
	}

	void write(const char* _lpBuf, int _size)
	{
		std::lock_guard<std::mutex> lck(mtx_class);
		expandTo(_size);
		memcpy(getWritePtr(), _lpBuf, _size);
		wposInc(_size);
	}

	//bool  GetLineText(CxData* des, std::string _endmark = "\r\n\r\n")
	//{
	//	char* c = strstr(c_str(), _endmark.c_str());
	//	if(c==NULL) return false;

	//	size_t ns = c - c_str()+_endmark.length();
	//	
	//	//des.append(c_str(), ns);
	//	if(des) des->SetData(c_str(), ns);
	//	
	//	_wpos -= ns;
	//	_storage.erase(_storage.begin(), _storage.begin() + ns);
	//	return true;
	//}


	bool firstEqual(const char* _Left,const char* _Right,size_t _size)
	{
		for (size_t i = 0; i < _size; i++) {
			if (_Left[i] != _Right[i]) return false;
		}
		return true;
	}

#if(1)
	CxDChunk*  getChunkEMark(const char* _endmark,size_t _size)
	{
		std::lock_guard<std::mutex> lck(mtx_class);

		size_t cl = getContentLength();
		if (cl == 0) return NULL;

		if (_endmark == NULL) _endmark = "\r\n\r\n";
		if (_size < 1) _size = strlen(_endmark);

		if (cl < _size) return NULL;

		//const char* mm = _endmark;
		//size_t pp = 0;
		size_t ml = _size;
		size_t k = _rpos;
		bool  b = false;


		for (;k<=_wpos-ml;k++)
		{
			b = firstEqual((const char*)data+k, _endmark, ml);
			if ( b== true) break;
		}

		if (!b) return NULL;

		int ns = (int)(k+ml-_rpos);

		CxDChunkEMark* chunk = new CxDChunkEMark(/*getReaderPtr(), ns*/);
		chunk->SetData(getReaderPtr(), ns);
		rposInc(ns);
		chunk->ReadPackageFix(_endmark, (int)_size);

		return chunk;
	}
    
    
    CxDChunk*  getChunkEMark2(const char* _endmark,size_t _size)
    {
        std::lock_guard<std::mutex> lck(mtx_class);
        
        size_t cl = getContentLength();
        if (cl == 0) return NULL;
        
        if (_endmark == NULL) _endmark = "\r\n\r\n";
        if (_size < 1) _size = strlen(_endmark);
        
        if (cl < _size) return NULL;
        
        size_t ml = _size;
        size_t k = _rpos;
        bool  b = false;
        
        
        for (;k<=_wpos-ml;k++)
        {
            b = firstEqual((const char*)data+k, _endmark, ml);
            if ( b== true) break;
        }
        
        if (!b) return NULL;
        
        int ns = (int)(k+ml-_rpos);
        
        CxDChunkEMark* chunk = new CxDChunkEMark(/*getReaderPtr(), ns*/);
        chunk->SetData(getReaderPtr(), ns-ml);
        rposInc(ns);
        chunk->ReadPackageFix(_endmark, (int)_size);
        
        return chunk;
    }
    
    
#endif

	void WritefixP32(const void* _dat, unsigned int _size)
	{
		resize(_size + 4);
		memcpy(data, &_size, 4);
		memcpy(data+4, _dat, _size);
		_wpos = _size + 4;
	}

#if(1)
	void WritefixEMark(const void* _dat, unsigned int _size, const char* _emark, int _emark_len)
	{
		CxDChunkEMark chunk;
		chunk.WritePackage(_dat, _size, _emark, _emark_len);
		//chunk.SwapBuffers(dat);
		resize(chunk.length());
		memcpy(c_str(), chunk.c_str(), chunk.length());
		_rpos = 0;
		_wpos = chunk.length();
	}

	/**
	看内容中是否有完整的后缀保留串，如果是转换
	最后一个字母是否为后缀中的保留字 如果是转换
	*/
	void WritefixEMark2(const char* _dat, unsigned int _size, const char* _emark, int _emark_len)
	{
		if(_size < 1) return;

		if(_emark==NULL) _emark="\r\n\r\n";
		if (_emark_len < 1) _emark_len = strlen(_emark);

		resize(_size*2);
		int l = _size - _emark_len;

		if (l > 0)
		{
			size_t mss = strlen(X_CLRF2_C);

			for (unsigned int i = 0; i < l; i++)
			{
				if (CxNetBufferHelper::x_str_first(_dat + 1, _emark, _emark_len))
				{
					write(X_CLRF2_C, (int)mss);
					i += (_emark_len - 1);
				}
				else {
					data[i] = _dat[i];
					_wpos++;
				}
			}//--for


			//处理最后几个字符
			write(_dat + l, _emark_len - 1);
		}
		else {
			write(_dat, _size - 1);
		}

		//处理最后一个字符
		char lastC = _dat[_size - 1];
		if (CxNetBufferHelper::x_str_include(_emark, _emark_len,lastC ))
		{
			//
			std::string s = CxNetBufferHelper::x_str_255t(_dat[_size - 1]);
			write(s.c_str(), s.length());
		}
		else write(&lastC, 1);

		//加上末尾标记
		write(_emark, _emark_len);

	}



#endif


	void clear()
	{
		_rpos = 0;
		_wpos = 0;
	}


};



#endif

