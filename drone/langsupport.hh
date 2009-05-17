//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#ifndef __langsupport_hh__
#define __langsupport_hh__
#include <string>
#include <map>

class LangSupport {
public:
	virtual bool hasSource(std::string name) = 0;
	virtual int compile(std::string name, int user, int group) = 0;
	virtual void restrictRun(std::string name, bool entryAccess) = 0;
	virtual void unrestrictRun(std::string name) = 0;
	virtual int run(std::string name, 
					int in, int out, int err, 
					size_t memoryLimit,
					size_t outputLimit,
					float & time,			   
					int user,
		            int group) = 0;
	virtual float rank() = 0;
	virtual std::string name() = 0;
};
typedef std::multimap<float, LangSupport *> langByRank_t;

extern langByRank_t langByRank;
extern std::map<std::string, LangSupport *> langByName;


LangSupport * producePythonLangSupport();
LangSupport * produceCCLangSupport();
LangSupport * produceJavaLangSupport();


#endif //__langsupport_hh__
