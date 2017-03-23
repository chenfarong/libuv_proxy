#ifndef config_h__
#define config_h__

#include <unordered_map>
#include <string>
#include <stdint.h>
#include <algorithm>

typedef std::unordered_map<std::string, std::string> XsConfigSegment;

class XsConfig
{
public:
	std::unordered_map<std::string, XsConfigSegment> attributes;

	virtual int LoadFromXmlFile(const char* fname) 
	{
		return 0;
	}

	virtual int LoadFromIniFile(const char* fname) { return 0; };

	int SetAttrib(std::string segmentName, std::string kname, std::string value)
	{
		attributes[segmentName][kname] = value;
		return 0;
	}

	template<class T>
	int SetValue(std::string segmentName, std::string kname, T value)
	{
		std::string str = std::to_string(value);
		attributes[segmentName][kname] = str;
		return 0;
	}


	std::string GetString(std::string segmentName, std::string kname)
	{
		return attributes[segmentName][kname];
	}

	int GetInt32(std::string segmentName, std::string kname)
	{
		std::string str = GetString(segmentName, kname);
		return atoi(str.c_str());
	}

	unsigned long GetIntULOG(std::string segmentName, std::string kname)
	{
		std::string str = GetString(segmentName, kname);
		return std::strtoul(str.c_str(), 0, 10);
	}

	bool GetBoolean(std::string segmentName, std::string kname)
	{
		std::string str = GetString(segmentName, kname);
		if (str.empty()) return false;

		std::transform(str.begin(), str.end(), str.begin(), tolower);
		if(str.compare("false")==0 || str.compare("0") == 0) return false;

		return true;
	}

	int64_t GetInt64(std::string segmentName, std::string kname)
	{
		std::string str = GetString(segmentName, kname);
		if (str.empty()) return false;
		return std::strtoll(str.c_str(), 0, 10);
	}

	uint64_t GetInt64U(std::string segmentName, std::string kname)
	{
		std::string str = GetString(segmentName, kname);
		if (str.empty()) return false;
		return std::strtoull(str.c_str(), 0, 10);
	}

	float GetFloat(std::string segmentName, std::string kname)
	{
		std::string str = GetString(segmentName, kname);
		return atof(str.c_str());
	}

	double  GetDouble(std::string segmentName, std::string kname)
	{
		std::string str = GetString(segmentName, kname);
		return std::strtod(str.c_str(), 0);
	}
};



#endif // config_h__
