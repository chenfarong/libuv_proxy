#ifndef config_h__
#define config_h__

#include <unordered_map>
#include <string>
#include <stdint.h>

typedef std::unordered_map<std::string, std::string> XsConfigSegment;

class XsConfig
{
public:
	std::unordered_map<std::string, XsConfigSegment> attributes;

	virtual int LoadFromXmlFile(const char* fname) 
	{
		return 0;
	}

	int LoadFromIniFile(const char* fname);

	int SetAttrib(std::string segmentName, std::string kname, std::string value)
	{
		return 0;
	}

	int AttribInt32(std::string segmentName, std::string kname);
	bool AttribBoolean(std::string segmentName, std::string kname);
	int64_t AttribInt64(std::string segmentName, std::string kname);


};



#endif // config_h__
