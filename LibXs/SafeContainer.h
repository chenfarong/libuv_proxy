#ifndef xsafeContainer_h__
#define xsafeContainer_h__

#include <map>
#include <mutex>
#include <unordered_map>
#include <set>
#include <list>
#include <vector>
#include <queue>
#include <deque>

#define X_LOCK_GUARD(a) std::lock_guard<std::mutex> lck(a)

class CxClassMutex
{
protected:
	std::mutex mtx_class;

public:

};


template<typename T1,typename T2>
class CxSafeMap : public CxClassMutex
{
public:
	std::map<T1, T2> container;

public:
	void SafeAppend(const T1& key, const T2& val)
	{
		X_LOCK_GUARD(mtx_class);
		container.insert(std::pair<T1, T2>(key, val));
	}

	void SafeRemove(const T1& key)
	{
		X_LOCK_GUARD(mtx_class);
		container.erase(key);
	}


};


template<typename T>
class XSafeSet
{
protected:
	std::mutex m_mutex;

	std::set<T> container;

public:
	void append(T a)
	{
		std::lock_guard<std::mutex> lck(m_mutex);
		container.insert(a);
	}

	void remove(T a)
	{
		std::lock_guard<std::mutex> lck(m_mutex);
		container.erase(a);
	}

	bool exist(T a)
	{
		auto it= container.find(a);
		return it != container.end();
	}

};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////



template<class T>
class CxSafeVector
{
protected:
	std::mutex m_mutex;
public:
	std::vector<T> container;

public:
	void safe_push_back(const T& val)
	{
		std::lock_guard<std::mutex> lck(m_mutex);
		container.push_back(val);
	}

	size_t safe_remove(const T& val)
	{
		size_t _k = 0;
		std::lock_guard<std::mutex> lck(m_mutex);
		auto it = container.begin();
		while (it!=container.end())
		{
			if (*it == val) {
				it = container.erase(it);
				_k++;
			}
			else it++;
		}
		return _k;
	}

};


template<class T1,class T2> 
class CxSafeUnorderMap : public CxClassMutex
{
public:
	std::unordered_map<T1, T2> container;

public:
	void SafeAppend(const T1& key, const T2& val)
	{
		X_LOCK_GUARD(mtx_class);
		container.insert(std::pair<T1, T2>(key, val));
	}

	void SafeRemove(const T1& key)
	{
		X_LOCK_GUARD(mtx_class);
		container.erase(key);
	}

};



template<class T>
class CxSafeQueue : public CxClassMutex
{

public:
	std::deque<T> dats;

public:

	void SafePushBack(T& val)
	{
		std::lock_guard<std::mutex> lck(mtx_class);
		dats.push_back(val);
	}


};


#endif // xsafeContainer_h__
